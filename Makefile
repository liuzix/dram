obj-m += dram.o
dram-objs := hello.o mock_data.o interface.o



all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean