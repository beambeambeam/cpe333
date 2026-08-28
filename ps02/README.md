# CPE333 PS02 – Build and run guide

Problem Session 2 covers process creation, process lifetime, `fork()`, `wait()`, and pipes.

- `problem-session.md` – assignment requirements.
- `src/` – starter C programs.
- `Makefile` – portable build commands.
- `notes/` – experiment observations and report material.

The CPE333 checkout contains no report source or screenshot directory. Keep
those files in the group's separate submission workspace.

## Platform support

The programs require a C17 compiler and POSIX process APIs.

Works on:

- Linux – Debian, Ubuntu, Fedora, Arch, and others.
- WSL2 Linux distributions.
- macOS with Xcode Command Line Tools.
- BSD systems with a C compiler and POSIX shell.

Native Windows is not supported because it has no compatible `fork()` API. Use WSL2 or a Linux VM.

Building and running does not require `sudo`. Use administrator privileges only to install packages.

## Install prerequisites

Install the core tools for the platform. Debugging and tracing tools are optional.

### Debian, Ubuntu, and WSL2 Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential git
sudo apt install -y gdb man-db manpages-dev procps psmisc strace
```

### Fedora, Rocky, Alma, and RHEL-like systems

```bash
sudo dnf install -y gcc make git
sudo dnf install -y gdb man-db man-pages procps-ng psmisc strace
```

If `sudo dnf` reports a package unavailable, install the first command and continue without that optional package.

### Arch Linux

```bash
sudo pacman -S --needed base-devel git
sudo pacman -S --needed gdb man-db man-pages procps-ng psmisc strace
```

### macOS

Install Apple command-line tools:

```bash
xcode-select --install
```

This provides `clang`, `make`, `git`, `ps`, `sleep`, and manual pages. Homebrew is optional:

```bash
brew install git
```

`strace` is Linux-only. Skip it on macOS.

### FreeBSD and other BSD systems

Use the system C compiler and `make`, or install GNU Make if required:

```bash
sudo pkg install gmake git gdb psmisc
```

Use `gmake` if the system `make` rejects the Makefile.

## Clone

Standalone clone – recommended:

```bash
git clone https://github.com/beambeambeam/CPE333.git
cd CPE333/ps02
```

Already cloned:

```bash
cd ~/CPE333
git pull --ff-only origin main
cd ps02
```

Parent-repository clone:

```bash
git clone --recurse-submodules https://github.com/beambeambeam/my-cpe-lab.git
cd my-cpe-lab/Y3/CPE333/ps02
```

If the parent repository was cloned without submodules:

```bash
cd ~/my-cpe-lab
git pull --ff-only origin main
git submodule update --init --recursive
cd Y3/CPE333/ps02
```

## Build

```bash
make
```

The Makefile uses the platform C compiler by default through `cc`. Override it when needed:

```bash
make CC=gcc
make CC=clang
```

Compiler flags enable C17, warnings, and debugging symbols. Binaries are written to `bin/`, which is ignored by Git.

If `make` is unavailable, install it with the platform package manager. The programs can also be compiled directly:

```bash
mkdir -p bin
cc -std=c17 -Wall -Wextra -Wpedantic -O0 -g \
   src/01_hello.c -o bin/01_hello
```

Clean build output:

```bash
make clean
make
```

## Generate the PDF report

Report source and screenshots are maintained outside this CPE333 checkout. Use
TeX Live, MacTeX, or another LaTeX distribution in the group's submission
workspace. The CPE333 source build does not depend on report files.

Run LaTeX twice so the table of contents and references settle. Generated PDF and auxiliary files are ignored by Git.

## Publish changes

CPE333 changes are committed and pushed from the standalone repository:

```bash
cd ~/CPE333
git add ps02
git commit -m "Update PS02 lab"
git push origin main
```

The parent repository stores a fixed CPE333 commit. Update its pointer only when needed:

```bash
cd ~/my-cpe-lab/Y3/CPE333
git pull --ff-only origin main
cd ../..
git add Y3/CPE333
git commit -m "Update CPE333 submodule"
git push origin main
```

## Run starter programs

```bash
./bin/01_hello
./bin/02_hello_wait
./bin/03_lifetime child-first
./bin/03_lifetime parent-first
./bin/05_pipe
```

`03_lifetime child-first` keeps the parent alive while the child exits. Inspect the child during the parent sleep:

Portable process inspection for a known PID:

```bash
ps -p <pid> -o pid,ppid,stat,command
```

Linux-specific process tree commands:

```bash
ps -ef --forest
pstree -p "$$"
```

Some systems use different `ps` field names. If `command` is rejected, try `comm`.

The child can appear as `<defunct>` until the parent calls `waitpid()`.

`03_lifetime parent-first` exits the parent while the child continues. The child prints its parent PID before and after sleeping. The second PPID normally becomes PID 1 or another system subreaper.

## Fork-count safety

Take a VM snapshot before running this experiment. Run it as a normal user, never with `sudo`.

The program creates one child at a time and each parent waits, but it still must run under a process limit:

```bash
( ulimit -u 128; ./bin/04_fork_count )
```

The program reports successful `fork()` calls when `fork()` fails. Do not run an uncontrolled program where every process forks again.

To stop an experiment, inspect exact user process IDs first:

```bash
pgrep -a -u "$USER"
kill <pid>
```

## Manual pages

```bash
man 2 fork
man 2 wait
man 2 pipe
man 2 read
man 2 write
man 2 close
man 3 sleep
man 1 ps
```

Optional syscall trace:

```bash
strace -f ./bin/05_pipe
```

## Pipe observations to record

The base pipe program demonstrates child-to-parent communication. Create or modify variants for the required experiments:

1. A sender reading from the same pipe consumes bytes from the shared byte stream; it does not read a private copy.
2. A receiver reading before the sender writes blocks while a write end remains open.
3. Several writes can be returned by one read, or one message can be split across reads. A pipe has no message boundaries.
4. Closing every write end lets the reader receive EOF (`read()` returns `0`).

Save commands, output, PIDs, PPIDs, process states, and explanations in `notes/` for the PDF report.
