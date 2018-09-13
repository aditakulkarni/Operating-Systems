obj-m += chardevice.o

all: kernel_module producer_code consumer_code

kernel_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

producer_code: producer.c
	gcc producer.c -o producer.o

consumer_code: consumer.c
	gcc consumer.c -o consumer.o

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
