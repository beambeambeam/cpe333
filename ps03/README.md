# CPE333 PS03 – Process Monitoring and Scheduling

Problem Session 3 covers Linux process/job commands, foreground and background
jobs, stopped jobs, GUI process monitors, Windows process monitors, and a
shortest-time-to-completion scheduler simulation.

## Layout

- `problem-session.md` – assignment requirements.
- `guide.md` – universal step-by-step experiment and report guide.
- `src/PS3.c` – scheduler implementation and CSV runner.
- `src/ss1_1.sh` – ten-second shell script.
- `src/ss1_2.sh` – 1000-second shell script for stop/resume experiments.
- `case1.csv`, `case2.csv`, `case3.csv` – scheduler inputs.
- `results/` – experiment screenshots and output evidence.
- `notes/` – observation checklist and experiment notes.

The CPE333 checkout intentionally contains no report files. Keep the report,
PDF, and screenshots in the group's separate submission workspace.

## Platform

Run the process experiments on Debian GNU/Linux, WSL2, or another Linux
system. Run the Windows section on Windows. The scheduler itself uses standard
C and should compile on Linux and macOS.

Native Windows is not required for the C scheduler, but the Windows GUI section
needs a Windows system or VM.

## Debian preparation

Install the compiler, build tools, process commands, and optional Linux GUI
monitor:

```bash
sudo apt update
sudo apt install -y build-essential procps gnome-system-monitor
```

If the Debian system has no desktop session, install only `build-essential` and
`procps`; perform the GUI experiment on a Debian desktop VM.

## Build and run

From this directory:

```bash
make
make check
```

`make check` runs all three CSV cases. Run one case separately when capturing a
screenshot:

```bash
./bin/PS3 case1.csv
./bin/PS3 case2.csv
./bin/PS3 case3.csv
```

The scheduler selects the ready process with the smallest remaining burst. It
re-evaluates the ready queue at every integer time and uses the process ID as a
tie-breaker. Adjacent slots for one process are printed as one interval.

## Shell-job experiments

Make the scripts executable if needed:

```bash
make scripts
```

Foreground and background execution:

```bash
./src/ss1_1.sh
./src/ss1_1.sh &
background_pid=$!
ps -p "$background_pid" -o pid,ppid,stat,ni,cmd
wait "$background_pid"
```

Stop and resume a job:

```bash
./src/ss1_2.sh
# press Ctrl+Z
jobs -l
fg %1
```

Repeat the experiment, press `Ctrl+Z`, then use `bg %1` instead. `fg` resumes
the job in the foreground and keeps the terminal occupied. `bg` resumes it in
the background and returns the prompt.

The job number may not be `1`; use the number printed by `jobs -l`.

## Process command experiments

Use a short-lived test process. Do not kill a process unless its PID is known.

```bash
sleep 1000 &
test_pid=$!
ps -p "$test_pid" -o pid,ppid,stat,ni,pri,%cpu,%mem,cmd
top
renice -n 10 -p "$test_pid"
kill -TERM "$test_pid"
wait "$test_pid" 2>/dev/null || true
```

Compare `ps` and `top` in the report:

- `ps` takes a snapshot of selected processes and exits.
- `top` continuously refreshes a process view and shows changing resource use.
- `nice` starts a process with an adjusted scheduling niceness.
- `renice` changes niceness for an existing process.
- `kill` sends a signal to a PID or job, such as `TERM`, `STOP`, or `CONT`.

Useful inspection commands:

```bash
ps -ef
ps -eo pid,ppid,stat,ni,pri,%cpu,%mem,cmd --sort=-%cpu | head
jobs -l
```

## GUI tools

Recommended Linux tool: **GNOME System Monitor**.

- Acquire it with `sudo apt install gnome-system-monitor`.
- Open it from the application menu or run `gnome-system-monitor`.
- Inspect the Processes and Resources tabs.
- Record PID, user, status, CPU, memory, priority, nice value, CPU history,
  memory/swap use, and network activity where available.

Recommended Windows tool: **Task Manager**.

- It is built into Windows; press `Ctrl+Shift+Esc`.
- Use the Processes, Details, and Performance tabs.
- Record process ID, CPU, memory, disk, network, GPU, status, priority, and
  process history where available.

Sysinternals Process Explorer is an optional Windows tool for a more detailed
process tree and per-process inspection.

## Report handoff

This repository does not include the report source or screenshot directory.
Record exact Debian/Windows output in `notes/README.md`, then transfer the
notes and screenshots to the group's separate submission workspace. Use the
assignment requirements to complete the report there.

The CPE333 Makefile intentionally has no report target. A fresh checkout must
build and run without any other repository:

```bash
make clean
make
make check
```
