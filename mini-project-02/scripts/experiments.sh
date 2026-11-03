#!/bin/sh
# Run every experiment in REPORT.md and save raw output to results/.
# Run inside the Linux VM from the project root: sh scripts/experiments.sh
set -eu
cd "$(dirname "$0")/.."
OUT=results
BIG=/tmp/pfcount-big.bin
PF="sudo tools/pfstat"
mkdir -p "$OUT"

make -s all >/dev/null 2>&1
lsmod | grep -q '^pfcount' && sudo rmmod pfcount
sudo insmod module/pfcount.ko
[ -f "$BIG" ] || dd if=/dev/urandom of="$BIG" bs=1M count=32 status=none

# Print a command line, quoting arguments with spaces or shell characters.
show() {
	printf '$'
	for a; do
		case $a in
		*[[:space:]\'\"\<\>\|\&\;\$]*) printf " '%s'" "$a" ;;
		*) printf ' %s' "$a" ;;
		esac
	done
	echo
}

run() { # name, description, command...
	name=$1; desc=$2; shift 2
	echo "== $name: $desc"
	{ show "$@"; "$@" 2>&1; } > "$OUT/$name.txt"
}

{
	echo "date:    $(date -u +%Y-%m-%dT%H:%M:%SZ)"
	echo "kernel:  $(uname -r) $(uname -m)"
	echo "distro:  $(. /etc/os-release; echo "$PRETTY_NAME")"
	echo "gcc:     $(gcc --version | head -1)"
	echo "page:    $(getconf PAGESIZE) bytes"
	echo "thp:     $(cat /sys/kernel/mm/transparent_hugepage/enabled)"
	echo "fault_around_bytes: $(sudo cat /sys/kernel/debug/fault_around_bytes)"
	echo "read_ahead_kb:      $(cat /sys/block/$(lsblk -no pkname "$(findmnt -no source -T /tmp)" | head -1)/queue/read_ahead_kb)"
	echo "cpus:    $(nproc)"
	echo "memory:  $(free -m | awk '/Mem:/{print $2" MB"}')"
} > "$OUT/00-environment.txt"

# E1: one-shot report vs procfs on an idle process and on PID 1
sleep 300 & SLEEP=$!
sleep 0.5
run e1a-report-sleep "report vs /proc on sleep(1)" $PF $SLEEP --maps
run e1b-report-init  "report vs /proc on PID 1"    $PF 1
kill $SLEEP

# E2-E6: controlled faults, counted live and checked against getrusage()
run e2-anon     "64 MB anonymous, 4 KB pages" $PF -- tests/faultgen anon:64
run e3-filecold "32 MB file, cold cache, MADV_RANDOM" $PF -- tests/faultgen file:$BIG
cat "$BIG" > /dev/null
run e4-filewarm "32 MB file, warm cache, MADV_RANDOM" $PF -- tests/faultgen filewarm:$BIG
run e5-fileseq  "32 MB file, cold cache, readahead on" $PF -- tests/faultgen fileseq:$BIG
run e6-thp      "64 MB anonymous, transparent huge pages" $PF -- tests/faultgen thp:64

# E7: region count and address space over time (watch mode)
run e7-timeline "timeline: anon, file, free" \
	$PF -w 0.5 -- tests/faultgen sleep:1 anon:64 sleep:1 file:$BIG sleep:1 free sleep:1

# E8: real programs
run e8a-python    "python3 -c pass"                  $PF -- python3 -c pass
run e8b-ls        "sh -c ls, process only"           $PF -- sh -c 'ls -R /usr/include >/dev/null'
run e8c-ls-follow "sh -c ls, with descendants"       $PF -f -- sh -c 'ls -R /usr/include >/dev/null'
run e8d-gcc       "gcc driver, process only"         $PF -- gcc -O2 -o /tmp/fg tests/faultgen.c
run e8e-gcc-follow "gcc with cc1/as/ld descendants"  $PF -f -- gcc -O2 -o /tmp/fg tests/faultgen.c

sudo rmmod pfcount
echo "results in $OUT/"
