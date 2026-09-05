# PS04 experiment notes

Record actual output here while running the experiments. Keep screenshots in
`ps04/results/`. Nothing has been run yet — this checklist is the handoff
for the friend running the lab on Linux.

## Environment (one machine for everything)

- OS / release / arch (`uname -a`, `cat /etc/os-release`):
- `cc --version`:
- `make --version`:
- `gcc -v` (to confirm `-no-pie` support):
- Screenshot: `00_environment.png`

## Part 1 — static storage class

Run each binary 3 times, back to back:

```bash
./bin/ps4_static
./bin/ps4_static_auto
./bin/ps4_static_nopie
./bin/ps4_static_auto_nopie
```

- `ps4_static` run 1/2/3 values of y:
- `ps4_static` addresses of y across runs (same or different?):
- `ps4_static_auto` run 1/2/3 values of y:
- `ps4_static_auto` addresses of y across runs:
- `_nopie` vs PIE addresses (stable or shifting?):
- Screenshots: `01_static_pie.png`, `02_static_auto_pie.png`,
  `03_static_nopie.png`, `04_static_auto_nopie.png`
- Discussion: why static climbs / auto repeats / PIE shifts / no-pie fixes:

## Part 2 — extern storage class

```bash
./bin/ps4_extern
./bin/ps4_extern_shadow
./bin/ps4_extern_nopie
./bin/ps4_extern_shadow_nopie
```

- `ps4_extern` value/address in main vs display (same?):
- `ps4_extern` addresses across 3 runs (PIE vs nopie):
- `ps4_extern_shadow` value/address in main (garbage? which address?):
- `ps4_extern_shadow` value/address in display (still global 20?):
- Screenshots: `05_extern_pie.png`, `06_extern_shadow_pie.png`,
  `07_extern_nopie.png`, `08_extern_shadow_nopie.png`
- Discussion: extern reference vs shadowing local, PIE effect:

## Part 3 — malloc / realloc

Run both on the SAME machine:

```bash
./bin/ps4_mem
./bin/ps4_mem_full
```

- Address of Pointer (`&a`, `&b`) — what region, stable across runs?:
- Effective Address (`a`, `b` before malloc) — garbage? why?:
- After malloc Pointer a vs Array c — heap vs stack? spacing?:
- After realloc Pointer a — moved or same? `&a[999]` offset check:
- `ps4_mem_full` extra block b — did a's addresses shift vs `ps4_mem`?:
- Screenshots: `09_mem.png`, `10_mem_full.png`

## Handoff

Copy to the report workspace with the screenshots:

- `notes/README.md` (this file, filled in)
- `results/*.png`
- Exact commands, full pasted outputs, compiler warnings, PIDs not needed
  (these programs print addresses, not PIDs)
