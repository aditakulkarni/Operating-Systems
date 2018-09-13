#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#define DEVICE_NAME "process_list"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adita");
MODULE_DESCRIPTION("A character device module");

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset);
static int device_open(struct inode *inodep, struct file *filep);
static int ret_number;
static char read_buffer[10000];
static char *state_buffer[21] = { "TASK_RUNNING", "TASK_INTERRUPTIBLE", "TASK_UNINTERRUPTIBLE", "TASK_STOPPED", "TASK_TRACED", "EXIT_DEAD", "EXIT_ZOMBIE", "TASK_DEAD", "TASK_WAKEKILL", "TASK_WAKING", "TASK_PARKED", "TASK_NOLOAD", "TASK_NEW", "TASK_STATE_MAX", "TASK_KILLABLE", "TASK_STOPPED", "TASK_TRACED", "TASK_IDLE", "TASK_NORMAL", "TASK_ALL", "TASK_REPORT"};
static int int_state_buffer[21] = {0,1,2,4,8,16,32,64,128,256,512,1024,2048,4096,TASK_KILLABLE,TASK_STOPPED,TASK_TRACED,TASK_IDLE,TASK_NORMAL,TASK_ALL,TASK_REPORT};
static char *ptr;
static int len = 0;
static int task_state_num;
struct task_struct *task_list;
struct task_struct *parent;

static struct file_operations fops = {
	.read = device_read,
	.open = device_open
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
    return 0;
}

static void __exit char_cleanup(void)
{
	//Deregister the char device
    printk(KERN_INFO "Cleaning up module.\n");
	misc_deregister(&my_misc_device);
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	int bytes_length = 0, ret_error=0;
	//If nothing to copy, return 0
	if(*ptr == 0)
		return 0;

	//Copy bytes to user
	while(length && *ptr) {
		ret_error = copy_to_user(buffer++,ptr++,1);
		if(ret_error != 0) {
			printk(KERN_INFO "\nError in copying..\n");
			return -EFAULT;
		}
		length--;
		bytes_length++;
		offset++;
	}
	//Return number of bytes read
	return bytes_length;
}

static int device_open(struct inode *inodep, struct file *filep)
{
	int i;

	//Load information of processes to buffer
	for_each_process(task_list) {
		parent = task_list->parent;
		task_state_num = task_list->state;

		//Copy the information of processes to buffer
		len  += sprintf(read_buffer+len, "\nPID: %d PPID: %d CPU: %u State: ",task_list->pid,parent->pid,task_cpu(task_list));

		//Check state of process
		if(task_state_num == 0) {
			len += sprintf(read_buffer+len,"%s ","TASK_RUNNING");
		}
		else if(task_state_num == -1) {
			len += sprintf(read_buffer+len,"%s ","ERROR");
		}
		else {
			for(i=0;i<21;i++) {
				if(task_state_num & int_state_buffer[i]) {
					len += sprintf(read_buffer+len,"%s ",state_buffer[i]);
				}
			}
		}
		len += sprintf(read_buffer+len,"\n");
	}
	ptr = read_buffer;
	return 0;
}

module_init(char_init);
module_exit(char_cleanup);
