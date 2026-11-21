# PS12 experiment guide – file system and implementation

The assignment is in `problem-session.md`. Raw evidence goes in `results/`.

## Linux (Labs 1–3)

`run-linux.sh` runs every Linux step and saves one transcript per lab in `results/`. It needs `strace`, `tmux`, `file`, and `script` (util-linux).

```bash
cd ps12
sudo apt install strace tmux file   # Debian/Ubuntu
./run-linux.sh
```

The script drives two real terminals with tmux for the named pipe lab and captures both screens after each step. It runs `strace` under `script` so `cat` writes to a terminal, which matches the slide output.

The committed transcripts came from an Ubuntu 24.04 container on an aarch64 OrbStack VM:

```bash
docker run --rm --cap-add SYS_PTRACE --security-opt seccomp=unconfined \
    -v "$PWD:/ps12" -w /ps12 ubuntu:24.04 bash -c \
    'apt-get update -qq && apt-get install -y -qq strace tmux file && ./run-linux.sh'
```

`SYS_PTRACE` and the unconfined seccomp profile are needed for `strace` inside Docker.

| File                            | Lab                                         |
| ------------------------------- | ------------------------------------------- |
| `00_environment.txt`            | Kernel, distribution, and tool versions     |
| `01_lab1_links.txt`             | Lab 1 on Linux                              |
| `02_lab2_fifo_setup.txt`        | Lab 2: `mkfifo`, `ls -la`, blocked writer   |
| `03_lab2_fifo_terminals.txt`    | Lab 2: two-terminal session, step by step   |
| `04_lab3_strace.txt`            | Lab 3: `strace cat foo`                     |
| `05_windows_links.txt`          | Lab 1 on Windows (run `windows-links.cmd`)  |

## Windows (Lab 1)

`mklink` needs an elevated Command Prompt, or Developer Mode for file and directory symbolic links. Hard links (`/H`) and junctions (`/J`) work without elevation. Use NTFS volumes.

```bat
cd ps12
windows-links.cmd D: > results\05_windows_links.txt 2>&1
```

The argument is the second drive for the cross-drive test. Use any other NTFS drive letter if `D:` does not exist. Take screenshots of the console if the report needs images.
