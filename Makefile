obj-m += rootkit.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
	sudo insmod rootkit.ko

unload:
	sudo rmmod rootkit

dirtest:
	mkdir test

rdirtest:
	rm -r test