obj-m += chardevice.o

all: kernel_module user_code

kernel_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

user_code: test.c
	gcc test.c -o test.out

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test.out
