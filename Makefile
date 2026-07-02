KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all kernel user clean load unload reload logs status

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD)/kernel modules

user:
	$(MAKE) -C user

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/kernel clean
	$(MAKE) -C user clean

load:
	sudo insmod kernel/mfw.ko

unload:
	sudo rmmod mfw

reload: unload load

logs:
	sudo dmesg -w

status:
	sudo cat /sys/kernel/debug/mfw/rules
