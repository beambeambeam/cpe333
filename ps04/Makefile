CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O0 -g
NOPIE = -no-pie

# Default PIE binaries (modern gcc PIE is on by default).
PIE_BINS = bin/ps4_static bin/ps4_static_auto \
	bin/ps4_extern bin/ps4_extern_shadow \
	bin/ps4_mem bin/ps4_mem_full

# Fixed-address binaries for Step 4 of parts 1-2.
NOPIE_BINS = bin/ps4_static_nopie bin/ps4_static_auto_nopie \
	bin/ps4_extern_nopie bin/ps4_extern_shadow_nopie

.PHONY: all nopie clean check check-nopie

all: bin $(PIE_BINS)

nopie: bin $(NOPIE_BINS)

bin:
	mkdir -p bin

# --- Part 1: static storage class ---
bin/ps4_static: bin src/ps4_static.c
	$(CC) $(CFLAGS) src/ps4_static.c -o $@

bin/ps4_static_auto: bin src/ps4_static_auto.c
	$(CC) $(CFLAGS) src/ps4_static_auto.c -o $@

bin/ps4_static_nopie: bin src/ps4_static.c
	$(CC) $(CFLAGS) $(NOPIE) src/ps4_static.c -o $@

bin/ps4_static_auto_nopie: bin src/ps4_static_auto.c
	$(CC) $(CFLAGS) $(NOPIE) src/ps4_static_auto.c -o $@

# --- Part 2: extern storage class ---
bin/ps4_extern: bin src/ps4_extern.c
	$(CC) $(CFLAGS) src/ps4_extern.c -o $@

bin/ps4_extern_shadow: bin src/ps4_extern_shadow.c
	$(CC) $(CFLAGS) src/ps4_extern_shadow.c -o $@

bin/ps4_extern_nopie: bin src/ps4_extern.c
	$(CC) $(CFLAGS) $(NOPIE) src/ps4_extern.c -o $@

bin/ps4_extern_shadow_nopie: bin src/ps4_extern_shadow.c
	$(CC) $(CFLAGS) $(NOPIE) src/ps4_extern_shadow.c -o $@

# --- Part 3: malloc / realloc (PIE only, per assignment) ---
bin/ps4_mem: bin src/ps4_mem.c
	$(CC) $(CFLAGS) src/ps4_mem.c -o $@

bin/ps4_mem_full: bin src/ps4_mem_full.c
	$(CC) $(CFLAGS) src/ps4_mem_full.c -o $@

# Smoke test for the friend running the lab: each program 3x (PIE).
# Capture evidence manually; see guide.md for screenshot names.
check: all
	@set -e; \
	for prog in ps4_static ps4_static_auto ps4_extern ps4_extern_shadow; do \
		for i in 1 2 3; do \
			echo "== $$prog run $$i =="; \
			./bin/$$prog; \
		done; \
	done; \
	echo "== ps4_mem =="; ./bin/ps4_mem; \
	echo "== ps4_mem_full =="; ./bin/ps4_mem_full

check-nopie: nopie
	@set -e; \
	for prog in ps4_static_nopie ps4_static_auto_nopie ps4_extern_nopie ps4_extern_shadow_nopie; do \
		for i in 1 2 3; do \
			echo "== $$prog run $$i =="; \
			./bin/$$prog; \
		done; \
	done

clean:
	rm -rf bin
