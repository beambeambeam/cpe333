# PS03 universal step-by-step guide

This guide prepares and runs PS03 on Linux, macOS, WSL2, and Windows. It also
explains which screenshots to capture and how to hand the evidence to the
person who owns the report.

## 0. Handoff boundary

A person receiving this project receives **only the CPE333 repository**:

```text
CPE333/
├── ps02/
└── ps03/
```

The CPE333 repository contains the source programs, CSV cases, shell scripts,
assignment instructions, experiment guide, notes, and the PS03 evidence
folder. It does not contain the report source or report PDF.

Keep project files in CPE333 and commit them to the CPE333 Git repository. Keep
the experiment screenshots in `ps03/results/`; this directory is part of the
CPE333 handoff and starts with an empty `.gitkeep` file. The report owner can
copy those screenshots into the separate submission workspace later.

Prepare the evidence folder from the CPE333 root:

```bash
cd /path/to/CPE333
export RESULTS_DIR="$PWD/ps03/results"
mkdir -p "$RESULTS_DIR"
```

## 1. Choose the platform

| Platform | Use it for | Notes |
|---|---|---|
| Debian/Ubuntu | All Linux experiments and report preparation | Recommended |
| WSL2 Ubuntu | Linux commands and C scheduler | GUI needs WSLg or a VM |
| Fedora/Arch | Linux experiments and C scheduler | Use the package commands below |
| macOS | C scheduler and shell preparation | Use Linux for required Linux evidence |
| Windows | Windows Task Manager experiment | Use WSL2 for POSIX C programs |

Native Windows does not provide the POSIX `fork()` API used by the C programs.
Do not build them with native Visual Studio. Use WSL2 or a Linux VM.

### Which image should run?

Use the course-provided Debian image when available. If no image was provided,
use a Debian stable installer image with a desktop environment:

- Intel or AMD computer/VM – Debian `amd64` ISO.
- ARM computer/VM, including Apple Silicon – Debian `arm64` ISO if supported
  by the VM software.
- Windows computer – Debian or Ubuntu inside WSL2 for command-line work; use a
  desktop Linux VM for the GUI screenshot if WSL has no graphical session.

A Debian desktop VM is the safest single choice because it supports Linux
commands, GNOME System Monitor, screenshots, and LaTeX. Suggested VM resources:

- 2 CPU cores
- 4 GB RAM
- 20 GB disk
- A desktop environment, preferably GNOME

Ubuntu Desktop LTS, Fedora Workstation, or Arch with a desktop environment are
valid alternatives. Record the exact distribution, release, architecture, and
tool versions in the report.

## 2. Install prerequisites

Install a C compiler, `make`, process commands, and LaTeX if the PDF will be
built on that machine.

### Debian, Ubuntu, and WSL2 Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential procps
sudo apt install -y texlive-latex-base texlive-latex-recommended texlive-latex-extra
```

For the Linux GUI experiment on a desktop installation:

```bash
sudo apt install -y gnome-system-monitor
```

### Fedora

```bash
sudo dnf install -y gcc make procps-ng
sudo dnf install -y texlive-scheme-basic texlive-collection-latexrecommended texlive-collection-latexextra
sudo dnf install -y gnome-system-monitor
```

If the GUI package is unavailable, use the process-monitoring tool supplied by
the installed desktop environment and record its exact name.

### Arch Linux

```bash
sudo pacman -S --needed base-devel procps-ng
sudo pacman -S --needed texlive-basic texlive-latexextra
sudo pacman -S --needed gnome-system-monitor
```

### macOS

Install Apple's command-line tools:

```bash
xcode-select --install
```

Check them:

```bash
cc --version
make --version
```

Install MacTeX only if the PDF is built on macOS:

```bash
brew install --cask mactex
```

macOS has Activity Monitor instead of GNOME System Monitor. Use Activity
Monitor for preparation, but collect the required Linux evidence on Linux
unless the instructor approves a substitute.

### Windows

Install WSL2 from an elevated PowerShell window if it is not already present:

```powershell
wsl --install -d Ubuntu
```

Reboot if requested, open Ubuntu, and follow the Debian/Ubuntu instructions.
Task Manager is already included with Windows. MiKTeX or TeX Live is optional
if the PDF is built inside Windows instead of WSL2.

## 3. Verify the CPE333 checkout

From the CPE333 repository:

```bash
cd /path/to/CPE333
printf 'repository: '
git rev-parse --show-toplevel
git status --short
```

Enter PS03:

```bash
cd ps03
ls -la
find src -maxdepth 1 -type f -print | sort
```

Required PS03 files:

```text
Makefile
case1.csv
case2.csv
case3.csv
problem-session.md
guide.md
src/PS3.c
src/ss1_1.sh
src/ss1_2.sh
results/.gitkeep
notes/README.md
```

Check tools:

```bash
command -v cc
command -v make
```

Make the scripts executable:

```bash
make scripts
ls -l src/ss1_1.sh src/ss1_2.sh
```

Both scripts should show an executable bit, such as `-rwxr-xr-x`.

## 4. Build and test the scheduler

Run from `CPE333/ps03/`, not from `src/`:

```bash
make clean
make
make check | tee /tmp/ps03-check.txt
```

Expected output:

```text
== case1.csv ==
[0-2]: P1
[2-4]: P2
[4-5]: P3
[5-7]: P2
[7-12]: P1
== case2.csv ==
[0-2]: P1
[2-5]: IDLE
[5-6]: P2
[6-7]: P4
[7-9]: P2
[9-12]: P3
== case3.csv ==
[0-3]: P2
[3-5]: P1
[5-7]: P2
[7-8]: P3
```

Run cases individually when capturing evidence:

```bash
./bin/PS3 case1.csv
./bin/PS3 case2.csv
./bin/PS3 case3.csv
```

The scheduler evaluates the ready queue at every integer time. It selects the
smallest remaining burst and uses the smaller process ID as a tie-breaker.
Adjacent slots for one process are combined into one interval.

If output differs:

1. Confirm the current directory is `CPE333/ps03/`.
2. Run `make clean && make`.
3. Check that the CSV header and rows were not edited.
4. Compare the output with the arrival and tie rules in `problem-session.md`.

## 5. Capture the environment evidence

Run this on the system supplying the Linux evidence. Use Debian details for the
final report when Debian is the selected system.

```bash
cd /path/to/CPE333/ps03
{
    printf '$ uname -a\n'
    uname -a
    if [ -f /etc/os-release ]; then
        printf '$ cat /etc/os-release\n'
        cat /etc/os-release
    elif command -v sw_vers >/dev/null 2>&1; then
        printf '$ sw_vers\n'
        sw_vers
    fi
    printf '$ cc --version\n'
    cc --version | head -1
    printf '$ make --version\n'
    make --version | head -1
    printf '$ ulimit -u\n'
    ulimit -u
} | tee /tmp/ps03-environment.txt
```

Capture a terminal screenshot containing the OS, architecture/version,
compiler, Make version, and process limit. Save it as:

```text
$RESULTS_DIR/00_environment.png
```

Do not use a macOS screenshot in a report claiming Debian execution.

### Universal screenshot methods

Capture only the relevant terminal or window. Keep the command, PID, output, or
application title visible.

- GNOME Linux – press `Print Screen` or use `gnome-screenshot -a`.
- KDE Linux – use Spectacle and select a rectangular region.
- macOS – press `Command+Shift+4`.
- Windows – press `Windows+Shift+S`, then save as PNG.

If using a terminal screenshot command, run it after the output is visible. Do
not overwrite an image until checking that it opens correctly.

## 6. Run Linux process commands

Use a disposable `sleep` process. Never use an unknown PID.

```bash
cd /path/to/CPE333/ps03
nice -n 10 sleep 120 &
test_pid=$!
printf 'test PID: %s\n' "$test_pid"
ps -p "$test_pid" -o pid,ppid,stat,ni,pri,%cpu,%mem,cmd
top -b -n 1 -p "$test_pid"
renice -n 15 -p "$test_pid"
ps -p "$test_pid" -o pid,ppid,stat,ni,pri,%cpu,%mem,cmd
kill -TERM "$test_pid"
wait "$test_pid" 2>/dev/null || true
printf 'test process cleaned up\n'
```

Record:

- `ps` gives a one-time snapshot.
- `top` refreshes a live process view. Press `q` to exit interactive `top`.
- `nice` starts a command with an adjusted nice value.
- `renice` changes the nice value of an existing process.
- `kill -TERM` requests normal termination.
- The `ps` columns show PID, parent PID, state, nice value, priority, CPU, and
  memory information.

Save the screenshot as:

```text
$RESULTS_DIR/01_linux_commands.png
```

If batch `top` is unavailable, run interactive `top`, locate the test PID, and
press `q` after capturing it. Linux and macOS `ps` options differ; use the Linux
commands for Linux evidence.

## 7. Run `ss1_1.sh` in the foreground and background

Make the scripts executable:

```bash
cd /path/to/CPE333/ps03
make scripts
```

### Foreground run

```bash
time ./src/ss1_1.sh
```

The shell does not return the prompt until about ten seconds have passed.
Capture the command, elapsed time, and returned prompt as:

```text
$RESULTS_DIR/02_ss1_1_foreground.png
```

### Background run

```bash
./src/ss1_1.sh &
background_pid=$!
printf 'background PID: %s\n' "$background_pid"
jobs -l
ps -p "$background_pid" -o pid,ppid,stat,cmd
wait "$background_pid"
printf 'background job finished\n'
```

The prompt returns immediately after `&`. The script continues as a background
job. Capture the command, job entry, PID, and prompt as:

```text
$RESULTS_DIR/03_ss1_1_background.png
```

Discuss foreground blocking versus background execution.

## 8. Stop and resume `ss1_2.sh`

This script sleeps for 1000 seconds. Do not leave it running after the test.
The job number shown by the shell may not be `1`.

### Stop the job

Run:

```bash
cd /path/to/CPE333/ps03
./src/ss1_2.sh
```

Immediately press `Ctrl+Z`, then run:

```bash
jobs -l
```

Record the job number, PID, and stopped state. Save the screenshot as:

```text
$RESULTS_DIR/04_ss1_2_stopped_jobs.png
```

`Ctrl+Z` sends a stop signal to the foreground process group. It does not
terminate the job.

### Resume with `fg`

Replace `1` with the job number printed by `jobs -l`:

```bash
fg %1
```

The job resumes in the foreground and occupies the terminal. Press `Ctrl+C`
after confirming that it resumed so the 1000-second sleep does not remain
running. Save the screenshot as:

```text
$RESULTS_DIR/05_ss1_2_fg.png
```

### Resume with `bg`

Repeat the stop procedure so a stopped job exists again:

```bash
./src/ss1_2.sh
# press Ctrl+Z
jobs -l
bg %1
jobs -l
```

The job resumes in the background and the prompt returns. Clean it up using
the actual job number:

```bash
kill %1
wait %1 2>/dev/null || true
```

Save the screenshot as:

```text
$RESULTS_DIR/06_ss1_2_bg.png
```

If `fg %1` reports that no such job exists, run `jobs -l` and use the current
job number. If a job was left running, find it with `jobs -l` or `ps` and stop
only that known process.

## 9. Try a Linux GUI process monitor

Use the tool matching the Linux desktop. GNOME System Monitor is the
recommended Debian choice.

### GNOME System Monitor

Install and start it:

```bash
sudo apt install -y gnome-system-monitor
gnome-system-monitor
```

In the Processes view, show a table containing PID, user, status, CPU, memory,
priority, and nice value where available. In the Resources view, show CPU
history and memory/swap or network graphs.

Save the screenshot as:

```text
$RESULTS_DIR/07_linux_gui.png
```

Record the tool name, package/version, acquisition command, how it was used,
and values it can monitor.

### Other Linux desktops

- KDE Plasma – System Monitor (`plasma-systemmonitor`)
- XFCE – Task Manager (`xfce4-taskmanager`)
- Other desktop – use its installed process monitor and record the exact name

A headless server cannot provide a GUI screenshot. Use a Debian desktop VM,
WSLg, or another Linux desktop.

## 10. Try a Windows process monitor

On Windows, open Task Manager:

```text
Ctrl+Shift+Esc
```

Use the Processes or Details tab and the Performance tab. Show process ID,
CPU, memory, disk, network, GPU, status, and priority where available.

Save the screenshot as:

```text
$RESULTS_DIR/08_windows_gui.png
```

Record:

- Windows version.
- Tool name and how it was acquired.
- How the Processes, Details, and Performance tabs were used.
- Values monitored and any differences from the Linux tool.

Optional alternative: Microsoft Sysinternals Process Explorer. If used, record
its download source and version; it is not built into Windows.

## 11. Capture the scheduler function and three cases

### Scheduler-function screenshot

From `CPE333/ps03/`, print the relevant source with line numbers:

```bash
nl -ba src/PS3.c | sed -n '1,115p'
```

Capture the section containing `select_process()`, `simulate_stfc()`, the
process-ID tie-break rule, and the time-slot loop. Save it as:

```text
$RESULTS_DIR/09_scheduler_function.png
```

An editor screenshot is acceptable if the function name and enough code to
explain the algorithm are visible.

### Case 1

```bash
cd /path/to/CPE333/ps03
./bin/PS3 case1.csv
```

Save the complete command and output as:

```text
$RESULTS_DIR/10_case1.png
```

Expected output:

```text
[0-2]: P1
[2-4]: P2
[4-5]: P3
[5-7]: P2
[7-12]: P1
```

### Case 2

```bash
./bin/PS3 case2.csv
```

Save it as:

```text
$RESULTS_DIR/11_case2.png
```

Expected output:

```text
[0-2]: P1
[2-5]: IDLE
[5-6]: P2
[6-7]: P4
[7-9]: P2
[9-12]: P3
```

### Case 3

```bash
./bin/PS3 case3.csv
```

Save it as:

```text
$RESULTS_DIR/12_case3.png
```

Expected output:

```text
[0-3]: P2
[3-5]: P1
[5-7]: P2
[7-8]: P3
```

Every scheduler screenshot must show the case command and the full output. Do
not crop away the filename or interval labels.

## 12. Check the evidence set

Verify the tracked `ps03/results/` directory:

```bash
find "$RESULTS_DIR" -maxdepth 1 -type f -print | sort
```

Required images:

```text
00_environment.png
01_linux_commands.png
02_ss1_1_foreground.png
03_ss1_1_background.png
04_ss1_2_stopped_jobs.png
05_ss1_2_fg.png
06_ss1_2_bg.png
07_linux_gui.png
08_windows_gui.png
09_scheduler_function.png
10_case1.png
11_case2.png
12_case3.png
```

Names are case-sensitive. If an image is missing, record that fact instead of
inventing evidence.

## 13. Hand the evidence to the report owner

The CPE333 repository does not contain `report.tex`. If the group has a
separate report workspace, copy the evidence from `ps03/results/` there using
its own documented layout:

```bash
cp "$RESULTS_DIR"/*.png /path/to/private-report-workspace/ps03/results/
```

Also provide:

- `/tmp/ps03-check.txt`
- `notes/README.md`
- Exact OS, compiler, tool versions, PIDs, job numbers, and observations.

The report owner replaces TODO text, adds the screenshots, compiles the PDF,
and submits the PDF to LEB2. A person who receives only CPE333 does not need
access to the report workspace to build or test the assignment.

If the report workspace uses a LaTeX template, compile from the directory that
contains its `report.tex`:

```bash
cd /path/to/private-report-workspace/ps03
pdflatex report.tex
pdflatex report.tex
```

The CPE333 Makefile intentionally has no report target. A fresh CPE333 clone
must work with only:

```bash
cd CPE333/ps03
make clean
make
make check
```

## 14. Git handoff

For the CPE333 repository, inspect only this repository:

```bash
cd /path/to/CPE333
git status --short
git diff --check
```

Commit project files such as PS03 source, scripts, Makefile, guide, assignment
documentation, and notes. Do not commit generated `bin/` files.

Evidence and report files are handled by the report owner in the separate
submission workspace. Do not add a missing report workspace to CPE333 merely to
make the local tree look complete.

## 15. Troubleshooting

### `Permission denied` for a shell script

```bash
cd /path/to/CPE333/ps03
make scripts
```

### `case1.csv` cannot be opened

Run the executable from `ps03/`:

```bash
cd /path/to/CPE333/ps03
./bin/PS3 case1.csv
```

### C program does not build on native Windows

Use WSL2 or a Linux VM. The code uses POSIX process behavior.

### `pdflatex` cannot find source files

This is a report-workspace path issue, not a CPE333 build issue. Compile from
the report template's directory and ensure its source-listing paths point to
`CPE333/ps03/src/`.

### GUI tool will not start

The system may be headless or have no display session. Use a Debian desktop VM,
WSLg, or another Linux desktop. Do not claim a GUI observation that was not
performed.

### `fg` or `bg` reports an invalid job

Run `jobs -l` and use the current job number. Job numbers are shell-local and
can change between terminals.

### A long-running test was left behind

Inspect only known processes and jobs:

```bash
jobs -l
ps -u "$USER" -o pid,ppid,stat,cmd
```

Terminate only the test PID or job you started:

```bash
kill <known-test-pid>
wait <known-test-pid> 2>/dev/null || true
```
