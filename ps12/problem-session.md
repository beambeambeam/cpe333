# Problem Session 12: File System and Implementation

**CPE 333 Operating Systems – 1/2026**

The requirement slides are stored in `materials/requirements/PS12_2026.pdf`.

## Lab 1: Symbolic and hard links (Linux/Windows)

Experiment:

- Use the concept of a general graph directory structure to create link files with `ln` on Linux and `mklink` on Windows.
- Create hard and soft links to a source. After deleting the source file, what happens to the symbolic and hard link files?
- Create a level 1 link to the source and a level 2 link from level 1. See what happens if you delete the source or delete level 1.
- Create a link to a directory or folder. If you can, create a link that loops back to an upper-level folder. Then try `ls -R` and `dir /s` to list the folder recursively and see what happens.
- Can you create a link on drive D: to a source on drive C:?

Question: write a report that shows the experimental steps with Linux and Windows commands, describes what you do, discusses the results step by step, and answers the questions.

## Lab 2: Linux file type – named pipe (Linux only)

Experiment:

- Create a pipe file with `mkfifo pipeFile`.
- List it with `ls -la pipeFile`.
- Open two terminals. In the first, run `cat > pipeFile`. In the second, run `cat pipeFile`.
- Type `hello linux` in the first terminal and watch the second. Repeat with other input, then press `^D` to end the file.

Question: explain the concept of this file type and why the experiment behaves this way.

## Lab 3: Reading and writing files (Linux only)

Experiment:

- Create a file with `echo hello > foo`.
- Trace the system calls with `strace cat foo`. Expected output:

```text
read(3, "hello\n", 32768) = 6
write(1, "hello\n", 6hello) = 6
read(3, "", 32768) = 0
close(3) = 0
```

Question: explain these four lines. What do the numbers 3, 1, 6, and 32768 represent?
