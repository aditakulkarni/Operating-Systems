#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kprobes.h>
#include <linux/time.h>
#include <linux/proc_fs.h>	

#define DEVICE_NAME "track_address"
#define N 2000

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adita");
MODULE_DESCRIPTION("Track page faults module");

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static int j_handle_mm_fault(struct vm_area_struct *vma, unsigned long address,	unsigned int flags);

static int process_id = 1;
module_param(process_id, int, 0644);

struct timespec ts;
static int index = 0, ret_number, read_index = 0, i;
static char * kernel_buffer[N];
struct proc_dir_entry *p;

static struct jprobe my_jprobe = {
	.entry = j_handle_mm_fault,
	.kp = {
		.symbol_name = "handle_mm_fault",
	},
};

static struct file_operations fops = {
	.read = device_read
};

static int __init char_init(void) {
	//Register the proc char device
	p = proc_create(DEVICE_NAME,0,NULL,&fops);
	if(p == NULL) {
		printk(KERN_INFO "Proc Failed\n");
		return -1;
	}
    printk(KERN_INFO "Device successfully registered..\n");

	//Register the jprobe
	ret_number = register_jprobe(&my_jprobe);
	if (ret_number < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret_number);
		return -1;
	}
	printk(KERN_INFO "jprobe is at %p\n", my_jprobe.kp.addr);

    return 0;
}

static void __exit char_cleanup(void) {
	//Deregister the char device and jprobe
	printk(KERN_INFO "Cleaning up module.\n");
	remove_proc_entry(DEVICE_NAME,NULL);
	unregister_jprobe(&my_jprobe);

	//Free memory allocated to kernel_buffer
	for(i=0;i<N;i++) {
		if(kernel_buffer[i])
			kfree(kernel_buffer[i]);
	}
}

static int j_handle_mm_fault(struct vm_area_struct *vma, unsigned long address,	unsigned int flags) {
	//Print the addresses on which given process faults
	if(process_id == current->pid) {
		printk(KERN_INFO "PID: %d, Address: %lx",process_id,address);

		//Get time
		getnstimeofday(&ts);

		//Circular buffer for storing address and time
		kernel_buffer[index%N] = kmalloc(sizeof(char)*100,GFP_KERNEL);
		sprintf(kernel_buffer[index%N], "Address:%lx Time in nano sec:%li", address, ts.tv_nsec);
		index++;
	}
	jprobe_return();
	return 0;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t length, loff_t *offset) {
	//Return address and time in nanoseconds to user space
	ret_number = copy_to_user(buffer, kernel_buffer[read_index], length);
	if(ret_number != 0) {
		printk(KERN_INFO "\nError in copying to user..\n");
		return -EFAULT;
	}
	read_index++;
	offset+=length;
	return length;
}

module_init(char_init);
module_exit(char_cleanup);
