// kernel/src/mfw_main.c

#define pr_fmt(fmt) "mfw: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/*
 * mfw_init
 *
 * This function is called when the module is loaded:
 *
 * sudo insmod mfw.ko
 */
static int __init mfw_init(void)
{
    pr_info("Mini Firewall kernel module loaded\n");
    pr_info("Stage 2: kernel module skeleton is working\n");

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
    pr_info("Mini Firewall kernel module unloaded\n");
}

module_init(mfw_init);
module_exit(mfw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mini Linux Firewall Project");
MODULE_DESCRIPTION("Mini Linux Firewall - Stage 2 Kernel Module Skeleton");
MODULE_VERSION("0.1");