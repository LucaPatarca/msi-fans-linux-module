#include "../include/msi_device.h"

static int major = 60;
static struct class *class = NULL;

// TODO dare un nome al dispositivo

int msi_dev_create(struct msi_dev *msi_device, struct file_operations *fo, const char *name)
{
    int err = 0;
    err = alloc_chrdev_region(&msi_device->dev, major, major, name);
    if (err < 0)
    {
        printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
        return err;
    }
    if (class == NULL)
    {
        class = class_create(THIS_MODULE, "msi_fans");
        if (IS_ERR(class))
        {
            err = PTR_ERR(class);
            printk(KERN_WARNING "[target] create_class() failed\n");
            return err;
        }
    }
    cdev_init(&msi_device->c_dev, fo);
    err = cdev_add(&msi_device->c_dev, msi_device->dev, 1);
    if (err)
    {
        printk(KERN_WARNING "[target] cdev_add() failed\n");
        return err;
    }
    msi_device->device = device_create(class, NULL, msi_device->dev, NULL, name);
    if (IS_ERR(msi_device->device))
    {
        err = PTR_ERR(msi_device->device);
        printk(KERN_WARNING "[target] Error while trying to create device");
        return err;
    }

    printk(KERN_INFO "hello: Hello world\n");
    return 0;
}

void msi_dev_destroy(struct msi_dev *dev)
{
    if (class == NULL)
        return;
    device_del(dev->device);
    device_destroy(class, dev->dev);
    cdev_del(&dev->c_dev);
    unregister_chrdev_region(dev->dev, 1);
}

void msi_class_destroy(void)
{
    if (class != NULL)
        class_destroy(class);
}
