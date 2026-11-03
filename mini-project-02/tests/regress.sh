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

CHECKS=${*:-retarget}
for c in $CHECKS; do "check_$c"; done
echo 0 > $T/target
exit $FAIL
