#define LINUX

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include "mp2_given.h"
//#include <stdio.h>
#include <linux/slab.h>
#include <asm/uaccess.h> 
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ajitps2");
MODULE_DESCRIPTION("CS-423 MP2");

#define DEBUG 			1
#define FILENAME 		"status"
#define DIRECTORY 		"mp2"
#define PROCFS_MAX_SIZE 	2048
#define TIMEOUT 		5000    //milliseconds

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static ssize_t mp2_read(struct file *file, char __user *buffer, size_t count, loff_t *offp)
{
	ssize_t bytes_read;
	char *buf;
	unsigned long flags;

	buf = (char*)kmalloc(count, GFP_KERNEL);

	// critical section
	// spin_lock_irqsave(&mp2_lock, flags);
	return count;	
}
	
// for when file is written to
static ssize_t mp2_write(struct file *file, const char __user *buffer, size_t count, loff_t *data) 
{
	printk(KERN_INFO "procfs_write: write %lu bytes\n", count);

	if (count > PROCFS_MAX_SIZE) {
		procfs_buffer_size = PROCFS_MAX_SIZE;  
	} else {
  		procfs_buffer_size = count;
   	}

   	if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
 		return -EFAULT;
   	}

	return procfs_buffer_size;
}

static const struct file_operations mp2_file = 
{
	.owner = THIS_MODULE,
	.read = mp2_read,
	.write = mp2_write,
};

// init -- called when module is loaded
int __init mp2_init(void)
{
	#ifdef DEBUG
	printk(KERN_ALERT "MP2 MODULE LOADING\n");
   	#endif

	// create direc and entry within proc filesys
  	 proc_dir = proc_mkdir(DIRECTORY, NULL);
   	if (!proc_dir) {
        	printk(KERN_ALERT "Error: Could not initialize directory /proc/%s\n", DIRECTORY);
		return -ENOMEM;
	} 

   	proc_entry = proc_create(FILENAME, 0666, proc_dir, &mp2_file);
  	if (!proc_entry) {
		printk(KERN_ALERT "Error: Could not initialize entry /proc/%s\n", FILENAME);
   		return -ENOMEM;
	}

	printk(KERN_ALERT "MP2 MODULE LOADED\n");
  	return 0;   
}

// exit -- called when module is unloaded
void __exit mp2_exit(void)
{
	#ifdef DEBUG
        printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
        #endif

	remove_proc_entry(FILENAME, proc_dir);
   	remove_proc_entry(DIRECTORY, NULL);
	
	printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
}

module_init(mp2_init);
module_exit(mp2_exit);

