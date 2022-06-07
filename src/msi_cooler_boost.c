#include "../include/msi_cooler_boost.h";
#include "../include/msi_device.h"

#include <linux/acpi.h>


#define COOLER_BOOST_ADDR           0x98
#define COOLER_BOOST_ON             0x80
#define COOLER_BOOST_OFF            0x00

static char data_buffer[MPC_MAX_USER_SIZE];

static struct msi_dev cb_dev;

ssize_t msi_cooler_boost_read(struct file *file, char *buf, size_t size, loff_t *f_pos){
    char result[3];
    if(*f_pos >= sizeof(result)){
        return 0;
    }
    u8 val = 0;
    int res = ec_read(COOLER_BOOST_ADDR, &val);
    if(res < 0){
        printk("hello: error reading");
    }
    u8 boost = val>=COOLER_BOOST_ON;
    sprintf(result, "%d\n", boost);
    if(copy_to_user(buf, result, sizeof(result))){
        printk(KERN_INFO "Not all data could be copied from kernel to user\n");
        return 0;
    }
    *f_pos+=sizeof(result);
    return sizeof(result);
}

ssize_t msi_cooler_boost_write(struct file *file, const char *buf, size_t size, loff_t *f_pos){
    memset(data_buffer, 0, sizeof(data_buffer));
    if(size > MPC_MAX_USER_SIZE){
        size = MPC_MAX_USER_SIZE;
    }

    copy_from_user(data_buffer, buf, size);

    int value;
    if(kstrtoint(data_buffer, 10, &value) != 0){
        printk("hello: Unknown operation: %s", data_buffer);
        return size;
    }

    if(value == 1){
        ec_write(COOLER_BOOST_ADDR, COOLER_BOOST_ON);
    } else if(value == 0){
        ec_write(COOLER_BOOST_ADDR, COOLER_BOOST_OFF);
    } else{
        printk("hello: Unknown operation: %s", data_buffer);
    }

    return size;
}

int msi_cooler_boost_open(struct inode *inode, struct file *filp){
    return 0;
}

int msi_cooler_boost_release(struct inode *inode, struct file *filp){
    return 0;
}

static const struct file_operations cb_fops = 
{
    read: msi_cooler_boost_read,
    write: msi_cooler_boost_write,
    open: msi_cooler_boost_open,
    release: msi_cooler_boost_release,
};

int msi_cooler_boost_create(void){
    int err = 0;
    err = msi_dev_create(&cb_dev, &cb_fops, "cooler_boost");

    printk(KERN_INFO "hello: Hello world\n");
    return err;
}

void msi_cooler_boost_destroy(void){
    msi_dev_destroy(&cb_dev);
}