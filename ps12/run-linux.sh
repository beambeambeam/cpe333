#!/usr/bin/env bash
# Run the PS12 Linux experiments and save transcripts in results/.
# Usage (on Linux, from ps12/): ./run-linux.sh
# Lab 3 needs strace; Lab 2 needs tmux to drive two real terminals.
set -u

RESULTS_DIR="${RESULTS_DIR:-$(cd "$(dirname "$0")" && pwd)/results}"
WORK="$(mktemp -d /tmp/ps12.XXXXXX)"
mkdir -p "$RESULTS_DIR"

# Print a command like a shell prompt, then run it in the current shell.
run() {
    printf '$ %s\n' "$*"
    eval "$*" 2>&1
    local rc=$?
    [ "$rc" -ne 0 ] && printf '[exit status %d]\n' "$rc"
    return 0
}

section() { printf '\n### %s\n' "$*"; }

environment() {
    run uname -srm
    run 'grep PRETTY_NAME /etc/os-release'
    run 'ls --version | head -1'
    run 'strace --version | head -1'
    run 'tmux -V'
    run date
}

lab1_links() {
    cd "$WORK" && mkdir lab1 && cd lab1

    section "1.1 Hard and soft link to a source, then delete the source"
    run 'echo "original data" > source.txt'
    run 'ln source.txt hard.txt'
    run 'ln -s source.txt soft.txt'
    run 'ls -li'
    run 'cat hard.txt'
    run 'cat soft.txt'
    run 'rm source.txt'
    run 'ls -li'
    run 'cat hard.txt'
    run 'cat soft.txt'
    run 'readlink soft.txt'
    run 'echo "new source" > source.txt'
    run 'cat soft.txt'
    run 'cat hard.txt'

    section "1.2a Symbolic link chain: delete the source"
    run 'mkdir sym-a && cd sym-a'
    run 'echo "level 0 data" > source.txt'
    run 'ln -s source.txt level1'
    run 'ln -s level1 level2'
    run 'ls -li'
    run 'cat level2'
    run 'rm source.txt'
    run 'cat level1'
    run 'cat level2'
    run 'cd ..'

    section "1.2b Symbolic link chain: delete level 1"
    run 'mkdir sym-b && cd sym-b'
    run 'echo "level 0 data" > source.txt'
    run 'ln -s source.txt level1'
    run 'ln -s level1 level2'
    run 'rm level1'
    run 'ls -li'
    run 'cat source.txt'
    run 'cat level2'
    run 'cd ..'

    section "1.2c Hard link chain: delete the source"
    run 'mkdir hard-a && cd hard-a'
    run 'echo "level 0 data" > source.txt'
    run 'ln source.txt level1'
    run 'ln level1 level2'
    run 'ls -li'
    run 'rm source.txt'
    run 'ls -li'
    run 'cat level1'
    run 'cat level2'
    run 'cd ..'

    section "1.2d Hard link chain: delete level 1"
    run 'mkdir hard-b && cd hard-b'
    run 'echo "level 0 data" > source.txt'
    run 'ln source.txt level1'
    run 'ln level1 level2'
    run 'rm level1'
    run 'ls -li'
    run 'cat level2'
    run 'cd ..'

    section "1.3 Link to a directory and a loop back to an upper level"
    run 'mkdir -p top/sub'
    run 'echo "file in sub" > top/sub/file.txt'
    run 'ln top top-hard'
    run 'ln -s top top-soft'
    run 'ls -l top-soft/sub'
    run 'ln -s .. top/sub/up'
    run 'ls -l top/sub'
    run 'ls top/sub/up/sub/up/sub/up/sub'
    run 'ls -R top'
    run 'ls -RL top'
    run 'find -L top'
    run 'realpath top/sub/up/sub/up/sub/up'

    section "1.4 Link from one file system to another (Linux equivalent of C: and D:)"
    run 'echo "data on file system A" > /tmp/ps12-fsA.txt'
    run 'df -T /tmp /dev/shm | cat'
    run 'stat -c "%n device=%d" /tmp /dev/shm'
    run 'ln /tmp/ps12-fsA.txt /dev/shm/ps12-hard.txt'
    run 'ln -s /tmp/ps12-fsA.txt /dev/shm/ps12-soft.txt'
    run 'ls -l /dev/shm/ps12-soft.txt'
    run 'cat /dev/shm/ps12-soft.txt'
    run 'rm -f /tmp/ps12-fsA.txt /dev/shm/ps12-soft.txt'
}

lab2_fifo_setup() {
    cd "$WORK" && mkdir lab2 && cd lab2
    run 'mkfifo pipeFile'
    run 'ls -la pipeFile'
    run 'stat pipeFile'
    run 'file pipeFile 2>/dev/null || stat -c %F pipeFile'
    section "Writer with no reader blocks in open(); stopped after 2 s"
    run "timeout 2 sh -c 'echo hello > pipeFile'"
}

# Drive two real terminals with tmux: T1 writes, T2 reads.
lab2_fifo_terminals() {
    cd "$WORK/lab2"
    local s=ps12
    # Use a private tmux server and explicit Bash panes. The host's default
    # shell may be fish, which does not accept the PS1 setup below.
    local socket="ps12-$$"
    tmux -L "$socket" kill-server 2>/dev/null
    tmux -L "$socket" new-session -d -s "$s" -x 60 -y 12 -n T1 'exec bash --noprofile --norc -i'
    tmux -L "$socket" new-window -t "$s" -n T2 'exec bash --noprofile --norc -i'
    snap() {
        printf -- '--- %s ---\n' "$1"
        printf '[Terminal 1]\n'; tmux -L "$socket" capture-pane -p -t "$s:T1" | sed '/^$/d'
        printf '[Terminal 2]\n'; tmux -L "$socket" capture-pane -p -t "$s:T2" | sed '/^$/d'
    }
    tmux -L "$socket" send-keys -t "$s:T1" "PS1='T1\$ '; cd $WORK/lab2; clear" Enter
    tmux -L "$socket" send-keys -t "$s:T2" "PS1='T2\$ '; cd $WORK/lab2; clear" Enter
    sleep 0.5
    tmux -L "$socket" send-keys -t "$s:T1" 'cat > pipeFile' Enter
    sleep 1
    snap "Step 1: T1 runs cat > pipeFile; no reader yet, so T1 waits in open()"
    tmux -L "$socket" send-keys -t "$s:T2" 'cat pipeFile' Enter
    sleep 1
    snap "Step 2: T2 runs cat pipeFile; both ends are now open"
    tmux -L "$socket" send-keys -t "$s:T1" 'hello linux' Enter
    sleep 1
    snap "Step 3: T1 types hello linux"
    tmux -L "$socket" send-keys -t "$s:T1" 'second line from terminal 1' Enter
    tmux -L "$socket" send-keys -t "$s:T1" 'third line' Enter
    sleep 1
    snap "Step 4: T1 types two more lines"
    tmux -L "$socket" send-keys -t "$s:T1" C-d
    sleep 1
    snap "Step 5: T1 presses ^D; both cat commands exit"
    tmux -L "$socket" kill-server
    printf -- '--- After the session ---\n'
    run 'ls -la pipeFile'
}

lab3_strace() {
    cd "$WORK" && mkdir lab3 && cd lab3
    run 'echo hello > foo'
    # script(1) gives cat a pseudo-terminal, so fd 1 is a terminal as in the lab slide.
    run "script -qc 'strace cat foo' /dev/null | tr -d '\r'"
    section "Only the file-related calls, on a terminal"
    run "script -qc 'strace -e trace=openat,read,write,close cat foo' /dev/null | tr -d '\r'"
    section "Same trace with stdout redirected to a regular file"
    run 'strace -e trace=openat,read,write,copy_file_range,close cat foo > out.txt'
    run 'cat out.txt'
}

environment      | tee "$RESULTS_DIR/00_environment.txt"
lab1_links       | tee "$RESULTS_DIR/01_lab1_links.txt"
lab2_fifo_setup  | tee "$RESULTS_DIR/02_lab2_fifo_setup.txt"
lab2_fifo_terminals | tee "$RESULTS_DIR/03_lab2_fifo_terminals.txt"
lab3_strace      | tee "$RESULTS_DIR/04_lab3_strace.txt"

rm -rf "$WORK"
