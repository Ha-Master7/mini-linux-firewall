/* mfw_device.c - /dev/mfw ioctl implementation */
// kernel/src/mfw_device.c

#define pr_fmt(fmt) "mfw_device: " fmt

#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "mfw_device.h"
#include "mfw_rules.h"
#include "mfw_uapi.h"

/*
 * mfw_ioctl
 *
 * This function is called whenever user-space sends an ioctl
 * command to /dev/mfw.
 *
 * The ioctl commands bridge user-space requests to the kernel rule table.
 */
static long mfw_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct mfw_rule rule;
    struct mfw_rules_dump *dump;
    __u32 src_ip;

    (void)file;

    switch (cmd) {
    case MFW_IOCTL_ADD_RULE:
        if (copy_from_user(&rule, (void __user *)arg, sizeof(rule)) != 0) {
            return -EFAULT;
        }

        pr_info("ADD_RULE received: src=%pI4 action=%u enabled=%u\n",
                &rule.src_ip,
                rule.action,
                rule.enabled);

        return mfw_rules_add(&rule);

    case MFW_IOCTL_DEL_RULE:
        if (copy_from_user(&src_ip, (void __user *)arg, sizeof(src_ip)) != 0) {
            return -EFAULT;
        }

        pr_info("DEL_RULE received: src=%pI4\n", &src_ip);

        return mfw_rules_delete(src_ip);

    case MFW_IOCTL_GET_RULES:
        dump = kzalloc(sizeof(*dump), GFP_KERNEL);
        if (dump == NULL) {
            return -ENOMEM;
        }

        mfw_rules_snapshot(dump);

        if (copy_to_user((void __user *)arg, dump, sizeof(*dump)) != 0) {
            kfree(dump);
            return -EFAULT;
        }

        pr_info("GET_RULES received: returned %u rules\n", dump->count);
        kfree(dump);

        return 0;

    case MFW_IOCTL_CLEAR:
        mfw_rules_clear();
        pr_info("CLEAR received: rules cleared\n");

        return 0;

    default:
        pr_err("Unknown ioctl command: 0x%x\n", cmd);
        return -ENOTTY;
    }
}

/*
 * File operations for /dev/mfw.
 *
 * unlocked_ioctl is the callback used for ioctl commands.
 */
static const struct file_operations mfw_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = mfw_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = mfw_ioctl,
#endif
};

/*
 * A misc device is the simplest way to create a small character device.
 *
 * It automatically allocates a minor number and creates:
 *
 * /dev/mfw
 */
static struct miscdevice mfw_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mfw",
    .fops = &mfw_fops,
    .mode = 0600,
};

int mfw_device_init(void)
{
    int ret;

    ret = misc_register(&mfw_misc_device);
    if (ret != 0) {
        pr_err("failed to register /dev/mfw: %d\n", ret);
        return ret;
    }

    pr_info("/dev/mfw registered successfully\n");

    return 0;
}

void mfw_device_exit(void)
{
    misc_deregister(&mfw_misc_device);
    pr_info("/dev/mfw unregistered\n");
}
 