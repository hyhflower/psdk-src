EXTRA_CFLAGS = -g -O2
KVERSION = $(shell uname -r)
#obj-m += 8250_fintek.o 8250_pnp.o
obj-m += fintek_gpio_int.o

all: default

default:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	#make -C /DataDisk/hpeter/DMA-210UII/samsung_android_kernel_3.0 M=$(PWD) modules
	kmodsign sha256 MOK.priv MOK.der fintek_gpio_int.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -rf *.*~ *~ *.o *.ko *.mod.c .*.cmd .*.o.d .tmp_versions Module.symvers modules.order Module.markers
