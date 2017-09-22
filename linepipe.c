#include<linux/miscdevice.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/errno.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include<linux/sched.h>
#include<linux/types.h>


MODULE_DESCRIPTION("Misc Character Driver for IPC");
MODULE_AUTHOR("Shubham Gupta <sgupta30@binghamton.edu>");
MODULE_LICENSE("GPL");

char **queue;
int front = -1;
int rear = -1;
int buffer_size;
//Get the module parameter from user
module_param(buffer_size,int,S_IRUGO);
static struct semaphore empty;
static struct semaphore full;
static struct semaphore mutex;
int status = 0;
int __init mydevice_init(void);
void __exit mydevice_exit(void);
//open file operation
static int mydevice_processlist_open(struct inode *inode,struct file *filep)
{	
	pr_info("File open \n");
        return 0;
}
//close file operation
static int mydevice_processlist_close(struct inode *inodep,struct file *filep)
{
	pr_info("File close \n");
        return 0;
}
//read file operation
static ssize_t mydevice_processlist_read(struct file *filep, char *buffer, size_t len, loff_t *offset) 
{
	int status;
	//down operation on full	
	if((down_interruptible(&full))==0)
	{
	//down operation on mutex
	if((down_interruptible(&mutex))==0)
	{
	front++;
	//circular queue logic
	if(front>=buffer_size)
	{
	front = 0;
	}
	status = copy_to_user(buffer,queue[front],len);
	if(status !=0)
	{
		pr_err("Failed to copy to user \n");
		return -EINVAL;
	}
	//up operation on mutex
	up(&mutex);
	}
	//up operation on empty
	up(&empty);
	}
	return len;
}
//write file operation
static ssize_t mydevice_processlist_write(struct file *filep,const char *buffer, size_t len, loff_t *offset) 
{
	int ret;
	//down operation on empty	
	if((down_interruptible(&empty))==0)
	{
	//down operation on mutex
	if((down_interruptible(&mutex))==0)
	{
	rear++;
	//circular queue logic
	if(rear>=buffer_size)
	{
	rear = 0;
	}
	ret = copy_from_user(queue[rear],buffer,len);
	if(ret !=0)
	{
		pr_err("Failed to copy from user \n");
		return -EINVAL;
	}
	//up operation on mutex	
	up(&mutex);
	}
	//up operation on full
	up(&full);
	}	
	return len;
}
//file operation initialization for module
static const struct file_operations mydevice_processlist_fops = {
        .owner = THIS_MODULE,
        .open = mydevice_processlist_open,
        .release = mydevice_processlist_close,
	.read = mydevice_processlist_read,
	.write = mydevice_processlist_write,
};
//device driver registration
static struct miscdevice mydevice_processlist_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "linepipe",
        .fops = &mydevice_processlist_fops,
};
//init module for device driver
int __init mydevice_init(void)
{
	int value;
	int i=0;
	value = misc_register(&mydevice_processlist_device);
        if(value)
        {
                pr_err("Device can't be register: \n");
                return value;
        }
	//initialization of semaphore
	sema_init(&full, 0);
	sema_init(&empty, buffer_size);
	sema_init(&mutex, 1);
        front = 0;
	rear = 0;
	//2-D char array initialization
	queue = (char **)kmalloc(sizeof(char *) * buffer_size, GFP_KERNEL);
	
	for(i=0;i<buffer_size;i++)
	{
	  queue[i] = (char *)kmalloc(sizeof(char) * 100, GFP_KERNEL);
	
	   memset(queue[i],0,sizeof(char)*buffer_size*100);
      	
	}
	
	if(!queue){
		value = -ENOMEM;
		goto fail;
	}
		
	pr_info("Device register \n");
        
	return 0;
	fail:
	     mydevice_exit();
	     return value;

}
//exit module for device driver
void __exit mydevice_exit(void)
{
	misc_deregister(&mydevice_processlist_device);
	if(queue){
	int j=0;	
	for(j=0;j<buffer_size;j++)
	{
	   kfree(queue[j]);
	}
	kfree(queue);
	}
	pr_info("Device unregister");
}
module_init(mydevice_init);
module_exit(mydevice_exit);
