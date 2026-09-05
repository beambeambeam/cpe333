# PS04 universal step-by-step guide

This guide prepares and runs PS04 on a Unix-based OS. It explains which
commands to run, which screenshots to capture, and how to hand the evidence
to the person who owns the report. Nothing here has been executed yet — the
friend running the lab follows this file top to bottom on Linux.

## 0. Handoff boundary

A person receiving this project receives **only the CPE333 repository**:

```text
CPE333/
├── ps02/
├── ps03/
└── ps04/
```

The CPE333 repository contains the source programs, assignment instructions,
lecture notes, experiment guide, notes, and the PS04 evidence folder. It does
not contain the report source or report PDF.

Keep project files in CPE333 and commit them to the CPE333 Git repository.
Keep the experiment screenshots in `ps04/results/`; this directory is part of
the CPE333 handoff and starts with an empty `.gitkeep` file. The report owner
can copy those screenshots into the separate submission workspace later.

Prepare the evidence folder from the CPE333 root:

```bash
cd /path/to/CPE333
export RESULTS_DIR="$PWD/ps04/results"
mkdir -p "$RESULTS_DIR"
```

## 1. Choose the platform

| Platform | Use it for | Notes |
|---|---|---|
| Debian/Ubuntu | All PS04 experiments and report preparation | Recommended |
| WSL2 Ubuntu | All PS04 experiments | Works, command-line only |
| Fedora/Arch | All PS04 experiments | Use the package commands below |
| macOS | PIE binaries preparation only | Collect final evidence on Linux |
| Windows native | Nothing | Use WSL2 or a Linux VM |

***This lab must be done on a Unix-based OS.*** The `-no-pie` comparison is
the core of Parts 1–2 and needs Linux `gcc`. A Debian desktop VM is the
safest single choice. Suggested VM resources: 2 CPU cores, 4 GB RAM, 20 GB
disk. Ubuntu Desktop LTS is a valid alternative. Record the exact
distribution, release, architecture, and compiler version in the report.

Parts 1–2 compare default PIE against `-no-pie`. Part 3 (`malloc`/`realloc`)
must run both variants **on the same machine**.

## 2. Install prerequisites

Install a C compiler and `make`. LaTeX is only needed if the PDF is built on
that machine.

### Debian, Ubuntu, and WSL2 Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential
sudo apt install -y texlive-latex-base texlive-latex-recommended texlive-latex-extra
```

### Fedora

```bash
sudo dnf install -y gcc make
sudo dnf install -y texlive-scheme-basic texlive-collection-latexrecommended texlive-collection-latexextra
```

### Arch Linux

```bash
sudo pacman -S --needed base-devel
sudo pacman -S --needed texlive-basic texlive-latexextra
```

### macOS (prep only)

```bash
xcode-select --install
```

```bash
cc --version
make --version
```

macOS `clang` does not accept Linux-style `-no-pie` the same way. Use macOS
only to prepare the PIE sources; run `make nopie` and collect evidence on
Linux.

## 3. Verify the CPE333 checkout

From the CPE333 repository:

```bash
cd /path/to/CPE333
printf 'repository: '
git rev-parse --show-toplevel
git status --short
```

Enter PS04:

```bash
cd ps04
ls -la
find src -maxdepth 1 -type f -print | sort
```

Required PS04 files:

```text
Makefile
problem-session.md
c_storge_class_and_pie.md
guide.md
README.md
src/ps4_static.c
src/ps4_static_auto.c
src/ps4_extern.c
src/ps4_extern_shadow.c
src/ps4_mem.c
src/ps4_mem_full.c
results/.gitkeep
notes/README.md
```

Check tools:

```bash
command -v cc
command -v make
command -v gcc
```

## 4. Build (friend runs this on Linux)

Run from `CPE333/ps04/`, not from `src/`:

```bash
make clean
make
make nopie
```

Expected binaries:

```text
bin/ps4_static  bin/ps4_static_auto
bin/ps4_extern  bin/ps4_extern_shadow
bin/ps4_mem     bin/ps4_mem_full
bin/ps4_static_nopie  bin/ps4_static_auto_nopie
bin/ps4_extern_nopie  bin/ps4_extern_shadow_nopie
```

Expect one warning per file and no errors:

```text
warning: return type of 'main' is not 'int'
```

The sources keep the handout's `void main()` verbatim so the report output
matches the assignment. Do not change them to `int main()`.

If `make nopie` fails with `unrecognized option '-no-pie'`, the machine is
not Linux gcc — switch to the Linux lab machine instead of working around it.

## 5. Capture the environment evidence

Run this on the machine supplying all PS04 evidence:

```bash
cd /path/to/CPE333/ps04
{
    printf '$ uname -a\n'
    uname -a
    printf '$ cat /etc/os-release\n'
    cat /etc/os-release
    printf '$ cc --version\n'
    cc --version | head -1
    printf '$ gcc -v\n'
    gcc -v 2>&1 | tail -1
    printf '$ make --version\n'
    make --version | head -1
} | tee /tmp/ps04-environment.txt
```

Capture a terminal screenshot containing OS, architecture/version, compiler,
and Make version. Save it as:

```text
$RESULTS_DIR/00_environment.png
```

### Universal screenshot methods

Capture only the relevant terminal. Keep the command and full output visible.

- GNOME Linux – press `Print Screen` or use `gnome-screenshot -a`.
- KDE Linux – use Spectacle and select a rectangular region.
- macOS – press `Command+Shift+4`.
- Windows – press `Windows+Shift+S`, then save as PNG.

Do not overwrite an image until checking that it opens correctly.

## 6. Part 1 — static storage class, PIE

```bash
cd /path/to/CPE333/ps04
./bin/ps4_static
./bin/ps4_static
./bin/ps4_static
```

Three separate process runs. Record the value and address of `y` each time.
Expected pattern: values climb within one run (6, 7, 8 — `static` persists
across loop iterations), while the address of `y` shifts between runs
because PIE loads the executable at a different base each time.

Save the full commands plus all three outputs as:

```text
$RESULTS_DIR/01_static_pie.png
```

## 7. Part 1 — `static` removed, PIE

```bash
./bin/ps4_static_auto
./bin/ps4_static_auto
./bin/ps4_static_auto
```

Now `y` is an `auto` variable re-created every iteration. Expected pattern:
the value repeats (6, 6, 6) instead of climbing.

Save it as:

```text
$RESULTS_DIR/02_static_auto_pie.png
```

Discussion points for the report: lifetime (persistent vs per-iteration),
 initialization (once vs every iteration), and why the address behavior
still shows PIE shifting across runs.

## 8. Part 1 — redo with `-no-pie`

```bash
./bin/ps4_static_nopie
./bin/ps4_static_nopie
./bin/ps4_static_nopie
```

Save it as:

```text
$RESULTS_DIR/03_static_nopie.png
```

```bash
./bin/ps4_static_auto_nopie
./bin/ps4_static_auto_nopie
./bin/ps4_static_auto_nopie
```

Save it as:

```text
$RESULTS_DIR/04_static_auto_nopie.png
```

Expected difference: values behave exactly as in the PIE runs (climb vs
repeat), but global/static addresses are now identical across runs because
the executable loads at a fixed virtual address. That contrast is the PIE
section of the report.

## 9. Part 2 — extern storage class, PIE

```bash
./bin/ps4_extern
./bin/ps4_extern
./bin/ps4_extern
```

`main` and `display` both declare `extern int x;`, so both print the global
value 20 and the same address. Addresses shift between runs under PIE.

Save it as:

```text
$RESULTS_DIR/05_extern_pie.png
```

## 10. Part 2 — `extern` removed in `main`, PIE

```bash
./bin/ps4_extern_shadow
./bin/ps4_extern_shadow
./bin/ps4_extern_shadow
```

`main` now has an uninitialized local `int x;` shadowing the global: its
printed value is indeterminate (garbage, varies per run) and its address is
a stack address, while `display()` still prints the global 20 at the global
address.

Save it as:

```text
$RESULTS_DIR/06_extern_shadow_pie.png
```

## 11. Part 2 — redo with `-no-pie`

```bash
./bin/ps4_extern_nopie
./bin/ps4_extern_nopie
./bin/ps4_extern_nopie
```

Save it as:

```text
$RESULTS_DIR/07_extern_nopie.png
```

```bash
./bin/ps4_extern_shadow_nopie
./bin/ps4_extern_shadow_nopie
./bin/ps4_extern_shadow_nopie
```

Save it as:

```text
$RESULTS_DIR/08_extern_shadow_nopie.png
```

Same value pattern as PIE, but the global address is now stable across runs.

## 12. Part 3 — malloc / realloc (same machine)

```bash
cd /path/to/CPE333/ps04
./bin/ps4_mem
```

Covers: Address of Pointer (`&a`, `&b` — stack slots holding the pointers),
Effective Address (`a`, `b` before `malloc` — indeterminate), array `a`
after `malloc` (heap block, `a == &a[0]`, `&a[9]` 36 bytes past for ints),
Array `c` (stack array, `c == &c[0]`), and After `realloc` to 1000 ints
(block may move; `&a[999]` ~4000 bytes past `a`).

Save it as:

```text
$RESULTS_DIR/09_mem.png
```

Then the uncommented variant:

```bash
./bin/ps4_mem_full
```

Save it as:

```text
$RESULTS_DIR/10_mem_full.png
```

Explain whether the result differs from the previous run, especially the
address of array `a`: the extra `malloc` for `b` consumes heap space first,
so the later `realloc` result can land elsewhere. Both runs must come from
the same machine or the comparison is meaningless.

## 13. Check the evidence set

Verify the tracked `ps04/results/` directory:

```bash
find "$RESULTS_DIR" -maxdepth 1 -type f -print | sort
```

Required images:

```text
00_environment.png
01_static_pie.png
02_static_auto_pie.png
03_static_nopie.png
04_static_auto_nopie.png
05_extern_pie.png
06_extern_shadow_pie.png
07_extern_nopie.png
08_extern_shadow_nopie.png
09_mem.png
10_mem_full.png
```

Names are case-sensitive. If an image is missing, record that fact instead
of inventing evidence. Every screenshot must show the exact command and the
complete, uncropped output — addresses are the graded data.

## 14. Hand the evidence to the report owner

The CPE333 repository does not contain `report.tex`. If the group has a
separate report workspace, copy the evidence from `ps04/results/` there using
its own documented layout:

```bash
cp "$RESULTS_DIR"/*.png /path/to/private-report-workspace/ps04/results/
```

Also provide:

- `/tmp/ps04-environment.txt`
- Pasted full outputs of every run (all 3 runs per binary, not just one)
- `notes/README.md` with the checklists filled in
- Exact OS, compiler, and tool versions

The report owner replaces TODO text, adds the screenshots, compiles the PDF,
and submits the PDF to LEB2. A person who receives only CPE333 does not need
access to the report workspace to build or test the assignment.

The CPE333 Makefile intentionally has no report target. A fresh CPE333 clone
must work with only:

```bash
cd CPE333/ps04
make clean
make
make nopie
```

`make check` runs the PIE set and `make check-nopie` runs the `-no-pie` set
for a quick smoke test; screenshots should still be captured from the
individual commands above so each image maps to one assignment step.

## 15. Git handoff

For the CPE333 repository, inspect only this repository:

```bash
cd /path/to/CPE333
git status --short
git diff --check
```

Commit project files such as PS04 sources, Makefile, guide, assignment
documentation, and notes. Do not commit generated `bin/` files.

Evidence and report files are handled by the report owner in the separate
submission workspace. Do not add a missing report workspace to CPE333 merely
to make the local tree look complete.

## 16. Troubleshooting

### `Permission denied` or `command not found: make`

```bash
sudo apt install -y build-essential
```

### `-no-pie` unrecognized

The machine is not Linux gcc. Move to the Linux lab machine; do not fake
the `-no-pie` evidence on macOS.

### `void main` warning

Expected: `warning: return type of 'main' is not 'int'`. It still builds and
runs. Keep the sources verbatim; note the warning in the report.

### `ps4_extern_shadow` prints a huge/negative number for `x` in main

Expected. That is the uninitialized shadowing local. Run it 3 times and show
it varies while `display()` stays at 20.

### Part 3 prints `(nil)` or odd pointers for Effective Address

Expected. `a`/`b` are uninitialized before `malloc`; printing them is part
of the lesson. Do not initialize them — record what the machine shows.

### Addresses identical across PIE runs

Check the binary is really PIE (`file bin/ps4_static` should mention
`pie executable`). Rebuild with `make clean && make`, run as separate
commands (not one process), and confirm ASLR is not disabled on the machine.
