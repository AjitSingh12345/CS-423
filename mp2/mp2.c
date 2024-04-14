#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */

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

static ssize_t mp2_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;
	
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", FILENAME);
	
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		/* fill the buffer, return the buffer size */
		memcpy(buffer, procfs_buffer, procfs_buffer_size);
		ret = procfs_buffer_size;
	}

	return ret;
}

// proc file is written to
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

static const struct file_operations mp1_file = 
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
}

// exit -- called when module is unloaded
void __exit mp2_exit(void)
{
	#ifdef DEBUG
        printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
        #endif
}

module_init(mp2_init);
module_exit(mp2_exit);

