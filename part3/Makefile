# obj-m := elevator.o
# elevator-objs := src/elevator.o

# ccflags-y := -g -Wno-error
# KDIR := /lib/modules/$(shell uname -r)/build

# all:
# 	$(MAKE) -C $(KDIR) M=$(PWD) modules

# clean:
# 	$(MAKE) -C $(KDIR) M=$(PWD) clean

ifneq ($(KERNELRELEASE),)
obj-m := elevator.o
elevator-y := src/elevator_main.o src/elevator_syscalls.o src/elevator_thread.o
else
KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all clean

all:
	$(MAKE) -C "$(KDIR)" M="$(CURDIR)" modules

clean:
	$(MAKE) -C "$(KDIR)" M="$(CURDIR)" clean
endif
