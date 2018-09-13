#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h> 
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/slab.h>

#define DEVICE_NAME "line_pipe"
#define pipe_size 10

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adita");
MODULE_DESCRIPTION("A named pipe module");

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

//Define semaphores and mutex
static DEFINE_SEMAPHORE(sem1);
static DEFINE_SEMAPHORE(sem2);
static DEFINE_MUTEX(mut);

static int ret_number;
static int i;
static int read_index = 0;
static int write_index = 0;

//Pipe
static char * kernel_buffer[pipe_size];

static struct file_operations fops = {
	.read = device_read,
	.write = device_write
};

static struct miscdevice my_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fops
};

static int __init char_init(void)
{
	//Register the char device
	ret_number = misc_register(&my_misc_device);
	if(ret_number) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", ret_number);
	  return ret_number;
	}
    printk(KERN_INFO "Device successfully registered..\n");

	//Initialize semaphores and mutex
	sema_init(&sem1, pipe_size);
	sema_init(&sem2, 0);
	mutex_init(&mut);
    return 0;
}

static void __exit char_cleanup(void)
{
	//Deregister the char device
	printk(KERN_INFO "Cleaning up module.\n");
	misc_deregister(&my_misc_device);

	//Free memory allocated to pipe(kernel_buffer)
	for(i=0;i<pipe_size;i++)
		kfree(kernel_buffer[i]);
}

static ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset)
{
	if(down_interruptible(&sem2)) {
		read_index = write_index;
		return -ERESTARTSYS;
	}
	if(mutex_lock_interruptible(&mut)) {
		return -EINTR;
	}
	if(read_index > pipe_size-1) {
		read_index = 0;
	}
	//Read from pipe
	ret_number = copy_to_user(buffer, kernel_buffer[read_index], length);
	if(ret_number != 0) {
		printk(KERN_INFO "\nError in copying to user..\n");
		return -EFAULT;
	}
	offset+=length;
	read_index++;
	mutex_unlock(&mut);
	up(&sem1);
	return length;
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t length, loff_t *offset)
{
	if(down_interruptible(&sem1)) {
		return -ERESTARTSYS;
	}
	if(mutex_lock_interruptible(&mut)) {
		return -EINTR;
	}
	if(write_index > pipe_size-1) {
		write_index = 0;
	}
	kernel_buffer[write_index] = kmalloc(sizeof(char)*length,GFP_KERNEL);
	if(!kernel_buffer[write_index]) {
		printk(KERN_INFO "\nError in allocating memory..\n");
	}
	//Write to pipe
	ret_number = copy_from_user(kernel_buffer[write_index], buffer, length);
	if(ret_number != 0) {
		printk(KERN_INFO "\nError in copying from user..\n");
		return -EFAULT;
	}
	offset+=length;
	write_index++;
	mutex_unlock(&mut);
	up(&sem2);
	return length;
}

module_init(char_init);
module_exit(char_cleanup);
