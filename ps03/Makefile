CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O0 -g

.PHONY: all clean scripts check

all: bin/PS3 scripts

bin:
	mkdir -p bin

bin/PS3: bin src/PS3.c
	$(CC) $(CFLAGS) src/PS3.c -o $@

scripts:
	chmod +x src/ss1_1.sh src/ss1_2.sh

check: bin/PS3
	@set -e; \
	for case_file in case1.csv case2.csv case3.csv; do \
		echo "== $$case_file =="; \
		./bin/PS3 "$$case_file"; \
	done

clean:
	rm -rf bin
