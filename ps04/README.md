# CPE333 PS04 – C Storage Class and PIE

Problem Session 4 covers C storage classes (`auto`, `static`, `extern`),
Position Independent Executables (PIE) versus `-no-pie`, and dynamic memory
(`malloc` / `realloc` / `free`) address behavior.

## Layout

- `problem-session.md` – assignment requirements.
- `c_storge_class_and_pie.md` – lecture notes on storage classes and PIE.
- `guide.md` – universal step-by-step experiment and report guide.
- `src/ps4_static.c` – Part 1 original (`static int y = 5;`).
- `src/ps4_static_auto.c` – Part 1 variant with `static` removed.
- `src/ps4_extern.c` – Part 2 original (`extern` in `main` and `display`).
- `src/ps4_extern_shadow.c` – Part 2 variant with `extern` removed in `main`.
- `src/ps4_mem.c` – Part 3 original (pointer `b` code commented out).
- `src/ps4_mem_full.c` – Part 3 variant with both places uncommented.
- `results/` – experiment screenshots and output evidence.
- `notes/` – observation checklist and experiment notes.

The CPE333 checkout intentionally contains no report files. Keep the report,
PDF, and screenshots in the group's separate submission workspace.

## Platform

***This lab must be done on a Unix-based OS.*** The `-no-pie` comparison
requires Linux (Debian/Ubuntu recommended, WSL2 Ubuntu works for
command-line evidence). macOS can build the default PIE binaries for
preparation, but collect the final `-no-pie` evidence on Linux.

Native Windows is not supported for the C programs. Use WSL2, a Linux VM, or
another Unix-like environment.

## Debian preparation

Install the compiler and build tools:

```bash
sudo apt update
sudo apt install -y build-essential
```

## Build

From this directory (NOT run yet — handoff for the friend running the lab):

```bash
make        # default PIE binaries
make nopie  # fixed-address -no-pie binaries (Linux only)
```

Expected binaries:

```text
bin/ps4_static  bin/ps4_static_auto
bin/ps4_extern  bin/ps4_extern_shadow
bin/ps4_mem     bin/ps4_mem_full
bin/ps4_static_nopie  bin/ps4_static_auto_nopie
bin/ps4_extern_nopie  bin/ps4_extern_shadow_nopie
```

The sources keep the handout's `void main()` verbatim so report output
matches the assignment. Expect one compiler warning per file:

```text
warning: return type of 'main' is not 'int'
```

That warning is harmless for this lab. Do not "fix" it to `int main()` in
the committed sources; the report must explain the handed-out programs.

## Run (friend's part)

Part 1 — static, default PIE, 3 runs each:

```bash
./bin/ps4_static
./bin/ps4_static
./bin/ps4_static
./bin/ps4_static_auto
./bin/ps4_static_auto
./bin/ps4_static_auto
```

Part 1 — redo everything with fixed addresses (Linux):

```bash
./bin/ps4_static_nopie
./bin/ps4_static_nopie
./bin/ps4_static_nopie
./bin/ps4_static_auto_nopie
./bin/ps4_static_auto_nopie
./bin/ps4_static_auto_nopie
```

Part 2 — extern, same pattern:

```bash
./bin/ps4_extern
./bin/ps4_extern
./bin/ps4_extern
./bin/ps4_extern_shadow
./bin/ps4_extern_shadow
./bin/ps4_extern_shadow
./bin/ps4_extern_nopie
./bin/ps4_extern_nopie
./bin/ps4_extern_nopie
./bin/ps4_extern_shadow_nopie
./bin/ps4_extern_shadow_nopie
./bin/ps4_extern_shadow_nopie
```

Part 3 — memory allocation, same machine for both runs:

```bash
./bin/ps4_mem
./bin/ps4_mem_full
```

Shortcut smoke test (runs the PIE set; `-no-pie` set separately):

```bash
make check
make check-nopie
```

See `guide.md` for the exact screenshot list and what to record.

## What to expect (for the report discussion)

- `static int y`: initialized once, keeps its value across loop iterations,
  so the printed value climbs (6, 7, 8) at one stable address per process.
- `int y` (static removed): re-created per iteration, so the printed value
  repeats (6, 6, 6). The stack address may repeat or shift slightly.
- Default PIE: global/static addresses (`&y`, `&x`) change between separate
  process runs because the loader picks a different base address each time.
- `-no-pie`: global/static addresses stay identical across runs because the
  executable loads at a fixed virtual address.
- `extern int x` in both functions: `main` and `display` print the same
  value (20) and the same address — one global object.
- `extern` removed in `main`: the local `int x;` shadows the global. Its
  value is indeterminate (garbage, differs per run), while `display()` still
  prints the global 20. Addresses differ.
- Part 3: `&a` / `&b` are stack addresses of the pointer variables; `a` / `b`
  before `malloc` are indeterminate; after `malloc` they point into the heap;
  `c` / `&c[0]` live on the stack; `realloc` to 1000 ints may move the block,
  so `a` can change. Uncommenting the `b` allocation adds a second heap block
  and can shift where later allocations (including the `realloc` result) land.

## Report handoff

This repository does not include the report source or screenshot directory.
Record exact Linux output in `notes/README.md`, keep screenshots in
`results/`, then transfer both to the group's separate submission workspace.
Use the assignment requirements to complete the report there.

The CPE333 Makefile intentionally has no report target. A fresh checkout must
build without any other repository (nothing has been run here yet):

```bash
make clean
make
make nopie
```
