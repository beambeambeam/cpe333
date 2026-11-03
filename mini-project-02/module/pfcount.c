// SPDX-License-Identifier: GPL-2.0
/*
 * pfcount - per-process page-fault counter and address-space inspector.
 *
 * /proc/pfcount/target  (root rw)  write a PID to start monitoring, "0" to stop
 * /proc/pfcount/stats   (root r)   region count, address-space size, fault counts
 * /proc/pfcount/maps    (root r)   VMA list of the target, like /proc/<pid>/maps
 *
 * All files are root-only. They expose any selected process, including ones
 * whose own /proc/<pid>/maps the reader could not open.
 *
 * Two fault sources:
 *   lifetime  - task->min_flt / maj_flt summed over all threads, the same
 *               counters /proc/<pid>/stat and getrusage() report.
 *   live      - a kretprobe on handle_mm_fault() counts every fault the
 *               target takes after monitoring starts, split minor/major and
 *               anonymous/file-backed. Uses the same major/minor rule as
 *               mm_account_fault() in mm/memory.c.
 *
 * A kprobe on do_exit() saves the region count and address-space size when
 * the target's last thread exits, so short-lived programs still get a result.
 *
 * Module parameter "follow" (writable in /sys/module/pfcount/parameters/)
 * makes the live counters also include the target's descendant processes,
 * the same scope as getrusage() of a waited-for child.
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
#include <linux/rcupdate.h>
#include <linux/refcount.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CPE333 Mini-Project 2");
MODULE_DESCRIPTION("Per-process page-fault counter");

static int pid;
module_param(pid, int, 0444);
MODULE_PARM_DESC(pid, "PID to monitor at load time (optional)");

static bool follow;
module_param(follow, bool, 0644);
MODULE_PARM_DESC(follow, "Also count faults of the target's descendants");

/*
 * A session is one monitoring period of one target. set_target() replaces
 * the session; it never resets counters in place. A fault accepted by the
 * entry handler holds a reference to its session until the return handler
 * runs, so a fault that started under session A is counted in A even if B
 * was selected in between.
 */
struct pf_session {
	struct rcu_head rcu;
	refcount_t ref;
	/* Thread-group pid of the target. The reference keeps this struct pid
	 * alive, so a new process that reuses the number gets a different
	 * struct pid and never matches. */
	struct pid *pid;
	u64 since_ns;
	atomic64_t minor, major, anon, file, error;
	/* Snapshot taken by the do_exit() probe. */
	bool exit_valid;
	int exit_regions;
	unsigned long exit_vm, exit_peak_vm;	/* pages */
};

static DEFINE_MUTEX(target_lock);		/* serializes set_target() */
static struct pf_session __rcu *cur_session;

static struct proc_dir_entry *pf_dir;

static void session_free_rcu(struct rcu_head *head)
{
	struct pf_session *s = container_of(head, struct pf_session, rcu);

	put_pid(s->pid);
	kfree(s);
}

static void session_put(struct pf_session *s)
{
	if (s && refcount_dec_and_test(&s->ref))
		call_rcu(&s->rcu, session_free_rcu);
}

/* Returns a referenced current session or NULL. */
static struct pf_session *session_get(void)
{
	struct pf_session *s;

	rcu_read_lock();
	s = rcu_dereference(cur_session);
	if (s && !refcount_inc_not_zero(&s->ref))
		s = NULL;
	rcu_read_unlock();
	return s;
}

/* ---------- live fault counting ---------- */

struct fault_ctx {
	struct pf_session *sess;
	unsigned int flags;
	bool file;
};

/* True if current belongs to the target, or descends from it in follow mode.
 * Caller holds rcu_read_lock(). */
static bool is_tracked(struct pid *tg)
{
	struct task_struct *t;
	int depth = 0;

	if (task_tgid(current) == tg)
		return true;
	if (!READ_ONCE(follow))
		return false;

	for (t = current; t->pid > 1 && depth < 64; depth++) {
		t = rcu_dereference(t->real_parent);
		if (task_tgid(t) == tg)
			return true;
	}
	return false;
}

/* handle_mm_fault(struct vm_area_struct *vma, unsigned long address,
 *                 unsigned int flags, struct pt_regs *regs) */
static int fault_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct fault_ctx *ctx = (struct fault_ctx *)ri->data;
	struct vm_area_struct *vma;
	struct pf_session *s;

	rcu_read_lock();
	s = rcu_dereference(cur_session);
	if (!s || !is_tracked(s->pid) || !refcount_inc_not_zero(&s->ref)) {
		rcu_read_unlock();
		return 1; /* not our process: skip the return handler */
	}
	rcu_read_unlock();

	vma = (struct vm_area_struct *)regs_get_kernel_argument(regs, 0);
	ctx->sess = s;
	ctx->flags = (unsigned int)regs_get_kernel_argument(regs, 2);
	ctx->file = vma && vma->vm_file;
	return 0;
}

static int fault_return(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct fault_ctx *ctx = (struct fault_ctx *)ri->data;
	struct pf_session *s = ctx->sess;
	vm_fault_t ret = (vm_fault_t)regs_return_value(regs);

	/* A retried fault is handled again and counted on the final attempt. */
	if (ret & VM_FAULT_RETRY)
		goto out;
	if (ret & VM_FAULT_ERROR) {
		atomic64_inc(&s->error);
		goto out;
	}
	if ((ret & VM_FAULT_MAJOR) || (ctx->flags & FAULT_FLAG_TRIED))
		atomic64_inc(&s->major);
	else
		atomic64_inc(&s->minor);
	atomic64_inc(ctx->file ? &s->file : &s->anon);
out:
	session_put(s);
	return 0;
}

/* An instance still in flight at unregister time never reaches
 * fault_return(), so its session reference leaks at module unload. */
static struct kretprobe fault_probe = {
	.kp.symbol_name	= "handle_mm_fault",
	.entry_handler	= fault_entry,
	.handler	= fault_return,
	.data_size	= sizeof(struct fault_ctx),
	.maxactive	= 64,
};

/* do_exit() runs before exit_mm(), so current->mm is still valid here. The
 * probe runs in atomic context: read the counters without mmap_lock. */
static int exit_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct mm_struct *mm = current->mm;
	struct pf_session *s;

	rcu_read_lock();
	s = rcu_dereference(cur_session);
	if (s && task_tgid(current) == s->pid && mm &&
	    atomic_read(&current->signal->live) == 1) {
		s->exit_regions = READ_ONCE(mm->map_count);
		s->exit_vm = READ_ONCE(mm->total_vm);
		s->exit_peak_vm = max(READ_ONCE(mm->hiwater_vm), s->exit_vm);
		smp_store_release(&s->exit_valid, true);
	}
	rcu_read_unlock();
	return 0;
}

static struct kprobe exit_probe = {
	.symbol_name	= "do_exit",
	.pre_handler	= exit_pre,
};

/* ---------- target selection ---------- */

/* Caller holds target_lock. nr == 0 clears the target. */
static int set_target(int nr)
{
	struct pf_session *s = NULL, *old;
	struct task_struct *task;

	if (nr < 0)
		return -EINVAL;
	if (nr) {
		s = kzalloc(sizeof(*s), GFP_KERNEL);
		if (!s)
			return -ENOMEM;
		struct pid *p = find_get_pid(nr);

		task = p ? get_pid_task(p, PIDTYPE_PID) : NULL;
		put_pid(p);
		if (!task) {
			kfree(s);
			return -ESRCH;
		}
		s->pid = get_pid(task_tgid(task));
		put_task_struct(task);
		refcount_set(&s->ref, 1);	/* owned by cur_session */
		s->since_ns = ktime_get_ns();
	}

	old = rcu_replace_pointer(cur_session, s, lockdep_is_held(&target_lock));
	session_put(old);
	return 0;
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
	unsigned long total_vm = 0, peak_vm = 0, rss = 0;
	struct task_struct *task;
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma;
	char comm[TASK_COMM_LEN];
	int threads, i, ret = 0;
	struct pf_session *s = session_get();

	if (!s) {
		seq_puts(m, "pid: 0\nstate: idle\n");
		return 0;
	}

	seq_printf(m, "pid: %d\n", pid_nr(s->pid));
	task = get_pid_task(s->pid, PIDTYPE_TGID);
	if (!task) {
		seq_puts(m, "state: exited\n");
		if (smp_load_acquire(&s->exit_valid)) {
			seq_printf(m, "exit_regions: %d\n", s->exit_regions);
			seq_printf(m, "exit_vsize_kb: %lu\n", s->exit_vm << (PAGE_SHIFT - 10));
			seq_printf(m, "peak_vsize_kb: %lu\n", s->exit_peak_vm << (PAGE_SHIFT - 10));
		}
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
			session_put(s);
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
		peak_vm = max(mm->hiwater_vm, total_vm);
		rss = get_mm_rss(mm);
		mmap_read_unlock(mm);
		mmput(mm);

		seq_printf(m, "regions: %lu\n", regions);
		seq_printf(m, "vsize_kb: %lu\n", vsize >> 10);
		seq_printf(m, "total_vm_kb: %lu\n", total_vm << (PAGE_SHIFT - 10));
		seq_printf(m, "peak_vsize_kb: %lu\n", peak_vm << (PAGE_SHIFT - 10));
		seq_printf(m, "rss_kb: %lu\n", rss << (PAGE_SHIFT - 10));
		for (i = 0; i < R_NKINDS; i++)
			seq_printf(m, "regions_%s: %lu %lu\n", kind_name[i],
				   count[i], bytes[i] >> 10);
	}

	seq_printf(m, "min_flt: %lu\n", min);
	seq_printf(m, "maj_flt: %lu\n", maj);

live:
	seq_printf(m, "live_secs: %llu\n", (ktime_get_ns() - s->since_ns) / NSEC_PER_SEC);
	seq_printf(m, "live_min_flt: %lld\n", atomic64_read(&s->minor));
	seq_printf(m, "live_maj_flt: %lld\n", atomic64_read(&s->major));
	seq_printf(m, "live_anon_flt: %lld\n", atomic64_read(&s->anon));
	seq_printf(m, "live_file_flt: %lld\n", atomic64_read(&s->file));
	seq_printf(m, "live_err_flt: %lld\n", atomic64_read(&s->error));
	seq_printf(m, "follow: %d\n", READ_ONCE(follow));
	seq_printf(m, "probe_missed: %d\n", fault_probe.nmissed);
	session_put(s);
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
	struct pf_session *s = session_get();
	int ret;

	if (!s)
		return 0;
	task = get_pid_task(s->pid, PIDTYPE_TGID);
	session_put(s);
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
	struct pf_session *s = session_get();

	seq_printf(m, "%d\n", s ? pid_nr(s->pid) : 0);
	session_put(s);
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
	ret = register_kprobe(&exit_probe);
	if (ret) {
		pr_err("pfcount: register_kprobe(do_exit) failed: %d\n", ret);
		unregister_kretprobe(&fault_probe);
		return ret;
	}

	pf_dir = proc_mkdir("pfcount", NULL);
	if (!pf_dir ||
	    !proc_create("target", 0600, pf_dir, &target_ops) ||
	    !proc_create("stats", 0400, pf_dir, &stats_ops) ||
	    !proc_create("maps", 0400, pf_dir, &maps_ops)) {
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
	unregister_kprobe(&exit_probe);
	unregister_kretprobe(&fault_probe);
	return ret;
}

static void __exit pfcount_exit(void)
{
	proc_remove(pf_dir);
	unregister_kprobe(&exit_probe);
	unregister_kretprobe(&fault_probe);
	mutex_lock(&target_lock);
	set_target(0);
	mutex_unlock(&target_lock);
	rcu_barrier();	/* wait for session_free_rcu() before the code goes away */
	pr_info("pfcount: unloaded\n");
}

module_init(pfcount_init);
module_exit(pfcount_exit);
