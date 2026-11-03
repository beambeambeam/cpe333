# CPE333 Mini-Project 02 – Page-Fault Counter

- [Assignment instructions](instructions.md)
- [Experiment report](REPORT.md) – design, findings, results, how to reproduce

`pfcount` is a kernel module. `pfstat` is its command-line front end. Give it a PID and it reports:

- number of memory regions (VMAs)
- total virtual address space
- minor and major page faults

It also checks each number against `/proc/<pid>/maps`, `/proc/<pid>/stat` and `getrusage()`.

## Layout

| Path | What |
|---|---|
| `module/pfcount.c` | Kernel module. Walks the VMAs and counts faults. |
| `tools/pfstat` | Python CLI: report, watch, and run modes. |
| `tests/faultgen.c` | Test program that causes a known number of faults. |
| `tests/threadexit.c`, `tests/regress.sh` | Regression checks for the module (`sudo sh tests/regress.sh`). |
| `scripts/experiments.sh` | Runs every experiment in the report, output to `results/`. |
| `results/` | Raw output of the last experiment run. |
| `Makefile` | Build, load, demo, and experiments targets. Runs inside Linux. |
| `vm.sh` | macOS helper for the Lima Ubuntu VM. |

## Quick start (macOS)

The module needs a Linux kernel, so it runs in an Ubuntu 24.04 VM through [Lima](https://lima-vm.io). This folder is mounted into the VM read-write.

```sh
./vm.sh setup        # one time: create the VM, install headers + gcc
./vm.sh make demo    # build, load the module, run the fault demo
./vm.sh make experiments   # rerun all report experiments into results/
./vm.sh shell        # shell inside the VM, in this folder
```

On native Linux, skip `vm.sh` and run `make demo`.

## Usage (inside the VM)

```sh
make load                              # build + insmod
sudo tools/pfstat <pid>                # one-shot report, checked against /proc
sudo tools/pfstat <pid> --maps         # also list every region
sudo tools/pfstat <pid> -w 0.5         # live: one line every 0.5 s with fault rates
sudo tools/pfstat -- <cmd> [args...]   # run cmd and count every fault it takes
sudo tools/pfstat -f -- <cmd> [args...]  # same, plus faults of cmd's child processes
make unload
```

Raw interface (root only: the files can show any process, so normal users cannot read them):

```sh
echo <pid> | sudo tee /proc/pfcount/target
sudo cat /proc/pfcount/stats
sudo cat /proc/pfcount/maps
echo 1 | sudo tee /sys/module/pfcount/parameters/follow   # include descendants
```

## How it works

**Regions and address space.** `get_task_mm()` gets the target's `mm_struct`. The module takes `mmap_read_lock` and walks every `vm_area_struct` with `for_each_vma`. Region count is the number of VMAs. Total size is the sum of `vm_end - vm_start`. That sum must equal `mm->total_vm` (`VmSize`). Each VMA is also put in a group: file, anon, heap, stack, or special (`[vdso]`, `[vvar]`).

**Faults – two sources.**

1. *Lifetime*: `task->min_flt` and `task->maj_flt`, summed over all threads, plus counts from threads that already exited (`signal->min_flt`). `/proc/<pid>/stat` uses the same sum, read under the same `signal->stats_lock` seqlock.
2. *Live*: a kretprobe on `handle_mm_fault()`, the function every user page fault goes through. It counts faults only for the target's thread group, from the moment you select it. It sorts each fault as minor or major and as anonymous or file-backed. It uses the same rule as the kernel's `mm_account_fault()`. Retried faults count once, and failed faults are counted separately.

A kprobe on `acct_collect()` (called by `do_exit()` with `group_dead` set for the last thread only) saves the region count, the size at exit, and the peak size (`hiwater_vm`) when the target exits. Short programs that exit before the first sample still get a result.

The hint names the `exceptions:page_fault_user` tracepoint. That tracepoint exists only on x86, and this VM is arm64 (Apple Silicon). Also, the tracepoint fires before the fault is handled, so it cannot tell minor from major. The kretprobe works on every arch and sees the result.

**Minor vs. major.** A minor fault maps a page that is already in memory, such as a zero-filled anonymous page or a page already in the page cache. A major fault must wait for I/O.

## The demo

`make demo` runs:

```
faultgen sleep:1 anon:64 sleep:1 file:/tmp/pfcount-big.bin sleep:1 free sleep:1
```

- `anon:64` maps 64 MB and touches each 4 KB page. Expect about 16,384 minor faults and one new region.
- `file:...` evicts a 32 MB file from the page cache, maps it, and reads each page. `MADV_RANDOM` turns off readahead. Expect about 8,192 major faults.
- `free` unmaps both, and the region count drops.

`pfstat` prints one line per interval while it runs. At the end it compares its totals with `getrusage()` for the child. With `-f`, `wait4()` covers only descendants that were reaped by a `wait()`, so a higher pfcount total is expected when children are auto-reaped (`SIGCHLD` ignored).
