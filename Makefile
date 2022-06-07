obj-m := msi_fans.o

msi_fans-y += src/msi_device.o src/msi_cooler_boost.o src/msi_fan.o msi_fans_main.o

all:	
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(shell pwd) modules_install
clean:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean