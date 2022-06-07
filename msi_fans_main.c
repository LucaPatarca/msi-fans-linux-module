/**
 * @file    hello.c
 * @author  Akshat Sinha
 * @date    10 Sept 2016
 * @version 0.1
 * @brief  An introductory "Hello World!" loadable kernel
 *  module (LKM) that can display a message in the /var/log/kern.log
 *  file when the module is loaded and removed. The module can accept
 *  an argument when it is loaded -- the name, which appears in the
 *  kernel log files.
 */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/init.h>   /* Needed for the macros */

#include "include/msi_cooler_boost.h"
#include "include/msi_fan.h"
#include "include/msi_device.h"

///< The license type -- this affects runtime behavior
MODULE_LICENSE("GPL");

///< The author -- visible when you use modinfo
MODULE_AUTHOR("Luca Patarca");

///< The description -- see modinfo
MODULE_DESCRIPTION("A driver for MSI laptop fans");

///< The version of the module
MODULE_VERSION("0.1");

static void cleanup(void)
{
    msi_cooler_boost_destroy();
    mpc_cpu_fan_destroy();
    mpc_gpu_fan_destroy();
    msi_class_destroy();
}

static int __init hello_start(void)
{
    printk(KERN_INFO "hello: Loading hello module...\n");
    int err;
    err = msi_cooler_boost_create();
    if (err)
    {
        cleanup();
        printk(KERN_ERR "hello: Error creating cooler boost device\n");
        return err;
    }
    err = mpc_cpu_fan_create();
    if (err)
    {
        cleanup();
        printk(KERN_ERR "hello: Error creating cpu fan device\n");
        return err;
    }
    err = mpc_gpu_fan_create();
    if (err)
    {
        cleanup();
        printk(KERN_ERR "hello: Error creating gpu fan device\n");
        return err;
    }
    return 0;
}

static void __exit hello_end(void)
{
    cleanup();
    printk(KERN_INFO "hello: Goodbye Mr.\n");
}

module_init(hello_start);
module_exit(hello_end);