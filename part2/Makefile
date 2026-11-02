obj-m += my_timer.o
my_timer-y := src/my_timer.o

KDIR ?= /lib/modules/$(shell uname -r)/build
MODULE_DIR := $(shell pwd)

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR) M=$(MODULE_DIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(MODULE_DIR) clean
