# cop4610-project2

A Linux kernel program supporting system call tracing, a timer module, 
and a pet elevator module supporting FIFO boarding, thread management, 
mutex synchronization, scheduling, and proc status reporting.

## Group Members
- **Shreya Dangol**: spd23@fsu.edu
- **Leyla Dibra**: lsd23a@fsu.edu
- **Sophia Arenas**: sa23h@fsu.edu

## Division of Labor (After)
*we stuck to initial assignments and did our own parts*

| Part | Description                     | Completed By     | Notes    |
|------|---------------------------------|------------------|----------|
| 1    | System Call Tracing             | Shreya Dangol    | Completed|
| 2    | Timer Module                    | Sophia Arenas    | Completed|
| 3a   | Adding System Calls             | Leyla Dibra      | Completed|
| 3b   | Kernel Compilation              | Shreya, Leyla    | Completed|
| 3c   | Threads                         | Sophia, Leyla    | Completed|
| 3d   | Linked List                     | Shreya Dangol    | Completed|
| 3e   | Mutexes                         | Leyla Dibra      | Completed|
| 3f   | Scheduling Algorithm            | Shreya, Sophia   | Completed|


## File Listing

```
cop4610-project2-main/
├── .gitignore
├── Makefile
├── README.md
├── part1/
│   ├── Makefile
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   └── part1.trace
├── part2/
│   ├── Makefile
│   └── src/
│       └── my_timer.c
└── part3/
    ├── Makefile
    ├── syscalls.c
    ├── wrappers.h
    └── src/
        ├── elevator.h
        ├── elevator_main.c
        ├── elevator_syscalls.c
        └── elevator_thread.c

```
## How to Compile & Execute
###Requirements
- **Compiler**: `gcc` and `make`
- **Kernel**: Linux 6.16.12 built with system calls 548-550. Confirm with `uname -r`
- **Tools**: `strace` (Part 1), kernel headers for the running kernel (Parts 2 and 3)

### Compile Part 1: System Call Tracing
```bash
cd part1
make
strace -o empty.trace ./empty
strace -o part1.trace ./part1
wc -l empty.trace part1.trace
```
### Compile Part 2: Timer Kernel Module
```bash
cd part2
make
sudo insmod my_timer.ko
cat /proc/timer; sleep1; cat /proc/timer; sleep 3; cat /proc/timer
sudo rmmod my_timer
```
### Compile Part 3: Elevator Kernel Module
**Kernel setup (one time)**: copy `part3/syscalls.c` to `[kernel]/syscalls/syscalls.c`,
add `obj-y:= syscalls.o` in `[kernel]/syscalls/Makefile`, register 548-550 in
`arch/x86/entry/syscalls/syscall_64.tbl`, and the prototypes to `include/linux/syscalls.h`,
add `syscalls/` to `core-y` in the top-level kernel `Makefile`, then:
```bash
make menuconfig
make -j$(nproc)
sudo make modules_install
sudo make install
sudo reboot
```

**Build and load the module:**
```bash
cd part3
make
sudo insmod elevator.ko
```
`make` builds `elevator.ko` in `part3/`

**Unload and clean up:**
```bash
sudo rmmod elevator
make clean
```

## Development Log

### Shreya Dangol
Contributed to project planning, repository setup, kernel development, and elevator module implementation. 
Established SSH-based repository access from the lab VM and completed Part 1 by developing `empty.c` and 
`part1.c` and generating and validating their corresponding `strace` output. For Parts 3a and 3b, integrated 
the three elevator system calls into the Linux kernel source, compiled and installed Linux 6.16.12, configured 
GRUB to boot the updated kernel, and verified system call functionality using `syscheck`. Implemented the elevator 
module for Parts 3c through 3f, incorporating kernel thread management, per-floor pet queues, `issue_request` 
functionality, FIFO boarding, mutex synchronization, the LOOK scheduling algorithm, and `/proc/elevator` status 
reporting. Conducted functional testing to verify capacity and weight constraints, elevator stopping behavior, 
invalid start and stop requests, and proper kernel module unloading.

### Sophia Arenas
Developed Part 2 by implementing the `my_timer` kernel module to report the current time and elapsed time 
between proc reads using `ktime_get_real_ts64()`. Tested module loading, elapsed-time calculations, and proc 
entry removal after unloading. Contributed to Parts 3c and 3f through elevator thread and scheduling development, 
shared header integration, and resolution of naming and build issues. Conducted VM testing to verify FIFO boarding,
the five-pet capacity, the 50-pound weight limit, movement and loading delays, and processing of queued requests. 
Verified graceful shutdown with pets onboard, continued service after restarting, and responsive `/proc/elevator` 
reads during movement. Recorded test results to document observed behavior.

### Leyla Dibra
Developed part 3, by implementing `syscalls.c`, `elevator.h`, `elevator_syscalls.c`, `elevator_thread.c`, and `elevator_main.c`
'Wrote `syscalls.c` with `STUB_*` hooks and added it to the kernel source; registered calls 548–550 in `syscall_64.tbl` and `syscalls.h` (part 3a)
Assisted in the building of the 6.16,12 kernel with the new system calls on the lab VM (part 3b)
Wrote `elevator.h` shared structures, `elevator_syscalls.c` with mutex-protected handlers, and `elevator_thread.c` kthread loop that sleeps without holding the lock (parts 3c, 3e)
Wrote the module init/exit syscall hookup and proc read in `elevator_main.c`; verified build, load, and syscall return values on the VM
Tested all parts from a clean clone of `main`: Part 1 traces, Part 2 timer, Part 3 start/stop/restart, 3-minute stress test, and unloading with pets waiting; checked `dmesg` for kernel errors

## Bugs
No known bugs at time of submission

## Extra Credit 
N/A

## Description of Group Meetings
**Meeting 1 — Task division:** The group met to divide the eight project
parts among the three members, assigning each person parts based on
availability and interest (see Division of Labor table above).

**Meeting 2 — Integration check:** The group met to ensure differing parts were being integrated properly together. Reviewed the timer module, kernel 
modifications, and elevator components to ensure compatible function declarations, 
shared structures, and naming conventions. Build files were updated to compile the 
separate elevator source files together.

**Meeting 3 - Finalization and Testing:** The group tested the integrated project on 
the Linux VM and corrected build and runtime issues. Testing covered timer output, 
elevator movement, FIFO boarding, capacity and weight limits, timing delays, and stopping 
and restarting behavior.
