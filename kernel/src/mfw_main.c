// kernel/src/mfw_main.c

#define pr_fmt(fmt) "mfw: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "mfw_device.h"
#include "mfw_rules.h"
#include "mfw_netfilter.h"
#include "mfw_debugfs.h"

/*
 * mfw_init
 *
 * This function is called when the module is loaded:
 *
 * sudo insmod mfw.ko
 */
static int __init mfw_init(void)
{
    int ret;

    pr_info("Mini Firewall kernel module loading\n");

    ret = mfw_device_init();
    if (ret != 0) {
        pr_err("failed to initialize device layer\n");
        return ret;
    }

    pr_info("Mini Firewall kernel module loaded successfully\n");

    return 0;
}

/*
 * mfw_exit
 *
 * This function is called when the module is removed:
 *
 * sudo rmmod mfw
 */
static void __exit mfw_exit(void)
{
    pr_info("Mini Firewall kernel module unloading\n");

    mfw_device_exit();

    pr_info("Mini Firewall kernel module unloaded successfully\n");
}

module_init(mfw_init);
module_exit(mfw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mini Linux Firewall Project");
MODULE_DESCRIPTION("Mini Linux Firewall - Stage 2 Kernel Module Skeleton");
MODULE_VERSION("0.1");