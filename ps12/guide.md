# PS12 experiment guide – file system and implementation

The assignment is in `problem-session.md`. Raw evidence goes in `results/`.

## Linux (Labs 1–3)

`run-linux.sh` runs every Linux step and saves one transcript per lab in `results/`. It needs `strace`, `tmux`, `file`, and `script` (util-linux).

```bash
cd ps12
sudo pacman -S --needed strace tmux file   # CachyOS/Arch
# Or: sudo apt install strace tmux file    # Debian/Ubuntu
./run-linux.sh
```

The script drives two real terminals with tmux for the named pipe lab and captures both screens after each step. It runs `strace` under `script` so `cat` writes to a terminal, which matches the slide output.

The current transcripts were captured directly on CachyOS x86_64 (kernel
`7.3.3-1-cachyos`, GNU coreutils `9.12`, strace `7.3`, tmux `3.8`). The Lab 3
requested buffer size depends on the installed coreutils version.

An Ubuntu 24.04 container can be used for a separate reproduction:

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

The Windows runner requires an Administrator Command Prompt for file and directory symbolic links. Hard links (`/H`) and junctions (`/J`) do not themselves require elevation. Use NTFS volumes. The runner does not change Developer Mode or system settings.

```bat
cd ps12
windows-links.cmd D:
```

The argument is the second drive for the cross-drive test. Use another NTFS drive letter if `D:` does not exist. `run-windows.ps1` runs actual `cmd.exe` commands, captures stdout, stderr, and exit codes, and writes the combined transcript to [results/05_windows_links.txt](results/05_windows_links.txt). Expected failures, such as reading a dangling symbolic link, are checked as failures rather than silently treated as successes.

Each run creates new isolated directories on C: and the second drive, bounds each command to 10 seconds, removes the junction loop before cleanup, and removes only those run directories. It never deletes an existing fixed-name `ps12-links` directory.

The Windows capture was made on 22 November 2026 at 20:36-20:37 +07:00, with native Windows version `10.0.22631.4751` and two distinct NTFS volumes, C: and D:. All 76 command and content checks passed. The transcript and eight screenshots contain the Windows evidence. For privacy, the Windows username, temporary run identifier, and volume serial numbers are replaced with placeholders in the transcript and covered with gray boxes in the screenshots. Commands, exit codes, and experimental results are preserved. The reported 505-character path length is the original measurement before redaction. Review new captures for local identifiers before publishing them.

| Screenshot | Observed result |
| --- | --- |
| [Environment](results/screenshots/windows/00_environment.png) | Native Windows version and distinct C: and D: NTFS volumes. |
| [Basic links](results/screenshots/windows/01_lab1_basic_links.png) | A hard link retains the original data after source deletion; a symbolic link breaks, then follows the recreated source. |
| [Symbolic chain: delete source](results/screenshots/windows/02a_lab1_symbolic_delete_source.png) | Both symbolic levels break after source deletion. |
| [Symbolic chain: delete level 1](results/screenshots/windows/02b_lab1_symbolic_delete_level1.png) | The source survives, but level 2 breaks. |
| [Hard-link cases](results/screenshots/windows/02c_lab1_hard_link_chains.png) | Level 2 survives deletion of either the source or the intermediate name. |
| [Directory links](results/screenshots/windows/03a_lab1_directory_links.png) | Directory hard links fail; directory symbolic links and junctions work. |
| [Directory loop](results/screenshots/windows/03b_lab1_directory_loop.png) | Recursive `dir /s` follows the junction repeatedly: 192 paths, maximum length 505 characters. |
| [Cross-drive links](results/screenshots/windows/04_lab1_cross_drive.png) | D:-to-C: hard links fail; symbolic links and directory junctions work. |

The screenshots show a native read-only viewer of the captured command output. They are screenshots of that viewer, rather than live Command Prompt windows. To reopen it:

```powershell
powershell.exe -NoProfile -File .\view-windows-results.ps1
```

The viewer uses the Windows .NET Framework compiler and reads `results/05_windows_links.txt`; it does not rerun commands. Any intermediate viewer data is generated under the system temporary directory. Screenshots are stored in `results/screenshots/windows/`, matching the Linux PR's platform-specific folder convention. Read the two screenshot columns from left to right; each states its working directory.

`report.tex` and `report.pdf` use this Windows transcript and screenshots. The deepest `dir /s` paths contain 63 `\up` junctions, which matches the Windows limit of 63 [reparse points](https://learn.microsoft.com/en-us/windows/win32/fileio/reparse-points) in one path.
