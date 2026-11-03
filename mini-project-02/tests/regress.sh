#!/bin/sh
# Regression checks for review findings on PR #11.
# Run inside the Linux VM from the project root, module loaded:
#   sudo sh tests/regress.sh [check...]     (default: all checks)
set -u
cd "$(dirname "$0")/.."
T=/proc/pfcount
FAIL=0

stat_of() { grep "^$1:" $T/stats | awk '{print $2}'; }
pass() { echo "PASS $1"; }
fail() { echo "FAIL $1: $2"; FAIL=1; }

# A faults in a loop, B is stopped. After A -> B, B must show 0 live faults.
check_retarget() {
	tests/faultgen anon:8 free loop >/dev/null & A=$!
	sleep 100 & B=$!
	kill -STOP $B
	sleep 0.3
	bad=0
	for i in $(seq 1 50); do
		echo $A > $T/target
		echo $B > $T/target
		sleep 0.02
		n=$(stat_of live_min_flt)
		[ "$n" = 0 ] || { bad=$((bad + 1)); last=$n; }
	done
	kill -9 $A $B 2>/dev/null; wait $A $B 2>/dev/null
	[ $bad = 0 ] && pass retarget || fail retarget "$bad/50 trials: stopped B got live_min_flt=$last"
}

# Target P exits. A new process reuses number P. P's counters must not move.
check_pidreuse() {
	for try in 1 2 3 4 5; do
		tests/faultgen sleep:1 >/dev/null & P=$!
		echo $P > $T/target
		wait $P
		before=$(stat_of live_min_flt)
		echo $((P - 1)) > /proc/sys/kernel/ns_last_pid
		tests/faultgen anon:16 >/dev/null & Q=$!
		wait $Q
		[ $Q = $P ] || continue		# another process took the number
		after=$(stat_of live_min_flt)
		state=$(stat_of state)
		if [ "$after" = "$before" ] && [ "$state" = exited ]; then
			pass "pidreuse (pid $P reused)"
		else
			fail pidreuse "pid $P reused: live_min_flt $before -> $after, state=$state"
		fi
		return
	done
	fail pidreuse "could not force PID reuse"
}

# Eight threads exit at the same time after main's pthread_exit(). The exit
# snapshot must be taken every time.
check_threadexit() {
	miss=0
	for i in $(seq 1 100); do
		tests/threadexit & P=$!
		echo $P > $T/target
		wait $P
		grep -q '^exit_regions:' $T/stats || miss=$((miss + 1))
	done
	[ $miss = 0 ] && pass "threadexit (100 runs)" || fail threadexit "$miss/100 runs had no exit snapshot"
}

# Threads start and exit in a loop. The lifetime min_flt sum must never go
# down: an exiting thread must not be counted twice or missed for a moment.
check_lifetime() {
	tests/threadchurn & P=$!
	echo $P > $T/target
	out=$(python3 - "$T/stats" <<'PY'
import sys
prev = drops = 0
for _ in range(20000):
    with open(sys.argv[1]) as f:
        n = int(next(l for l in f if l.startswith("min_flt:")).split()[1])
    if n < prev:
        drops += 1
    prev = n
print(drops)
PY
)
	kill -9 $P; wait $P 2>/dev/null
	[ "$out" = 0 ] && pass "lifetime (20000 reads)" || fail lifetime "min_flt went down $out times in 20000 reads"
}

CHECKS=${*:-retarget pidreuse threadexit lifetime}
for c in $CHECKS; do "check_$c"; done
echo 0 > $T/target
exit $FAIL
