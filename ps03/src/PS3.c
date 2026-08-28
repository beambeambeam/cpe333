#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100

// Process data loaded from one CSV row.
typedef struct {
    int pid;        // process ID
    int arrival;    // arrival time
    int burst;      // original CPU burst
    int remaining;  // remaining CPU burst
    int finished;   // 0 = not finished, 1 = done
} Process;

/*
 * Return the ready process with the shortest remaining time.
 * Process ID breaks ties, as required by the problem session.
 */
static int select_process(const Process *procs, int n, int time)
{
    int selected = -1;

    for (int i = 0; i < n; i++) {
        if (procs[i].finished || procs[i].remaining <= 0 ||
            procs[i].arrival > time) {
            continue;
        }

        if (selected == -1 ||
            procs[i].remaining < procs[selected].remaining ||
            (procs[i].remaining == procs[selected].remaining &&
             procs[i].pid < procs[selected].pid)) {
            selected = i;
        }
    }

    return selected;
}

static void print_interval(const Process *procs, int selected, int start,
                           int end)
{
    if (selected == -1) {
        printf("[%d-%d]: IDLE\n", start, end);
    } else {
        printf("[%d-%d]: P%d\n", start, end, procs[selected].pid);
    }
}

/*
 * Simulate shortest-time-to-completion first (STCE).
 *
 * The scheduler is evaluated at every integer time slot.  Therefore a process
 * arriving at time T is included before the decision for slot [T, T+1).
 * Adjacent slots running the same process are combined into one output line.
 */
void simulate_stfc(Process *procs, int n)
{
    int time = 0;
    int completed = 0;
    int selected = -2; // -2 means that no output interval exists yet
    int interval_start = 0;

    for (int i = 0; i < n; i++) {
        if (procs[i].remaining <= 0) {
            procs[i].remaining = 0;
            procs[i].finished = 1;
            completed++;
        }
    }

    while (completed < n) {
        int next = select_process(procs, n, time);

        if (next != selected) {
            if (selected != -2) {
                print_interval(procs, selected, interval_start, time);
            }
            selected = next;
            interval_start = time;
        }

        if (next == -1) {
            // No process is ready. Advance until the next scheduling slot.
            time++;
            continue;
        }

        procs[next].remaining--;
        time++;

        if (procs[next].remaining == 0) {
            procs[next].finished = 1;
            completed++;
        }
    }

    if (selected != -2) {
        print_interval(procs, selected, interval_start, time);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <csvfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    Process procs[MAX_PROCESSES];
    int n = 0;
    char line[256];

    // Skip the CSV header.
    if (!fgets(line, sizeof(line), fp)) {
        fprintf(stderr, "Input file is empty\n");
        fclose(fp);
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), fp) && n < MAX_PROCESSES) {
        int pid;
        int arrival;
        int burst;

        if (sscanf(line, "%d,%d,%d", &pid, &arrival, &burst) != 3) {
            continue;
        }
        if (burst < 0) {
            fprintf(stderr, "Burst time cannot be negative: %s", line);
            fclose(fp);
            return EXIT_FAILURE;
        }

        procs[n].pid = pid;
        procs[n].arrival = arrival;
        procs[n].burst = burst;
        procs[n].remaining = burst;
        procs[n].finished = 0;
        n++;
    }

    fclose(fp);
    simulate_stfc(procs, n);
    return EXIT_SUCCESS;
}
