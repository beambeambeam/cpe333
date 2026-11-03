// SPDX-License-Identifier: GPL-2.0
/*
 * pfcount - per-process page-fault counter and address-space inspector.
 *
 * /proc/pfcount/target  (rw)  write a PID to start monitoring, "0" to stop
 * /proc/pfcount/stats   (r)   region count, address-space size, fault counts
 * /proc/pfcount/maps    (r)   VMA list of the target, like /proc/<pid>/maps
 *
 * Two fault sources:
 *   lifetime  - task->min_flt / maj_flt summed over all threads, the same
 *               counters /proc/<pid>/stat and getrusage() report.
 *   live      - a kretprobe on handle_mm_fault() counts every fault the
 *               target takes after monitoring starts, split minor/major and
 *               anonymous/file-backed. Uses the same major/minor rule as
 *               mm_account_fault() in mm/memory.c.
 */
#include <linux/atomic.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/mmap_lock.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pid.h>
#include <linux/proc_fs.h>
#include <linux/sched/mm.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CPE333 Mini-Project 2");
MODULE_DESCRIPTION("Per-process page-fault counter");

static int pid;
module_param(pid, int, 0444);
MODULE_PARM_DESC(pid, "PID to monitor at load time (optional)");

/* Target state. target_pid is protected by target_lock. */
static DEFINE_MUTEX(target_lock);
static struct pid *target_pid;
static pid_t target_tgid;	/* read locklessly by the probe */
static u64 target_since_ns;

static atomic64_t live_minor, live_major, live_anon, live_file, live_error;

static struct proc_dir_entry *pf_dir;

/* ---------- live fault counting ---------- */

struct fault_ctx {
	unsigned int flags;
	bool file;
};

/* handle_mm_fault(struct vm_area_struct *vma, unsigned long address,
 *                 unsigned int flags, struct pt_regs *regs) */
static int fault_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	pid_t tgid = READ_ONCE(target_tgid);
	struct fault_ctx *ctx = (struct fault_ctx *)ri->data;
	struct vm_area_struct *vma;

	if (!tgid || current->tgid != tgid)
		return 1; /* not our process: skip the return handler */

	vma = (struct vm_area_struct *)regs_get_kernel_argument(regs, 0);
	ctx->flags = (unsigned int)regs_get_kernel_argument(regs, 2);
	ctx->file = vma && vma->vm_file;
	return 0;
}

static int fault_return(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct fault_ctx *ctx = (struct fault_ctx *)ri->data;
	vm_fault_t ret = (vm_fault_t)regs_return_value(regs);

	/* A retried fault is handled again and counted on the final attempt. */
	if (ret & VM_FAULT_RETRY)
		return 0;
	if (ret & VM_FAULT_ERROR) {
		atomic64_inc(&live_error);
		return 0;
	}
	if ((ret & VM_FAULT_MAJOR) || (ctx->flags & FAULT_FLAG_TRIED))
		atomic64_inc(&live_major);
	else
		atomic64_inc(&live_minor);
	atomic64_inc(ctx->file ? &live_file : &live_anon);
	return 0;
}

static struct kretprobe fault_probe = {
	.kp.symbol_name	= "handle_mm_fault",
	.entry_handler	= fault_entry,
	.handler	= fault_return,
	.data_size	= sizeof(struct fault_ctx),
	.maxactive	= 64,
};

/* ---------- target selection ---------- */

static void reset_live(void)
{
	atomic64_set(&live_minor, 0);
	atomic64_set(&live_major, 0);
	atomic64_set(&live_anon, 0);
	atomic64_set(&live_file, 0);
	atomic64_set(&live_error, 0);
}

/* Caller holds target_lock. nr == 0 clears the target. */
static int set_target(int nr)
{
	struct pid *p = NULL;
	struct task_struct *task;
	pid_t tgid = 0;

	if (nr < 0)
		return -EINVAL;
	if (nr) {
		p = find_get_pid(nr);
		if (!p)
			return -ESRCH;
		task = get_pid_task(p, PIDTYPE_PID);
		if (!task) {
			put_pid(p);
			return -ESRCH;
		}
		tgid = task->tgid;
		put_task_struct(task);
	}

	WRITE_ONCE(target_tgid, 0);
	put_pid(target_pid);
	target_pid = p;
	reset_live();
	target_since_ns = ktime_get_ns();
	WRITE_ONCE(target_tgid, tgid);
	return 0;
}

/* Returns a referenced task or NULL. Caller holds target_lock. */
static struct task_struct *target_task(void)
{
	return target_pid ? get_pid_task(target_pid, PIDTYPE_PID) : NULL;
}

/* ---------- helpers ---------- */

/* Same sum as /proc/<pid>/stat: exited threads + live threads. */
static void lifetime_faults(struct task_struct *task, unsigned long *min,
			    unsigned long *maj, int *threads)
{
	struct task_struct *t;

	rcu_read_lock();
	*min = task->signal->min_flt;
	*maj = task->signal->maj_flt;
	*threads = 0;
	for_each_thread(task, t) {
		*min += t->min_flt;
		*maj += t->maj_flt;
		(*threads)++;
	}
	rcu_read_unlock();
}

enum region_kind { R_FILE, R_ANON, R_HEAP, R_STACK, R_SPECIAL, R_NKINDS };

static const char *const kind_name[R_NKINDS] = {
	"file", "anon", "heap", "stack", "special",
};

static enum region_kind classify(struct vm_area_struct *vma, const char **name)
{
	struct mm_struct *mm = vma->vm_mm;

	*name = NULL;
	if (vma->vm_file)
		return R_FILE;
	if (vma->vm_ops && vma->vm_ops->name) {
		*name = vma->vm_ops->name(vma);
		if (*name)
			return R_SPECIAL;
	}
	if (vma->vm_start <= mm->brk && vma->vm_end >= mm->start_brk) {
		*name = "[heap]";
		return R_HEAP;
	}
	if (vma->vm_start <= mm->start_stack && vma->vm_end >= mm->start_stack) {
		*name = "[stack]";
		return R_STACK;
	}
	return R_ANON;
}

/* ---------- /proc/pfcount/stats ---------- */

static int stats_show(struct seq_file *m, void *v)
{
	unsigned long count[R_NKINDS] = {}, bytes[R_NKINDS] = {};
	unsigned long regions = 0, vsize = 0, min, maj;
	unsigned long total_vm = 0, rss = 0;
	struct task_struct *task;
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma;
	char comm[TASK_COMM_LEN];
	int threads, i, ret = 0;
	pid_t nr;
	u64 since;

	mutex_lock(&target_lock);
	nr = target_pid ? pid_nr(target_pid) : 0;
	since = target_since_ns;
	task = target_task();
	mutex_unlock(&target_lock);

	if (!nr) {
		seq_puts(m, "pid: 0\nstate: idle\n");
		return 0;
	}

	seq_printf(m, "pid: %d\n", nr);
	if (!task) {
		seq_puts(m, "state: exited\n");
		goto live;
	}

	get_task_comm(comm, task);
	lifetime_faults(task, &min, &maj, &threads);
	mm = get_task_mm(task);
	put_task_struct(task);

	seq_printf(m, "comm: %s\n", comm);
	seq_printf(m, "state: %s\n", mm ? "running" : "no-mm");
	seq_printf(m, "threads: %d\n", threads);

	if (mm) {
		VMA_ITERATOR(vmi, mm, 0);

		ret = mmap_read_lock_killable(mm);
		if (ret) {
			mmput(mm);
			return ret;
		}
		for_each_vma(vmi, vma) {
			const char *name;
			enum region_kind k = classify(vma, &name);
			unsigned long sz = vma->vm_end - vma->vm_start;

			regions++;
			vsize += sz;
			count[k]++;
			bytes[k] += sz;
		}
		total_vm = mm->total_vm;
		rss = get_mm_rss(mm);
		mmap_read_unlock(mm);
		mmput(mm);

		seq_printf(m, "regions: %lu\n", regions);
		seq_printf(m, "vsize_kb: %lu\n", vsize >> 10);
		seq_printf(m, "total_vm_kb: %lu\n", total_vm << (PAGE_SHIFT - 10));
		seq_printf(m, "rss_kb: %lu\n", rss << (PAGE_SHIFT - 10));
		for (i = 0; i < R_NKINDS; i++)
			seq_printf(m, "regions_%s: %lu %lu\n", kind_name[i],
				   count[i], bytes[i] >> 10);
	}

	seq_printf(m, "min_flt: %lu\n", min);
	seq_printf(m, "maj_flt: %lu\n", maj);

live:
	seq_printf(m, "live_secs: %llu\n", (ktime_get_ns() - since) / NSEC_PER_SEC);
	seq_printf(m, "live_min_flt: %lld\n", atomic64_read(&live_minor));
	seq_printf(m, "live_maj_flt: %lld\n", atomic64_read(&live_major));
	seq_printf(m, "live_anon_flt: %lld\n", atomic64_read(&live_anon));
	seq_printf(m, "live_file_flt: %lld\n", atomic64_read(&live_file));
	seq_printf(m, "live_err_flt: %lld\n", atomic64_read(&live_error));
	seq_printf(m, "probe_missed: %d\n", fault_probe.nmissed);
	return 0;
}

static int stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, stats_show, NULL);
}

static const struct proc_ops stats_ops = {
	.proc_open	= stats_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

/* ---------- /proc/pfcount/maps ---------- */

static int maps_show(struct seq_file *m, void *v)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	int ret;

	mutex_lock(&target_lock);
	task = target_task();
	mutex_unlock(&target_lock);
	if (!task)
		return 0;

	mm = get_task_mm(task);
	put_task_struct(task);
	if (!mm)
		return 0;

	ret = mmap_read_lock_killable(mm);
	if (ret) {
		mmput(mm);
		return ret;
	}
	{
		VMA_ITERATOR(vmi, mm, 0);

		for_each_vma(vmi, vma) {
			const char *name;
			enum region_kind k = classify(vma, &name);
			vm_flags_t f = vma->vm_flags;

			seq_printf(m, "%08lx-%08lx %c%c%c%c %8lu %-7s ",
				   vma->vm_start, vma->vm_end,
				   f & VM_READ ? 'r' : '-',
				   f & VM_WRITE ? 'w' : '-',
				   f & VM_EXEC ? 'x' : '-',
				   f & VM_MAYSHARE ? 's' : 'p',
				   (vma->vm_end - vma->vm_start) >> 10,
				   kind_name[k]);
			if (vma->vm_file)
				seq_file_path(m, vma->vm_file, "\n");
			else if (name)
				seq_puts(m, name);
			seq_putc(m, '\n');
		}
	}
	mmap_read_unlock(mm);
	mmput(mm);
	return 0;
}

static int maps_open(struct inode *inode, struct file *file)
{
	return single_open(file, maps_show, NULL);
}

static const struct proc_ops maps_ops = {
	.proc_open	= maps_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

/* ---------- /proc/pfcount/target ---------- */

static int target_show(struct seq_file *m, void *v)
{
	mutex_lock(&target_lock);
	seq_printf(m, "%d\n", target_pid ? pid_nr(target_pid) : 0);
	mutex_unlock(&target_lock);
	return 0;
}

static int target_open(struct inode *inode, struct file *file)
{
	return single_open(file, target_show, NULL);
}

static ssize_t target_write(struct file *file, const char __user *buf,
			    size_t len, loff_t *off)
{
	int nr, ret;

	ret = kstrtoint_from_user(buf, len, 10, &nr);
	if (ret)
		return ret;
	mutex_lock(&target_lock);
	ret = set_target(nr);
	mutex_unlock(&target_lock);
	return ret ? ret : len;
}

static const struct proc_ops target_ops = {
	.proc_open	= target_open,
	.proc_read	= seq_read,
	.proc_write	= target_write,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

/* ---------- init / exit ---------- */

static int __init pfcount_init(void)
{
	int ret;

	ret = register_kretprobe(&fault_probe);
	if (ret) {
		pr_err("pfcount: register_kretprobe(handle_mm_fault) failed: %d\n", ret);
		return ret;
	}

	pf_dir = proc_mkdir("pfcount", NULL);
	if (!pf_dir ||
	    !proc_create("target", 0644, pf_dir, &target_ops) ||
	    !proc_create("stats", 0444, pf_dir, &stats_ops) ||
	    !proc_create("maps", 0444, pf_dir, &maps_ops)) {
		ret = -ENOMEM;
		goto err;
	}

	if (pid) {
		mutex_lock(&target_lock);
		ret = set_target(pid);
		mutex_unlock(&target_lock);
		if (ret) {
			pr_err("pfcount: pid %d not found\n", pid);
			goto err;
		}
	}

	pr_info("pfcount: loaded, probe on %s\n", fault_probe.kp.symbol_name);
	return 0;

err:
	proc_remove(pf_dir);
	unregister_kretprobe(&fault_probe);
	return ret;
}

static void __exit pfcount_exit(void)
{
	proc_remove(pf_dir);
	unregister_kretprobe(&fault_probe);
	mutex_lock(&target_lock);
	set_target(0);
	mutex_unlock(&target_lock);
	pr_info("pfcount: unloaded\n");
}

module_init(pfcount_init);
module_exit(pfcount_exit);
