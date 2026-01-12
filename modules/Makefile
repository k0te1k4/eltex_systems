obj-m += proc_var.o

KDIR ?= /home/egor/eltex_systems/kernel/linux
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
