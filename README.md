# CPE333

Standalone repository for CPE333 coursework.

## Platform support

The programs use C17 and POSIX process APIs: `fork()`, `wait()`, `pipe()`, `read()`, `write()`, `close()`, and `sleep()`.

Supported environments:

- Linux distributions – Debian, Ubuntu, Fedora, Arch, and others.
- WSL2 with a Linux distribution.
- macOS with Xcode Command Line Tools.
- BSD systems with a C compiler and POSIX userland.

Native Windows is not supported because Windows does not provide `fork()` with these POSIX semantics. Use WSL2, a Linux VM, or another Unix-like environment.

## Clone

```bash
git clone https://github.com/beambeambeam/CPE333.git
cd CPE333
```

## Contents

- `ps02/` – Problem Session 2: process creation and pipes.
- `ps03/` – Problem Session 3: process monitoring, job control, and scheduling.
- `ps04/` – Problem Session 4: C storage classes, PIE vs `-no-pie`, and dynamic memory.

Build PS02 programs:

```bash
cd ps02
make
```

Build PS03 scheduler:

```bash
cd ps03
make
```

Build PS04 storage-class and memory programs (Linux for `-no-pie`):

```bash
cd ps04
make
make nopie
```

## Reports

Report files are maintained separately from this source repository and are not
included in a fresh CPE333 checkout. This repository contains the programs,
input cases, experiment guide, and notes needed to produce the report.

`CPE333` is also available inside `my-cpe-lab` as a Git submodule. It remains an independent repository.
