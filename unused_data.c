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
//static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
//static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);
ssize_t char_device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos);

static const struct file_operations mychardev_fops = {
    .owner      = THIS_MODULE,
    .open       = mychardev_open,
    .release    = mychardev_release,
    .read = char_device_read,
    .write = char_device_write
    // .read       = mychardev_read,
    // .write       = mychardev_write
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

ssize_t char_device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_read : Bad parameters!");
        return -ENODEV;
    }
    printk(KERN_INFO "READ count %d offset %d", size, (int)*ppos);
    char *p_dev = filp->private_data;
    if (size > DEV_SIZE) 
        printk(KERN_ALERT "COUNT = %d", size);
        goto out;
    if (*ppos + size > DEV_SIZE)
        size = DEV_SIZE- *ppos;

    if (copy_to_user(buf, p_dev + *ppos, size)) {
        goto out;
    } else {
        *ppos += size;
        return size;
    }

out:
    return -EFAULT;
}

ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_write : Bad parameters!");
        return -ENODEV;
    }
    printk(KERN_INFO "WRITE count %d offset %d", size, (int)*ppos);
    // check buffer size
    char *p_dev = filp->private_data;
    if (size > DEV_SIZE)
        goto out;
    if (*ppos + size > DEV_SIZE)
        size = DEV_SIZE - *ppos;

    // Now copy data to kernel space
    if (copy_from_user(p_dev + *ppos, buf, size)) {
        goto out;
    } else {
        *ppos + size;
        return size;
    }

out:
    return -EFAULT;
}

// static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
// {
// //     if (!file || !buf || !ppos) {
// //         printk(KERN_ALERT "char_device_read : Bad parameters!");
// //         return -ENODEV;
// //     }

// //     char* p_dev = filp->private_data;
// //     if (count > DEV_SIZE) 
// //         goto out;
// //     if (*ppos + count > DEV_SIZE)
// //         count = DEV_SIZE - *ppos;

// //     if (copy_to_user(buf, p_dev + *ppos, count)) {
// //         goto out;
// //     } else {
// //         *ppos += co;
// //         return size;
// //     }

// // out:
// //     return -EFAULT;
//     printk(KERN_INFO "WRITE");
//     char* data = file->private_data;
//     copy_from_user(data + *ppos, buf, count);
//     printk(KERN_INFO "char_device_write : WRITTEN %d!",count);
//     //*ppos += count;
//     return count;
//     // if (!file || !buf || !ppos) {
//     //     printk(KERN_ALERT "char_device_write : Bad parameters!");
//     //     return -ENODEV;
//     // }

//     // // check buffer size
//     // char* data = file->private_data;
//     // if (count > DEV_SIZE)
//     //     printk(KERN_ALERT "TRY TO WRITE MORE THAN FILE SIZE!");
//     //     return -EFAULT;
//     // if (*ppos + count > DEV_SIZE)
//     //     count = DEV_SIZE - *ppos;
//     // // Now copy data to kernel space
//     // if (copy_from_user(data + *ppos, buf, count)) {
//     //     printk(KERN_INFO "COPY ERR\n");
//     //     return -EFAULT;
//     // } else {
//     //     printk(KERN_INFO "char_device_write : WRITTEN %d!",count);
//     //     *ppos += count;
//     //     return count;
//     // }
// }

// static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
// {
//     printk(KERN_INFO "READ count %d offset %d", count, (int)*ppos);
//     count = 5;
//     char* data = file->private_data;
//     copy_from_user(buf, data + *ppos, count);
//     printk(KERN_INFO "char_device_write : WRITTEN %d!",count);
//     *ppos += count;
//     return count;
// //     if (!file || !buf || !ppos) {
// //         printk(KERN_ALERT "char_device_read : Bad parameters!");
// //         return -ENODEV;
// //     }

// //     char* p_dev = file->private_data;
// //     if (count > DEV_SIZE) 
// //         goto out;
// //     if (*ppos + count > DEV_SIZE)
// //         count = DEV_SIZE - *ppos;

// //     if (copy_to_user(buf, p_dev + *ppos, count)) {
// //         goto out;
// //     } else {
// //         *ppos += count;
// //         return count;
// //     }

// // out:
// //     return -EFAULT;
//     // printk(KERN_ALERT "READ");
//     // char* data = file->private_data;
//     // printk(KERN_INFO "offset %d: count %d!",*ppos,count);
//     // copy_to_user(buf, data + *ppos, count);
//     // printk(KERN_INFO "char_device_read : READ %d!",count);
//     // *ppos += count;
//     // return count;
    
//     // printk(KERN_ALERT "READ");
//     // if (!file || !buf || !ppos) {
//     //     printk(KERN_ALERT "char_device_read : Bad parameters!");
//     //     return -ENODEV;
//     // }
//     // char* data = file->private_data;
//     // if (count > DEV_SIZE) 
//     //     printk(KERN_ALERT "TRY TO READ MORE THAN FILE SIZE!");
//     //     return -EFAULT;
//     // if (*ppos + count > DEV_SIZE)
//     //     count = DEV_SIZE - *ppos;
//     // if (copy_to_user(buf, data + *ppos, count)) {
//     //     return -EFAULT;
//     // } else {
//     //     printk(KERN_INFO "char_device_write : READ %d!",count);
//     //     *ppos += count;
//     //     return count;
//     // }
// }

module_init(mychardev_init);
module_exit(mychardev_exit);














#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>    // -> kmalloc
#include <asm/uaccess.h>

#include "char_device.h"

MODULE_LICENSE("Dual BSD/GPL");

// The device
static struct cdev char_device;

// Memory device - The device manipulated by file_operations
static struct mem_dev *mem_dev_p;

// open this device
int char_device_open(struct inode *inode, struct file *filp) {
    unsigned int minor = iminor(inode);

    if (minor >= MAX_MINOR_NUM) {
        printk(KERN_ALERT "Bad minor number!");
        return -ENODEV;
    }

    struct mem_dev *p = &mem_dev_p[minor];

    // let filp->private_data points to mem_dev
    filp->private_data = p;

    return 0;
}

int char_device_release(struct inode *inode, struct file *filp) {
    return 0;
}

ssize_t char_device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_read : Bad parameters!");
        return -ENODEV;
    }

    // check buffer size
    struct mem_dev *p_dev = filp->private_data;
    if (size > p_dev->size) 
        goto out;
    if (*ppos + size > p_dev->size)
        size = p_dev->size - *ppos;

    // Now copy data to user space
    if (copy_to_user(buf, p_dev->data + *ppos, size)) {
        goto out;
    } else {
        *ppos += size;
        return size;
    }

out:
    return -EFAULT;
}

ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_write : Bad parameters!");
        return -ENODEV;
    }

    // check buffer size
    struct mem_dev *p_dev = filp->private_data;
    if (size > p_dev->size)#include <stdio.h>
#include <stdlib.h>#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>

/* CONSTANTS */
#define DEVICE_FILE "/dev/s33drax"
#define LENGTH      1024
#define READ        'r'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE

/******************************************************************************
 Allows the user to enter an option on the keyboard.
******************************************************************************/
char get_option() 
{
    char option;
    
    do {
        system("clear");
        printf("DEVICE OPTIONS:\n\n- Read(%c)\n- Write(%c)\n- Quit(%c)\n", 
                READ, WRITE, QUIT);
        option = tolower(getchar());
        __fpurge(stdin);
    } while (option != READ && option != WRITE && option != QUIT);

    return option;
}

/******************************************************************************
 Entry point of the process.
******************************************************************************/
int main()
{
    int fd, done = FALSE;
    char write_buf[LENGTH], read_buf[LENGTH], command[LENGTH/8];

    /* Give permission to use the device file */
    sprintf(command, "sudo chmod 777 %s", DEVICE_FILE);
    system(command);

    assert(fd = open(DEVICE_FILE, O_RDWR));
    
    while (!done) {
        switch(get_option()) {
        case READ:
            read(fd, read_buf, LENGTH);
            printf("device: %s\n", read_buf);
            getchar();
            break;
        case WRITE:
            printf("%s", "Enter data: ");
            scanf("%1023[^\n]s", write_buf);
            __fpurge(stdin);
            write(fd, write_buf, LENGTH);
            break;
        case QUIT:
            done = TRUE;
            break;
        }
    }
    close(fd);
    return 0;
}
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>

/* CONSTANTS */
#define DEVICE_FILE "/dev/s33drax"
#define LENGTH      1024
#define READ        'r'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE

/******************************************************************************
 Allows the user to enter an option on the keyboard.
******************************************************************************/
char get_option() 
{
    char option;
    
    do {
        system("clear");
        printf("DEVICE OPTIONS:\n\n- Read(%c)\n- Write(%c)\n- Quit(%c)\n", 
                READ, WRITE, QUIT);
        option = tolower(getchar());
        __fpurge(stdin);
    } while (option != READ && option != WRITE && option != QUIT);

    return option;
}

/******************************************************************************
 Entry point of the process.
******************************************************************************/
int main()
{
    int fd, done = FALSE;
    char write_buf[LENGTH], read_buf[LENGTH], command[LENGTH/8];

    /* Give permission to use the device file */
    sprintf(command, "sudo chmod 777 %s", DEVICE_FILE);
    system(command);

    assert(fd = open(DEVICE_FILE, O_RDWR));
    
    while (!done) {
        switch(get_option()) {
        case READ:
            read(fd, read_buf, LENGTH);
            printf("device: %s\n", read_buf);
            getchar();
            break;
        case WRITE:
            printf("%s", "Enter data: ");
            scanf("%1023[^\n]s", write_buf);
            __fpurge(stdin);
            write(fd, write_buf, LENGTH);
            break;
        case QUIT:
            done = TRUE;
            break;
        }
    }
    close(fd);
    return 0;
}
        goto out;
    if (*ppos + size > p_dev->size)
        size = p_dev->size - *ppos;

    // Now copy data to kernel space
    if (copy_from_user(p_dev->data + *ppos, buf, size)) {
        goto out;
    } else {
        *ppos + size;
        return size;
    }

out:
    return -EFAULT;
}

// File operator - It set up the connection between the dirver's operators
// and device number
static const struct file_operations char_device_ops = {
    .owner = THIS_MODULE,
    //.llseek = char_device_llseek,
    .read = char_device_read, 
    .write = char_device_write, 
    .open = char_device_open, 
    .release = char_device_release, 
};



static int char_device_init(void) {
    printk(KERN_ALERT "Init!");
    dev_t device_num = MKDEV(MAJOR_DEVICE_NUM, 0);
    printk(KERN_ALERT "Device number is %d\n", device_num);
    printk(KERN_ALERT "Major number is %d\n", MAJOR(device_num));
    printk(KERN_ALERT "Minor number is %d\n", MINOR(device_num));

    // Allocate/Register device number, now it appreas in /proc/device
    if (register_chrdev_region(device_num, MAX_MINOR_NUM, "char_device")) {
        printk(KERN_ALERT "Failed to allocate device number");
        goto register_fail;
    }

    // Init a deivce, establish the connection between device and operations
    cdev_init(&char_device, &char_device_ops);

    // Let kernel know the existence of this device
    int err = cdev_add(&char_device, device_num, MAX_MINOR_NUM);
    if (err) {
        printk(KERN_ALERT "cdev_add fail!");
        goto fail_malloc;
    }

    // Allocate memory for device
    mem_dev_p = kmalloc(sizeof(struct mem_dev) * MEMDEV_NUM, GFP_KERNEL);
    if (!mem_dev_p) {
        printk(KERN_ALERT "mem_dev_p malloc fail!");
        goto fail_malloc;
    }

    int i = 0;
    for (; i < MEMDEV_NUM; ++i) {
        mem_dev_p[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
        if (!mem_dev_p[i].data) {
            printk(KERN_ALERT "mem_dev_p[i].data malloc fail!");
            goto fail_data_malloc;
        }

        mem_dev_p[i].size = MEMDEV_SIZE;
        memset(mem_dev_p[i].data, 0, mem_dev_p[i].size);
    }

    return 0;

fail_data_malloc:
    kfree(mem_dev_p);
fail_malloc:
    unregister_chrdev_region(MKDEV(MAJOR_DEVICE_NUM, 0), MAX_MINOR_NUM);
register_fail:
    return 0;
}

static void char_device_exit(void) {
    printk(KERN_ALERT "Goodbye!");

    // free memory device
    int i = 0;
    for (; i < MEMDEV_NUM; ++i) {
        if (mem_dev_p && mem_dev_p[i].data)
            kfree(mem_dev_p[i].data);
    }

    if (mem_dev_p)
        kfree(mem_dev_p);

    cdev_del(&char_device);
    unregister_chrdev_region(MKDEV(MAJOR_DEVICE_NUM, 0), MAX_MINOR_NUM);
}

module_init(char_device_init);
module_exit(char_device_exit);


//     if (!file || !buf || !ppos) {
//         printk(KERN_ALERT "char_device_read : Bad parameters!");
//         return -ENODEV;
//     }

//     char* p_dev = file->private_data;
//     if (count > DEV_SIZE) 
//         goto out;
//     if (*ppos + count > DEV_SIZE)
//         count = DEV_SIZE - *ppos;

//     if (copy_to_user(buf, p_dev + *ppos, count)) {
//         goto out;
//     } else {
//         *ppos += count;
//         return count;
//     }

// out:
//     return -EFAULT;
    // printk(KERN_ALERT "READ");
    // char* data = file->private_data;
    // printk(KERN_INFO "offset %d: count %d!",*ppos,count);
    // copy_to_user(buf, data + *ppos, count);
    // printk(KERN_INFO "char_device_read : READ %d!",count);
    // *ppos += count;
    // return count;
    
    // printk(KERN_ALERT "READ");
    // if (!file || !buf || !ppos) {
    //     printk(KERN_ALERT "char_device_read : Bad parameters!");
    //     return -ENODEV;
    // }
    // char* data = file->private_data;
    // if (count > DEV_SIZE) 
    //     printk(KERN_ALERT "TRY TO READ MORE THAN FILE SIZE!");
    //     return -EFAULT;
    // if (*ppos + count > DEV_SIZE)
    //     count = DEV_SIZE - *ppos;
    // if (copy_to_user(buf, data + *ppos, count)) {
    //     return -EFAULT;
    // } else {
    //     printk(KERN_INFO "char_device_write : READ %d!",count);
    //     *ppos += count;
    //     return count;
    // }

    / // out:
// //     return -EFAULT;
//     printk(KERN_INFO "WRITE");
//     char* data = file->private_data;
//     copy_from_user(data + *ppos, buf, count);
//     printk(KERN_INFO "char_device_write : WRITTEN %d!",count);
//     //*ppos += count;
//     return count;
//     // if (!file || !buf || !ppos) {
//     //     printk(KERN_ALERT "char_device_write : Bad parameters!");
//     //     return -ENODEV;
//     // }

//     // // check buffer size
//     // char* data = file->private_data;
//     // if (count > DEV_SIZE)
//     //     printk(KERN_ALERT "TRY TO WRITE MORE THAN FILE SIZE!");
//     //     return -EFAULT;
//     // if (*ppos + count > DEV_SIZE)
//     //     count = DEV_SIZE - *ppos;
//     // // Now copy data to kernel space
//     // if (copy_from_user(data + *ppos, buf, count)) {
//     //     printk(KERN_INFO "COPY ERR\n");
//     //     return -EFAULT;
//     // } else {
//     //     printk(KERN_INFO "char_device_write : WRITTEN %d!",count);
//     //     *ppos += count;
//     //     return count;
//     // }
// }


    // ssize_t char_device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
//     if (!filp || !buf || !ppos) {
//         printk(KERN_ALERT "char_device_read : Bad parameters!");
//         return -ENODEV;
//     }
//     printk(KERN_INFO "READ count %d offset %d", size, (int)*ppos);
//     char *p_dev = filp->private_data;
//     if (size > DEV_SIZE) 
//         printk(KERN_ALERT "COUNT = %d", size);
//         goto out;
//     if (*ppos + size > DEV_SIZE)
//         size = DEV_SIZE- *ppos;

//     if (copy_to_user(buf, p_dev + *ppos, size)) {
//         goto out;
//     } else {
//         *ppos += size;
//         return size;
//     }

// out:
//     return -EFAULT;
// }

// ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
//     if (!filp || !buf || !ppos) {
//         printk(KERN_ALERT "char_device_write : Bad parameters!");
//         return -ENODEV;
//     }
//     printk(KERN_INFO "WRITE count %d offset %d", size, (int)*ppos);
//     // check buffer size
//     char *p_dev = filp->private_data;
//     if (size > DEV_SIZE)
//         goto out;
//     if (*ppos + size > DEV_SIZE)
//         size = DEV_SIZE - *ppos;

//     // Now copy data to kernel space
//     if (copy_from_user(p_dev + *ppos, buf, size)) {
//         goto out;
//     } else {
//         *ppos + size;
//         return size;
//     }

// out:
//     return -EFAULT;
// }