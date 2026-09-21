# PS05 universal step-by-step guide

This guide prepares PS05 and hands the experiment work to the person who will
run it. It explains what to inspect, which commands to use, which screenshots
to capture, and what to give the report owner.

## 0. Handoff boundary

A person receiving this project receives only the CPE333 repository:

```text
CPE333/
├── ps02/
├── ps03/
├── ps04/
└── ps05/
```

The CPE333 repository contains the assignment, this experiment guide, the
observation checklist, and the evidence folder. It does not contain the report
source or report PDF.

Keep project files in CPE333 and commit them to the CPE333 Git repository. Keep
experiment screenshots in `ps05/results/`; this directory is part of the
handoff and starts with an empty `.gitkeep` file. The report owner can copy the
screenshots into the separate submission workspace later.

Prepare the evidence directory from the CPE333 root:

```bash
cd /path/to/CPE333
export RESULTS_DIR="$PWD/ps05/results"
mkdir -p "$RESULTS_DIR"
```

Nothing in this guide has been run for the report. Replace every placeholder in
`notes/README.md` with output from the selected machines.

## 1. Choose the platforms

| Platform         | Use it for                         | Notes                                                                        |
| ---------------- | ---------------------------------- | ---------------------------------------------------------------------------- |
| Debian/Ubuntu    | All Linux sections                 | Recommended                                                                  |
| WSL2 Ubuntu      | Linux commands and swap experiment | GUI evidence needs WSLg or a Linux VM                                        |
| Fedora/Arch      | All Linux sections                 | Use the package commands below                                               |
| Linux desktop VM | All Linux evidence                 | Safest option for screenshots                                                |
| Windows          | Windows Resource Monitor section   | Physical Windows or a Windows VM                                             |
| macOS            | Preparation only                   | `vm_stat` is not a replacement for `free`/`vmstat` or Resource Monitor |

The report must identify which operating system produced each screenshot. Do
not combine Linux values from one machine with Windows values from another
without recording both environments.

The Linux swap experiment needs `sudo` and enough free disk space. Use a
separate temporary swap file rather than disabling the machine's existing swap
area.

## 2. Install and verify Linux tools

### Debian, Ubuntu, and WSL2 Ubuntu

```bash
sudo apt update
sudo apt install -y procps
```

### Fedora

```bash
sudo dnf install -y procps-ng
```

### Arch Linux

```bash
sudo pacman -S --needed procps-ng
```

Check the tools from `CPE333/ps05/`:

```bash
cd /path/to/CPE333/ps05
{
    printf '$ command -v free\n'
    command -v free
    printf '$ command -v vmstat\n'
    command -v vmstat
    printf '$ command -v swapon\n'
    command -v swapon
    printf '$ command -v mkswap\n'
    command -v mkswap
    printf '$ free --version\n'
    free --version 2>/dev/null || true
    printf '$ vmstat --version\n'
    vmstat --version 2>/dev/null || true
    printf '$ uname -a\n'
    uname -a
    printf '$ cat /etc/os-release\n'
    cat /etc/os-release 2>/dev/null || true
} | tee /tmp/ps05-environment.txt
```

Save the complete output as `/tmp/ps05-environment.txt` and capture it as:

```text
$RESULTS_DIR/00_environment.png
```

A suitable capture shows the Linux distribution, kernel, architecture, tool
locations, and tool versions. Keep the command and its output visible.

## 3. Explain virtual memory

This part is a written comparison, not a command-output experiment. Use the
course material and reliable OS documentation. Record the sources in
`notes/README.md`.

Cover these points for both operating systems:

- Per-process virtual address spaces and protection between processes.
- Virtual-to-physical address translation through page tables and the MMU.
- Pages, frames, demand paging, and page faults.
- What happens when a referenced page is not currently resident in RAM.
- The role of disk-backed swap or paging storage.
- The difference between virtual memory, physical RAM, and disk-backed paging
  space.

Discuss Linux specifically:

- Linux memory is divided into virtual address spaces backed by physical pages
  when needed.
- Anonymous pages can be reclaimed or written to a configured swap area.
- A swap area can be a partition or a file and is inspected with commands such
  as `swapon --show`, `cat /proc/swaps`, and `free`.
- Linux also uses free RAM for caches; cached memory is normally reclaimable and
  is not the same as unavailable memory.

Discuss Windows specifically:

- Each process has a protected virtual address space managed by the Windows
  memory manager.
- Windows tracks committed virtual memory, physical working sets, page faults,
  and pagefile-backed storage.
- The Resource Monitor Memory tab exposes working-set and hard-fault data as
  well as physical-memory categories.
- A pagefile is paging storage; it is not extra physical RAM.

Then compare the systems directly. Separate common paging concepts from
implementation and terminology differences. Do not claim that a displayed
metric has the same definition on both systems without checking its
platform-specific documentation.

Useful Linux observations to include in the discussion:

```bash
getconf PAGE_SIZE
cat /proc/meminfo | head -20
cat /proc/swaps
free -h
```

These commands are supplementary. The required `free` and `vmstat` evidence is
captured in later sections.

## 4. Increase and decrease Linux swap space

### 4.1. Safety rules

This experiment changes active kernel memory configuration. Run it on a Linux
VM or a disposable lab system when possible.

- Work as a normal user and use `sudo` only for the commands that need it.
- Do not run `swapoff` on an unknown production swap device.
- Use a dedicated path such as `/swapfile-ps05` and record that path.
- Check that the path does not already exist before creating it.
- Stop if `swapoff` reports that the system cannot move pages out of swap.
- Do not edit `/etc/fstab` unless persistent swap is explicitly required.

### 4.2. Record the initial state

From `CPE333/ps05/`:

```bash
printf '$ free -h\n'
free -h
printf '$ swapon --show\n'
swapon --show
printf '$ cat /proc/swaps\n'
cat /proc/swaps
printf '$ df -h /\n'
df -h /
```

Capture the initial swap total, used amount, free amount, active swap devices,
and available disk space as:

```text
$RESULTS_DIR/01_swap_before.png
```

If the system has no swap, record that fact. The temporary swap file will still
make the increase/decrease comparison visible.

### 4.3. Create and enable a temporary swap file

Choose a size suitable for the VM. The following example uses 512 MiB. First
check that `/swapfile-ps05` is not already present:

```bash
SWAPFILE=/swapfile-ps05
printf 'swap file: %s\n' "$SWAPFILE"
sudo test ! -e "$SWAPFILE" && echo 'path is unused'
df -h /
```

If the path exists, choose another path and record it. Do not overwrite an
existing file.

Create the file with `fallocate`:

```bash
sudo fallocate -l 512M "$SWAPFILE"
```

If `fallocate` is unavailable or the filesystem rejects the resulting file,
use this Linux fallback instead:

```bash
sudo dd if=/dev/zero of="$SWAPFILE" bs=1M count=512 status=progress
```

Set the required permissions, format the file as swap, and enable it:

```bash
sudo chmod 600 "$SWAPFILE"
sudo mkswap "$SWAPFILE"
sudo swapon "$SWAPFILE"
```

Capture the creation commands, `mkswap` result, and any warnings as:

```text
$RESULTS_DIR/02_swap_create.png
```

Verify the increase:

```bash
printf '$ free -h\n'
free -h
printf '$ swapon --show\n'
swapon --show
printf '$ cat /proc/swaps\n'
cat /proc/swaps
```

Capture the complete verification output as:

```text
$RESULTS_DIR/03_swap_after_increase.png
```

Explain that adding an enabled swap file increases the total configured swap
space. It does not increase physical RAM. The `used` value may remain zero if
nothing has needed to move pages there.

### 4.4. Disable and remove the temporary swap file

Only disable the known file created for this experiment:

```bash
sudo swapoff "$SWAPFILE"
sudo rm "$SWAPFILE"
```

Verify that the system returned to its original swap configuration:

```bash
printf '$ free -h\n'
free -h
printf '$ swapon --show\n'
swapon --show
printf '$ cat /proc/swaps\n'
cat /proc/swaps
```

Capture the decreased/cleaned-up state as:

```text
$RESULTS_DIR/04_swap_after_decrease.png
```

Compare the before, increased, and decreased totals in the report. Explain why
`swapoff` needs enough available memory: pages currently in the temporary swap
area must be brought back into RAM or moved to another active swap area.

If `swapoff` fails, do not force it. Record the error, leave the system stable,
and ask the lab supervisor or use a VM with more memory. If a persistent entry
was added to `/etc/fstab`, remove only the exact line for the temporary path
before deleting the file, then verify the file is gone.

## 5. Monitor Linux memory with `free`

### 5.1. Required base command

Run the command named in the assignment. Use a readable terminal width and
keep the command in the capture:

```bash
printf '$ free\n'
free
printf '$ free -h\n'
free -h
```

Capture both outputs as:

```text
$RESULTS_DIR/05_free.png
```

The exact values depend on the machine and change as processes run. Copy the
values into `notes/README.md`; never use values from this guide as results.

### 5.2. Explain the base output

Use `free --help` and `man free` for the installed version. Explain every
heading that appears in the captured output. On a current procps version, the
main rows commonly include:

- `Mem` – physical memory statistics.
- `total` – total memory visible to the operating system.
- `used` – memory classified as used by the command's version-specific
  calculation.
- `free` – memory currently unused.
- `shared` – memory used mainly by shared-memory filesystems such as `tmpfs`.
- `buff/cache` – buffers and filesystem/page cache; much of this can be
  reclaimed when applications need memory.
- `available` – an estimate of memory available for new applications without
  swapping, not simply the `free` column.
- `Swap` – configured swap totals, used swap, and unused swap.

If `free -w` is used, explain the separate `buffers` and `cache` columns rather
than calling the combined column `buff/cache`. State the displayed unit
(`KiB`, `MiB`, human-readable units, or another unit) and the installed procps
version.

Do not assume `used + free` equals the amount available to applications. Use
the command's definition of `available` and explain the role of reclaimable
cache.

### 5.3. Select two useful `free` arguments

First capture the installed help text or the relevant part of it:

```bash
free --help
```

Choose two arguments that are useful for this experiment. The following are
portable examples; use only options shown by the local help output:

```bash
printf '$ free -h -w\n'
free -h -w

printf '$ free -s 2 -c 3\n'
free -s 2 -c 3
```

Possible discussion choices:

- `-h` – scale values into human-readable units.
- `-w` – show buffers and cache as separate columns.
- `-s 2` – repeat the display every two seconds.
- `-c 3` – stop after three displays when used with `-s`.
- `-t` – add a total row for memory plus swap.

If using `-s` and `-c` as a pair, explain both the interval and the sample
count. If using `-h` and `-w` as a pair, compare the readable units and the
separate cache fields with the base output.

Capture the help text and complete sample outputs as:

```text
$RESULTS_DIR/06_free_arguments.png
```

Describe what each selected argument does and explain the numerical changes or
lack of changes in the sample output.

## 6. Monitor Linux memory with `vmstat`

### 6.1. Required base command

Run the command named in the assignment:

```bash
printf '$ vmstat\n'
vmstat
```

For a short time-series capture, also run:

```bash
printf '$ vmstat 1 5\n'
vmstat 1 5
```

The first `vmstat` report is commonly an average since boot, while later rows
from an interval command describe the preceding interval. Confirm the exact
wording with `man vmstat` for the installed procps version.

Capture the base command and the complete short time series as:

```text
$RESULTS_DIR/07_vmstat.png
```

### 6.2. Explain every `vmstat` column

Use the headings printed by the local command. A standard Linux procps output
has these groups:

| Group      | Common columns                          | What to explain                                                   |
| ---------- | --------------------------------------- | ----------------------------------------------------------------- |
| `procs`  | `r`, `b`                            | Runnable processes and processes blocked in uninterruptible sleep |
| `memory` | `swpd`, `free`, `buff`, `cache` | Virtual memory used for swap, free memory, buffers, and cache     |
| `swap`   | `si`, `so`                          | Memory swapped in from and out to disk during the interval        |
| `io`     | `bi`, `bo`                          | Blocks received from and sent to block devices                    |
| `system` | `in`, `cs`                          | Interrupts and context switches per second                        |
| `cpu`    | `us`, `sy`, `id`, `wa`, `st`  | User time, kernel time, idle time, I/O wait, and stolen time      |

Some options or versions add `inact` and `active`, change memory-unit display,
or show extra CPU columns. Explain every extra heading actually shown rather
than relying only on the table above. State the sampling interval and units.

Interpret the values as measurements, not permanent machine properties. For
example, a high `si`/`so` indicates paging activity during the sampled period;
zero values mean no swap transfer was observed during that period, not that the
system has no configured swap.

Summarize every numerical column in a paragraph. Include the first-row
baseline/average and the later interval values, and relate relevant values to
physical memory and swap use.

### 6.3. Select two useful `vmstat` arguments

Capture the installed help text first:

```bash
vmstat --help
```

Two useful VM-focused choices are:

```bash
printf '$ vmstat -a 1 5\n'
vmstat -a 1 5

printf '$ vmstat -s\n'
vmstat -s
```

- `-a` reports active and inactive memory, which gives more detail about the
  memory state than the default buffer/cache view.
- `-s` displays a summary of memory and event counters instead of the compact
  interval table.

If the local help output uses different options, choose two options it lists
and explain the installed version's output. `-S M` is another useful choice
for reporting memory in mebibytes:

```bash
vmstat -S M 1 3
```

Do not call an option supported by another operating system version available
unless it appears in the help output on the experiment machine.

Capture the help text, commands, and complete outputs as:

```text
$RESULTS_DIR/08_vmstat_arguments.png
```

Explain every new heading and each numerical value introduced by the selected
arguments. Keep the summary separate from the base `vmstat` paragraph.

## 7. Monitor Windows memory with Resource Monitor

Run this section on Windows:

1. Open PowerShell.
2. Run `resmon`.
3. Select the **Memory** tab.
4. Maximize the window if needed so the labels and numerical values are
   readable.
5. Capture the process table and the **Physical Memory** graph/panel.

The Memory tab commonly shows process columns such as image name, PID, hard
faults per second, working set, shareable memory, and private memory. It also
commonly shows physical-memory categories such as hardware reserved, in use,
modified, standby, and free. The exact labels depend on the Windows version.

Explain the information actually visible in the screenshot, including:

- The meaning of the physical-memory categories.
- The relationship between in-use, available, cached/standby, and reserved
  memory.
- The process columns and their units.
- What hard faults indicate and why they are not identical to a software bug.
- Any commit, working-set, or paging values shown elsewhere in the interface.

Record every numerical value in the **Physical Memory** section, preserving the
units and labels. Write one paragraph summarizing those values. Do not infer a
value hidden by a collapsed panel or cropped from the screenshot.

Save the screenshot as:

```text
$RESULTS_DIR/09_windows_resource_monitor.png
```

Record the Windows edition/version and how Resource Monitor was opened in
`notes/README.md`.

## 8. Check the evidence set

From the CPE333 root:

```bash
find "$RESULTS_DIR" -maxdepth 1 -type f -print | sort
```

Required evidence files:

```text
00_environment.png
01_swap_before.png
02_swap_create.png
03_swap_after_increase.png
04_swap_after_decrease.png
05_free.png
06_free_arguments.png
07_vmstat.png
08_vmstat_arguments.png
09_windows_resource_monitor.png
```

Names are case-sensitive. Every screenshot should show the exact command or
application title, the complete relevant output, and enough context to identify
the machine. Check that PNG files open correctly before handing them off.

If a required section could not be run, record the limitation in the notes and
inform the report owner. Do not create a screenshot containing guessed output.

## 9. Hand the evidence to the report owner

The CPE333 repository does not contain `report.tex`. If the group has a
separate report workspace, copy the evidence there using its documented
layout:

```bash
cp "$RESULTS_DIR"/*.png /path/to/private-report-workspace/ps05/results/
```

Also provide:

- `ps05/notes/README.md` with all placeholders filled in.
- `/tmp/ps05-environment.txt`.
- Exact Linux and Windows versions.
- Full outputs for `free`, `vmstat`, and the selected help commands.
- The swap-file size, path, before/after values, and cleanup result.
- Sources used for the Linux/Windows virtual-memory comparison.

The report owner adds the screenshots, writes the explanations and numerical
summaries, compiles the PDF, and submits it to LEB2. The report owner should
keep the report source and generated PDF outside this repository.

## 10. Git handoff

Before handing over the CPE333 changes:

```bash
cd /path/to/CPE333
git status --short
git diff --check
```

Commit the assignment and handoff files, including `ps05/problem-session.md`,
`README.md`, `guide.md`, `notes/README.md`, `.gitignore`, and
`results/.gitkeep`. Do not commit generated report files or system-generated
`.DS_Store` files.

## 11. Troubleshooting

### `free` or `vmstat` is missing

Install the `procps`/`procps-ng` package for the selected Linux distribution,
then verify with `command -v free` and `command -v vmstat`.

### `sudo swapon` rejects the file

Check that the file has mode `600`, was created on a supported local
filesystem, and was initialized with `mkswap`. Recreate only the dedicated
experiment file. Do not change an existing system swap device.

### `swapoff` reports insufficient memory

Stop the experiment and leave the system stable. Add RAM or use a larger VM,
or ask the supervisor. Do not force `swapoff` on an active system.

### The swap total did not change

Check `swapon --show` and `/proc/swaps`. The file must be successfully enabled,
and the `free` output must be captured after `swapon` completes.

### `vmstat` output changes between runs

That is expected. CPU load, processes, filesystem activity, and the sampling
interval affect the values. Record the exact interval and the machine state.

### Resource Monitor will not open

Run `resmon` from PowerShell or press `Win+R`, enter `resmon`, and press Enter.
Use a Windows machine or VM; Linux tools are not substitutes for the required
Windows evidence.

### Screenshot is unreadable

Maximize the terminal/window, increase font size, and capture only the relevant
area without cropping command names, headings, units, or values.
