# Mini-Project 2: CPE333 Operating Systems

## Critical Section

- Form a group of around 6–8 students.
- Study the assigned topic.
- Rebuild kernel files to complete the assigned tasks.
- Prepare a 7–11 minute presentation.
  - Explain the topic. A fancy presentation is allowed.
  - Demonstrate your implementation.
- Write a report of no more than 25 pages, excluding the appendix.

## Topic: Virtual Memory

Create a page-fault counter tool for a specific process. The tool must accept the process ID to monitor and report:

- Number of memory regions.
- Total virtual address space.
- Minor page faults.
- Major page faults.

### Hints

- For a kernel module, see the [exception tracepoints](https://github.com/torvalds/linux/blob/master/include/trace/events/exceptions.h) and the [Linux tracepoints documentation](https://docs.kernel.org/trace/tracepoints.html).
- A process's address space is divided into Virtual Memory Areas (VMAs). Investigate `struct mm_struct`, `struct vm_area_struct`, and `get_task_mm`.
- Count the VMAs to obtain the number of memory regions. Calculate each region's size from its start and end addresses.
- Compare your result with `/proc/<pid>/maps`.
- For more information, see [Process Addresses](https://docs.kernel.org/mm/process_addrs.html).

