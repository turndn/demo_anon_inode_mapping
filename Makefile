obj-m := mapping_module.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

insmod:
	sudo insmod mapping_module.ko

rmmod:
	sudo rmmod mapping_module.ko

reload:
	-make rmmod
	make insmod

user:
	gcc -Wall -o user user.c

clean:
	-rm user
	rm *.o
	rm *.ko
	rm *.mod.c
