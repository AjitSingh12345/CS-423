#define LINUX

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include "mp1_given.h"
//#include <stdio.h>
#include <linux/slab.h>
#include <asm/uaccess.h> 
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ajitps2");
MODULE_DESCRIPTION("CS-423 MP1");

#define DEBUG 			1
#define FILENAME 		"status"
#define DIRECTORY 		"mp1"
#define PROCFS_MAX_SIZE 	2048
#define TIMEOUT 		5000    //milliseconds

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;
static struct registered_processes_list my_list;

// TIMER stuff
static struct timer_list my_timer;

// WORKQUEUE stuff
struct workqueue_struct *my_workqueue;
struct work_data {
    struct work_struct work;
    int data;
};

static void work_handler(struct work_struct *work) {
	struct work_data * data = (struct work_data *)work;
	printk(KERN_INFO "work handler exec'd \n");
	kfree(data);
}

//Timer Callback function. This will be called when timer expires
void timer_callback(struct timer_list *timer) {	
	printk(KERN_ALERT "This line is printed after 5 seconds.\n");
	
	struct work_data *my_work = kmalloc(sizeof(struct work_data), GFP_KERNEL);
	INIT_WORK(&my_work->work, work_handler);
	if (queue_work(my_workqueue, &my_work->work)) {
		printk(KERN_INFO "error submitting to work_queue \n");
	}

	// set timer interval to 5 sec 
	printk(KERN_ALERT "Re-enabling timer \n");	
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMEOUT));

}

struct registered_processes_list {
	int pid;
	unsigned long cpu_time;
	struct list_head list;
};


// read
static ssize_t mp1_read(struct file *file, char __user *buffer, size_t count, loff_t *data) {
   static int finished = 0;
   
   if (finished) {
	printk(KERN_INFO "procfs_read: END\n");
	finished = 0;
	return 0;
   }

   finished = 1;
   
   // EDIT (look into seq files)
   // format: "PID1: CPU Time of PID1\n"
   struct list_head *pos, *q;
   struct registered_processes_list* tmp;
   int j = 0;

   list_for_each(pos, &my_list.list) {
        tmp = list_entry(pos, struct registered_processes_list, list);
        printk(KERN_INFO "read list entry: pid = %d, cpu_time = %lu, j: %d \n", tmp->pid, tmp->cpu_time, j);
        j += sprintf(procfs_buffer+j, "%d: %lu\n", tmp->pid, tmp->cpu_time);
   }

   if (copy_to_user(buffer, procfs_buffer, sizeof(procfs_buffer))) {
        return -EFAULT;
   }

   return count;
}

// write
static ssize_t mp1_write(struct file *file, const char __user *buffer, size_t count, loff_t *data) {
   printk(KERN_INFO "procfs_write: write %lu bytes\n", count);
   
   // EDIT -> no matter what user writed, we just regiter their PID unless its alr registered
   // ** IMPORTANT -> PID is written from user app so we have to do get_from_user
   if (count > PROCFS_MAX_SIZE) {
	procfs_buffer_size = PROCFS_MAX_SIZE;  
   } else {
  	procfs_buffer_size = count;
   }

   if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
 	return -EFAULT;
   }

   int _pid;
   sscanf(procfs_buffer, "%d", &_pid);
   
   printk(KERN_INFO "input pid from user: %d \n", _pid);
  
   // check if this pid is alr in registered_processes_list
   struct list_head *pos, *q;
   struct registered_processes_list* tmp;

   list_for_each(pos, &my_list.list) {
	tmp = list_entry(pos, struct registered_processes_list, list);
	printk(KERN_INFO "write list entry: pid = %d, cpu_time = %lu \n", tmp->pid, tmp->cpu_time);
	if (tmp->pid == _pid) {
		return procfs_buffer_size;
	}
   }
   
   // if not, kmalloc a new node and add this new entry using get_cpu_time from given.c
   tmp = (struct registered_processes_list*)kmalloc(sizeof(struct registered_processes_list), GFP_KERNEL);
   tmp->pid = _pid; 
   tmp->cpu_time = -1;
   if (get_cpu_use(tmp->pid, &tmp->cpu_time)) {
	printk(KERN_INFO "process %d doesnt exist to get cpu time \n", tmp->pid);
   } // DO ERROR CHECKING??
   list_add(&(tmp->list), &(my_list.list));  
   printk(KERN_INFO "Added to list: process %d: cpu time %lu \n", tmp->pid, tmp->cpu_time);

   return procfs_buffer_size;
}

static const struct file_operations mp1_file = {
   .owner = THIS_MODULE,
   .read = mp1_read,
   .write = mp1_write,
};

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE LOADING\n");
   #endif

   // Insert your code here ...

   // init timer
   timer_setup(&my_timer, timer_callback, 0);

   // set timer interval to 5 sec 
   mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMEOUT));

   // create direc and entry within proc filesys
   proc_dir = proc_mkdir(DIRECTORY, NULL);
   if (!proc_dir) {
        printk(KERN_ALERT "Error: Could not initialize directory /proc/%s\n", DIRECTORY);
        return -ENOMEM;
   } 

   proc_entry = proc_create(FILENAME, 0666, proc_dir, &mp1_file);
   if (!proc_entry) {
	printk(KERN_ALERT "Error: Could not initialize entry /proc/%s\n", FILENAME);
   	return -ENOMEM;
   }

   // init list
   INIT_LIST_HEAD(&my_list.list);     

   // init workqueue
   my_workqueue = create_workqueue("my_workqueue");

   printk(KERN_ALERT "MP1 MODULE LOADED\n");
   return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp1_exit(void)
{
   #ifdef DEBUG
   printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
   #endif

   // Insert your code here ...   
   
   // EDIT -> kfree all LL nodes, stop pending work func, destory workqueue


   // free all LL nodes
   printk("deleting the list using list_for_each_safe()\n");
   struct list_head *pos, *q;
   struct registered_processes_list* tmp;
   list_for_each_safe(pos, q, &my_list.list) {
   	tmp= list_entry(pos, struct registered_processes_list, list);
	printk("freeing item pid= %d cpu_time= %lu\n", tmp->pid, tmp->cpu_time);
	list_del(pos);
	kfree(tmp);
   }

   remove_proc_entry(FILENAME, proc_dir);
   remove_proc_entry(DIRECTORY, NULL);

   // destory kernel timer
   del_timer(&my_timer);

   // flush & destory workqueue -- CANCEL REMAINING WORK??
   flush_workqueue(my_workqueue);
   destroy_workqueue(my_workqueue);

   printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);
