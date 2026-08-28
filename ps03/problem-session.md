# Problem Session 3: Process Monitoring and Scheduling

## 1. Linux process and job commands

Learn important Linux commands for manipulating and monitoring processes and jobs. Discuss:

1. The difference between the `top` and `ps` commands.
2. The purpose of the `nice` command and how to apply `nice` to a process.
3. How to kill processes and jobs. Give an example in the report.

## 2. Shell-script process states

Discuss the following situations.

### 2.1. Foreground and background execution

1. Create a shell script named `ss1_1.sh`. The script may contain only text.
2. Call `sleep` with a duration of 10 seconds in the script.
3. Run the script in the foreground:

   ```bash
   ./ss1_1.sh
   ```

   Wait for 10 seconds.

4. Run the script in the background:

   ```bash
   ./ss1_1.sh &
   ```

5. Discuss the difference between the two commands.

If the text file is not executable, use `chmod` to fix the permission. You may use a loop in the script and sleep for one second during each iteration.

### 2.2. Stopping and resuming a job

1. Create a shell script named `ss1_2.sh`.
2. Call `sleep` with a duration of 1000 seconds in the script.
3. Run the script in the foreground:

   ```bash
   ./ss1_2.sh
   ```

   Press `Ctrl+Z`.

4. Use the `jobs` command, show its result, and discuss the effect of pressing `Ctrl+Z`.
5. Show how to use `fg` and `bg` to resume `ss1_2.sh`. Discuss the difference between resuming the script with `fg` and with `bg`.

## 3. Linux GUI process-monitoring tools

Find and try a process-monitoring tool with a GUI on Linux. Explain:

1. What it is.
2. How to acquire it.
3. How to use it.
4. What values it can monitor.

## 4. Windows process-monitoring tools

Find and try a process-monitoring tool on Windows. Explain:

1. What it is.
2. How to acquire it.
3. How to use it.
4. What values it can monitor.

## 5. STCE scheduler simulation

Write a C function to simulate STCE. The provided `PS3.c` program reads input data in CSV format, converts it into a structure, and passes the structure to your function. Your function must process the structure and show which process is running in each time slot.

### Assumptions

- The function and context switch execute very quickly. Do not count them as CPU usage.
- A process that arrives at time `T` arrives at the very end of time `T-1`, but before the scheduler updates the ready queue for time `T`.
- If multiple processes arrive at the same time, sort them by `process_id`.
- If multiple processes have the same remaining time, sort them by `process_id`.

The files `case1.csv`, `case2.csv`, and `case3.csv` must be in the same directory as the executable. Run the program from the terminal, for example:

```bash
./PS3 case1.csv
```

All input cases and the code that processes them are already available in LEB2. You may use the provided `PS3.c` and fill in the blank space in the scheduler function with your implementation.

### Expected output format

```text
[0-3]: P0
[3-4]: IDLE
[4-8]: P1
[8-11]: P3
[11-21]: P2
```

Include the following in the report:

- A screenshot of the scheduler function.
- An overview explaining the function.
- Screenshots of the results for all three cases.
- A discussion of the results.

## Submission

Submit the document file to LEB2 before the due date. This problem session may be completed by a group of up to three or four students. Do not submit the C source file.
