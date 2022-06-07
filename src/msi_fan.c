#include "../include/msi_fan.h"
#include "../include/msi_device.h"
#include <linux/string.h>
#include <linux/acpi.h>

#define CPU_TEMP_BASE_ADDRESS 0x6A
#define CPU_FAN_SPEED_BASE_ADDRESS 0x72

#define GPU_TEMP_BASE_ADDRESS 0x82
#define GPU_FAN_SPEED_BASE_ADDRESS 0x8A

static char cpu_user_buff[MPC_MAX_USER_SIZE];
static struct msi_dev cpu_fan_dev;

static char gpu_user_buff[MPC_MAX_USER_SIZE];
static struct msi_dev gpu_fan_dev;

static struct fan_config cpu_fan_config = {
    temps_low:              {47, 53, 67, 80, 90, 95, 100},
    temps_silent:           {47, 53, 67, 80, 90, 95, 100},
    temps_balanced:         {50, 58, 65, 70, 90, 95, 100},
    temps_performance:      {50, 58, 65, 70, 90, 95, 100},

    fan_speed_low:          { 0, 16, 62, 69, 76, 84,  91},
    fan_speed_silent:       {20, 33, 62, 69, 76, 84, 100},
    fan_speed_balanced:     {45, 58, 65, 72, 80, 85, 100},
    fan_speed_performance:  {55, 66, 75, 82, 90, 95, 100},
};

static struct fan_config gpu_fan_config = {
    temps_low:              {55, 65, 75, 85, 90, 93, 100},
    temps_silent:           {55, 65, 75, 85, 90, 93, 100},
    temps_balanced:         {50, 60, 70, 82, 90, 93, 100},
    temps_performance:      {50, 60, 70, 82, 90, 93, 100},
    
    fan_speed_low:          { 0, 45, 60, 70, 80, 85, 100},
    fan_speed_silent:       {20, 45, 62, 69, 76, 84, 100},
    fan_speed_balanced:     {45, 50, 65, 72, 80, 85, 100},
    fan_speed_performance:  {55, 62, 75, 82, 90, 95, 100},
};

int fan_read_from_ec(int base_temp_addr, int base_speed_addr, char *buf, size_t size){
    int i;
    for (i = 0; i < 7; i++)
    {
        u8 temp = 0;
        u8 value = 0;
        char couple_str[16];
        if (ec_read(base_temp_addr + i, &temp) < 0)
        {
            return -1;
        }
        if (ec_read(base_speed_addr + i, &value) < 0)
        {
            return -1;
        }
        if (temp > 150 || value > 150)
        {
            return -1;
        }
        sprintf(couple_str, "%d %d,", temp, value);
        strcat(buf, couple_str);
    }
    int last_char=0;
    last_char = strlen(buf) - 1;
    buf[last_char] = '\n';
    return 0;
}

int fan_write_to_ec(int base_temp_addr, int base_speed_addr, const char *profile, struct fan_config *config){
    u8 *temps = NULL;
    u8 *fan_speed = NULL;

    if (strcmp(profile, "low") == 0)
    {
        temps = config->temps_low;
        fan_speed = config->fan_speed_low;
    }
    else if (strcmp(profile, "silent") == 0)
    {
        temps = config->temps_silent;
        fan_speed = config->fan_speed_silent;
    }
    else if (strcmp(profile, "balanced") == 0)
    {
        temps = config->temps_balanced;
        fan_speed = config->fan_speed_balanced;
    }
    else if (strcmp(profile, "performance") == 0)
    {
        temps = config->temps_performance;
        fan_speed = config->fan_speed_performance;
    }
    else
    {
        printk(KERN_WARNING "Invalid command '%s'", profile);
        return -1;
    }

    int i;
    for(i=0;i<7;i++){
        ec_write(base_temp_addr + i, temps[i]);
        ec_write(base_speed_addr + i, fan_speed[i]);
    }

    return 0;
}

ssize_t mpc_cpu_fan_read(struct file *file, char *buf, size_t size, loff_t *f_pos)
{
    char result[128];

    if(*f_pos >= sizeof(result)){
        return 0;
    }

    memset(result, 0, sizeof(result));
    
    if(fan_read_from_ec(CPU_TEMP_BASE_ADDRESS, CPU_FAN_SPEED_BASE_ADDRESS, result, sizeof(result))){
        copy_to_user(buf, "ERROR", 6);
        return 6;
    }

    if(copy_to_user(buf, result, sizeof(result))){
        printk(KERN_INFO "Not all data could be copied from kernel to user\n");
        return 0;
    }
    *f_pos+=sizeof(result);

    return sizeof(result);
}

ssize_t mpc_cpu_fan_write(struct file *file, const char *buf, size_t size, loff_t *f_pos)
{
    memset(cpu_user_buff, 0, sizeof(cpu_user_buff));
    if (size > MPC_MAX_USER_SIZE)
    {
        size = MPC_MAX_USER_SIZE;
    }

    copy_from_user(cpu_user_buff, buf, size);

    int res = fan_write_to_ec(CPU_TEMP_BASE_ADDRESS, CPU_FAN_SPEED_BASE_ADDRESS, cpu_user_buff, &cpu_fan_config);

    if(res){
        printk(KERN_ERR "Error writing fan config\n");
    }


    return size;
}

int mpc_cpu_fan_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int mpc_cpu_fan_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations cpu_fan_fops =
    {
        read : mpc_cpu_fan_read,
        write : mpc_cpu_fan_write,
        open : mpc_cpu_fan_open,
        release : mpc_cpu_fan_release,
    };

int mpc_cpu_fan_create(void)
{
    int err = 0;
    err = msi_dev_create(&cpu_fan_dev, &cpu_fan_fops, "cpu_fan");
    return err;
}

void mpc_cpu_fan_destroy(void)
{
    msi_dev_destroy(&cpu_fan_dev);
}

ssize_t mpc_gpu_fan_read(struct file *file, char *buf, size_t size, loff_t *f_pos)
{
    char result[128];

    if(*f_pos >= sizeof(result)){
        return 0;
    }

    memset(result, 0, sizeof(result));
    
    if(fan_read_from_ec(GPU_TEMP_BASE_ADDRESS, GPU_FAN_SPEED_BASE_ADDRESS, result, sizeof(result))){
        copy_to_user(buf, "ERROR", 6);
        return 6;
    }

    if(copy_to_user(buf, result, sizeof(result))){
        printk(KERN_INFO "Not all data could be copied from kernel to user\n");
        return 0;
    }
    *f_pos+=sizeof(result);

    return sizeof(result);
}

ssize_t mpc_gpu_fan_write(struct file *file, const char *buf, size_t size, loff_t *f_pos)
{
    memset(gpu_user_buff, 0, sizeof(gpu_user_buff));
    if (size > MPC_MAX_USER_SIZE)
    {
        size = MPC_MAX_USER_SIZE;
    }

    copy_from_user(gpu_user_buff, buf, size);

    int res = fan_write_to_ec(GPU_TEMP_BASE_ADDRESS, GPU_FAN_SPEED_BASE_ADDRESS, gpu_user_buff, &gpu_fan_config);

    if(res){
        printk(KERN_ERR "Error writing fan config\n");
    }


    return size;
}

int mpc_gpu_fan_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int mpc_gpu_fan_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations gpu_fan_fops =
    {
        read : mpc_gpu_fan_read,
        write : mpc_gpu_fan_write,
        open : mpc_gpu_fan_open,
        release : mpc_gpu_fan_release,
    };

int mpc_gpu_fan_create(void)
{
    int err = 0;
    err = msi_dev_create(&gpu_fan_dev, &gpu_fan_fops, "gpu_fan");
    return err;
}

void mpc_gpu_fan_destroy(void)
{
    msi_dev_destroy(&gpu_fan_dev);
}