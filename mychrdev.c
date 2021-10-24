#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>    // -> kmalloc
#include <linux/uaccess.h>
//#include <string.h>

#define DEV_COUNT 2
#define DEV_SIZE 256

MODULE_LICENSE("GPL");

static int mychardev_open(struct inode *inode, struct file *file);
static int mychardev_release(struct inode *inode, struct file *file);
static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static loff_t mychardev_llseek(struct file *file, loff_t ppos, int whence);

static const struct file_operations mychardev_fops = {
    .owner      = THIS_MODULE,
    .llseek     = mychardev_llseek,
    .open       = mychardev_open,
    .release    = mychardev_release,
    .read       = mychardev_read,
    .write      = mychardev_write
};

struct mychar_device_data {
    char *data;
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[DEV_COUNT];

static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init mychardev_init(void)
{
    int err, i;
    dev_t dev;

    err = alloc_chrdev_region(&dev, 0, DEV_COUNT, "mychardev");

    dev_major = MAJOR(dev);

    mychardev_class = class_create(THIS_MODULE, "mychardev");
    mychardev_class->dev_uevent = mychardev_uevent;

    for (i = 0; i < DEV_COUNT; i++) {
        cdev_init(&mychardev_data[i].cdev, &mychardev_fops);
        mychardev_data[i].cdev.owner = THIS_MODULE;

        err = cdev_add(&mychardev_data[i].cdev, MKDEV(dev_major, i), 1);
        mychardev_data[i].data = kmalloc(DEV_SIZE, GFP_KERNEL);
        if (!mychardev_data[i].data) {
            printk(KERN_ALERT "mem_dev_p[i].data malloc fail!");
            //goto fail_data_malloc;
        }

        memset(mychardev_data[i].data, 0, DEV_SIZE);
        device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
    }

    return 0;
}

static void __exit mychardev_exit(void)
{
    int i;

    for (i = 0; i < DEV_COUNT; i++) {
        device_destroy(mychardev_class, MKDEV(dev_major, i));
        if (mychardev_data[i].data)
            kfree(mychardev_data[i].data);
    }

    class_unregister(mychardev_class);
    class_destroy(mychardev_class);
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static loff_t mychardev_llseek(struct file *file, loff_t ppos, int whence){
    if (ppos < 0 || ppos >= DEV_SIZE){
        printk(KERN_INFO "Bad lseek parametr");
        return -EFAULT;
    }
    printk("offset %d moved to ", (int)file->f_pos);
    switch (whence) {
    case 0:
        file->f_pos = ppos;
        break;
    case 1:
        file->f_pos += ppos;
        break;
    case 2:
        file->f_pos = DEV_SIZE + ppos;
        break;
    default:
        printk(KERN_ALERT "WRONG PARAMETR FOR LSEEK\n");
        return -EFAULT;
    }
    printk("%d Success!\n", (int)file->f_pos);
    return file->f_pos;
}

static int mychardev_open(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device open\n");
    int i = iminor(inode);
    file->private_data = mychardev_data[i].data;
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device close\n");
    return 0;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "writing to device %d bytes with %d offset", count,(int)*ppos);
    if (count > DEV_SIZE){
        printk(KERN_ALERT "Too big count to write\n");
        return -EFAULT;
    }
    int nbytes = copy_from_user(file->private_data + *ppos, buf, count);
    *ppos += (count - nbytes);
    if (nbytes != 0){
        printk(KERN_ALERT "Too big count to write\n");
        return -EFAULT;
    }
    return 0;
}

static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "reading from device %d bytes\n",count);
    if (count > DEV_SIZE){
        printk(KERN_ALERT "Too big count to read\n");
        return -EFAULT;
    }
    int nbytes = copy_to_user(buf, file->private_data + *ppos, count);
    if (nbytes != 0){
        printk(KERN_ALERT "Too big count to read\n");
        return -EFAULT;
    }
    *ppos += count ;
    return nbytes;
}

module_init(mychardev_init);
module_exit(mychardev_exit);
