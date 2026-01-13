obj-m += proc_sys_var.o

KDIR ?= /home/egor/eltex_systems/kernel/linux
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

clean:
	make -C $(KDIR) M=$(PWD) clean
