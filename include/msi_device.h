#ifndef MSI_FANS_MSI_DEVICE_H
#define MSI_FANS_MSI_DEVICE_H

#include <linux/fs.h>
#include <linux/cdev.h>

#define MPC_MAX_USER_SIZE           1024

struct msi_dev {
    struct cdev c_dev;
    dev_t dev;
    struct device *device;
};

int msi_dev_create(struct msi_dev *dev, struct file_operations *fo, const char *name);
void msi_dev_destroy(struct msi_dev *dev);
void msi_class_destroy(void);

#endif