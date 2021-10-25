#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>    // -> kmalloc
#include <linux/uaccess.h>

#define DEV_COUNT 2
#define DEV_SIZE 256

MODULE_LICENSE("GPL");

static int chardev_open(struct inode *inode, struct file *file);
static int chardev_release(struct inode *inode, struct file *file);
static ssize_t chardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t chardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static loff_t chardev_llseek(struct file *file, loff_t ppos, int whence);

static const struct file_operations chardev_fops = {
    .owner      = THIS_MODULE,
    .llseek     = chardev_llseek,
    .open       = chardev_open,
    .release    = chardev_release,
    .read       = chardev_read,
    .write      = chardev_write
};

struct char_device_data {
    char *data;
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *chardev_class = NULL;
static struct char_device_data chardev_data[DEV_COUNT];

// for create device with wright and read opportunity, no need in use chmod
static int chardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init chardev_init(void)
{
    int error, i;
    dev_t dev;

    error = alloc_chrdev_region(&dev, 0, DEV_COUNT, "mychardev");
    if (error) {
        printk(KERN_ALERT "Can't assign major and minor numbers\n");
        return -EFAULT;
    }

    dev_major = MAJOR(dev);

    chardev_class = class_create(THIS_MODULE, "mychardev");
    chardev_class->dev_uevent = chardev_uevent;

    for (i = 0; i < DEV_COUNT; i++) {
        cdev_init(&chardev_data[i].cdev, &chardev_fops);
        chardev_data[i].cdev.owner = THIS_MODULE;

        error = cdev_add(&chardev_data[i].cdev, MKDEV(dev_major, i), 1);
        if (error){
            printk(KERN_ALERT "Can't add char device %d\n",i);
            return -EFAULT; 
        }
        chardev_data[i].data = kmalloc(DEV_SIZE, GFP_KERNEL);
        if (!chardev_data[i].data) {
            printk(KERN_ALERT "Malloc fail for device %d\n!",i);
            return -EFAULT;
        }

        memset(chardev_data[i].data, 0, DEV_SIZE);
        device_create(chardev_class, NULL, MKDEV(dev_major, i), NULL, "my_cdev%d", i);
    }
    return 0;
}

static void __exit chardev_exit(void)
{
    int i;

    for (i = 0; i < DEV_COUNT; i++) {
        device_destroy(chardev_class, MKDEV(dev_major, i));
        if (chardev_data[i].data)
            kfree(chardev_data[i].data);
    }

    class_unregister(chardev_class);
    class_destroy(chardev_class);
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static loff_t chardev_llseek(struct file *file, loff_t ppos, int whence){
    if (ppos < 0 || ppos >= DEV_SIZE){
        printk(KERN_INFO "Bad lseek parametr");
        return -EFAULT;
    }
    printk("offset %d moved to: ", (int)file->f_pos);
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
    printk("%d position. Success!\n", (int)file->f_pos);
    return file->f_pos;
}

static int chardev_open(struct inode *inode, struct file *file)
{
    int i = iminor(inode);
    printk("MY_CDEV%d: Device open\n", i);
    file->private_data = chardev_data[i].data;
    return 0;
}

static int chardev_release(struct inode *inode, struct file *file)
{
    int i = iminor(inode);
    printk("MY_CDEV%d: Device close\n",i);
    return 0;
}

static ssize_t chardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int minor = MINOR(file->f_path.dentry->d_inode->i_rdev);
    printk(KERN_INFO "Writing to device my_cdev%d, offset - %llu....\n", minor, (long long unsigned)*ppos);
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
    printk(KERN_INFO "Wrote %d bytes, current offset - %llu\n", count,(long long unsigned)*ppos);
    return 0;
}

static ssize_t chardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int minor = MINOR(file->f_path.dentry->d_inode->i_rdev);
    printk(KERN_INFO "Reading from device my_cdev%d, offset - %llu....\n", minor, (long long unsigned)*ppos);
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
    printk(KERN_INFO "Read %d bytes, current offset - %llu\n", count,(long long unsigned)*ppos);
    return nbytes;
}

module_init(chardev_init);
module_exit(chardev_exit);
