# kernel/Makefile
# Build mfw as an external Linux kernel module.
#
# Usage:
#   make        Build mfw.ko
#   make clean  Remove generated build artifacts
#
# KDIR may be overridden to build against a different kernel tree:
#   make KDIR=/path/to/kernel/build

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += mfw.o

mfw-y := src/mfw_main.o \
         src/mfw_device.o \
         src/mfw_rules.o
ccflags-y += -I$(src)/include
ccflags-y += -I$(src)/../include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
