# cop4610-project2

"..."

## Group Members
- **Shreya Dangol**: spd23@fsu.edu
- **Leyla Dibra**: lsd23a@fsu.edu
- **Sophia Arenas**: sa23h@fsu.edu

## Division of Labor (After)
*we stuck to initial assignments and did our own parts*

| Part | Description                     | Completed By     | Notes |
|------|---------------------------------|------------------|----------|
| 1    | System Call Tracing             | Shreya Dangol    | Completed|
| 2    | Timer Module                    | Sophia Arenas    | Completed|
| 3a   | Adding System Calls             | Leyla Dibra      | Completed|
| 3b   | Kernel Compilation              |                  | Completed|
| 3c   | Threads                         |                  | Completed|
| 3d   | Linked List                     |                  | Completed|
| 3e   | Mutexes                         |                  | Completed|
| 3f   | Scheduling Algorithm            |                  | Completed|

### Extra Credit
- **Responsibilities**: 
- **Assigned to**:

- **Responsibilities**: 
- **Assigned to**: 

- **Responsibilities**: 
- **Assigned to**:

## File Listing

```


```
## How to Compile & Execute


## Development Log

### Shreya Dangol
Contributed to project planning, repository setup, kernel development, and elevator module implementation. Established SSH-based repository access from the lab VM and completed Part 1 by developing `empty.c` and `part1.c` and generating and validating their corresponding `strace` output. For Parts 3a and 3b, integrated the three elevator system calls into the Linux kernel source, compiled and installed Linux 6.16.12, configured GRUB to boot the updated kernel, and verified system call functionality using `syscheck`. Implemented the elevator module for Parts 3c through 3f, incorporating kernel thread management, per-floor pet queues, `issue_request` functionality, FIFO boarding, mutex synchronization, the LOOK scheduling algorithm, and `/proc/elevator` status reporting. Conducted functional testing to verify capacity and weight constraints, elevator stopping behavior, invalid start and stop requests, and proper kernel module unloading.
### Sophia Arenas


### Leyla Dibra


## Description of Group Meetings
**Meeting 1 — Task division:** The group met to divide the eight project
parts among the three members, assigning each person parts based on
availability and interest (see Division of Labor table above).

**Meeting 2 — Integration check:** The group met again to compare each
member's implementation and confirm that individual pieces of code
(parsing, execution, redirection, piping) were compatible with one another
before merging everything into a single shared codebase.

**Meeting 3 - Finalization and Testing:** The group met to confirm that merged code parts
ran smoothly with one another and minor errors were corrected. Code was ran on linprog
on individual computers to ensure execution was seamless across devices. 

## Extra Credit
- **Extra Credit 1:** 
- **Extra Credit 2:** 
- **Extra Credit 3:** 
