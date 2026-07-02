// SPDX-License-Identifier: GPL-2.0
/*
 * Mini Linux Firewall Framework (MFW)
 *
 * Educational kernel module:
 * - Registers an IPv4 Netfilter hook at LOCAL_IN.
 * - Maintains a small in-kernel source-IP rule table.
 * - Exposes a /dev/mfw ioctl interface for a user-space CLI.
 * - Exposes counters via debugfs: /sys/kernel/debug/mfw/rules
 *
 * Safety notes:
 * - Run in a VM, not on your main workstation.
 * - This is not production firewall code.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#include "mfw_uapi.h"

#define MFW_DEVICE_NAME "mfw"
#define MFW_DEBUGFS_DIR "mfw"

static struct mfw_rule mfw_rules[MFW_MAX_RULES];
static u32 mfw_rule_count;
static spinlock_t mfw_rules_lock;
static struct dentry *mfw_debugfs_dir;

static const char *mfw_action_name(u8 action)
{
    switch (action) {
    case MFW_ACTION_DROP:
        return "DROP";
    case MFW_ACTION_PASS:
        return "PASS";
    default:
        return "UNKNOWN";
    }
}

static int mfw_find_rule_locked(__u32 src_ip)
{
    u32 i;

    for (i = 0; i < mfw_rule_count; i++) {
        if (mfw_rules[i].enabled && mfw_rules[i].src_ip == src_ip)
            return (int)i;
    }

    return -1;
}

static unsigned int mfw_nf_hook(void *priv,
                                struct sk_buff *skb,
                                const struct nf_hook_state *state)
{
    struct iphdr *iph;
    unsigned int verdict = NF_ACCEPT;
    int idx;

    if (!skb)
        return NF_ACCEPT;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;

    /* This educational version only filters IPv4 source address. */
    spin_lock_bh(&mfw_rules_lock);
    idx = mfw_find_rule_locked(iph->saddr);
    if (idx >= 0) {
        mfw_rules[idx].hits++;

        if (mfw_rules[idx].action == MFW_ACTION_DROP) {
            verdict = NF_DROP;
            pr_info_ratelimited("mfw: dropping packet from %pI4\n", &iph->saddr);
        }
    }
    spin_unlock_bh(&mfw_rules_lock);

    return verdict;
}

static struct nf_hook_ops mfw_nfho = {
    .hook = mfw_nf_hook,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FIRST,
};

static long mfw_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct mfw_rule rule;
    struct mfw_rules_dump dump;
    __u32 src_ip;
    unsigned long flags;
    int idx;
    u32 i;

    switch (cmd) {
    case MFW_IOCTL_ADD_RULE:
        if (copy_from_user(&rule, (void __user *)arg, sizeof(rule)))
            return -EFAULT;

        if (rule.action != MFW_ACTION_DROP && rule.action != MFW_ACTION_PASS)
            return -EINVAL;

        rule.enabled = 1;
        rule.hits = 0;
        rule.reserved[0] = 0;
        rule.reserved[1] = 0;

        spin_lock_irqsave(&mfw_rules_lock, flags);
        idx = mfw_find_rule_locked(rule.src_ip);
        if (idx >= 0) {
            /* Update existing rule but preserve its hit counter. */
            mfw_rules[idx].action = rule.action;
            mfw_rules[idx].enabled = 1;
        } else {
            if (mfw_rule_count >= MFW_MAX_RULES) {
                spin_unlock_irqrestore(&mfw_rules_lock, flags);
                return -ENOSPC;
            }
            mfw_rules[mfw_rule_count++] = rule;
        }
        spin_unlock_irqrestore(&mfw_rules_lock, flags);
        return 0;

    case MFW_IOCTL_DEL_RULE:
        if (copy_from_user(&src_ip, (void __user *)arg, sizeof(src_ip)))
            return -EFAULT;

        spin_lock_irqsave(&mfw_rules_lock, flags);
        idx = mfw_find_rule_locked(src_ip);
        if (idx < 0) {
            spin_unlock_irqrestore(&mfw_rules_lock, flags);
            return -ENOENT;
        }

        for (i = idx; i + 1 < mfw_rule_count; i++)
            mfw_rules[i] = mfw_rules[i + 1];
        mfw_rule_count--;
        memset(&mfw_rules[mfw_rule_count], 0, sizeof(mfw_rules[mfw_rule_count]));
        spin_unlock_irqrestore(&mfw_rules_lock, flags);
        return 0;

    case MFW_IOCTL_GET_RULES:
        memset(&dump, 0, sizeof(dump));

        spin_lock_irqsave(&mfw_rules_lock, flags);
        dump.count = mfw_rule_count;
        memcpy(dump.rules, mfw_rules, sizeof(struct mfw_rule) * mfw_rule_count);
        spin_unlock_irqrestore(&mfw_rules_lock, flags);

        if (copy_to_user((void __user *)arg, &dump, sizeof(dump)))
            return -EFAULT;
        return 0;

    case MFW_IOCTL_CLEAR:
        spin_lock_irqsave(&mfw_rules_lock, flags);
        memset(mfw_rules, 0, sizeof(mfw_rules));
        mfw_rule_count = 0;
        spin_unlock_irqrestore(&mfw_rules_lock, flags);
        return 0;

    default:
        return -ENOTTY;
    }
}

static const struct file_operations mfw_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = mfw_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = mfw_ioctl,
#endif
};

static struct miscdevice mfw_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MFW_DEVICE_NAME,
    .fops = &mfw_fops,
    .mode = 0600,
};

static int mfw_debug_rules_show(struct seq_file *m, void *v)
{
    struct mfw_rule snapshot[MFW_MAX_RULES];
    u32 count;
    u32 i;
    unsigned long flags;

    spin_lock_irqsave(&mfw_rules_lock, flags);
    count = mfw_rule_count;
    memcpy(snapshot, mfw_rules, sizeof(struct mfw_rule) * count);
    spin_unlock_irqrestore(&mfw_rules_lock, flags);

    seq_printf(m, "Mini Linux Firewall Framework\n");
    seq_printf(m, "rules: %u/%u\n", count, MFW_MAX_RULES);
    seq_printf(m, "%-4s %-15s %-6s %-20s\n", "ID", "SRC", "ACTION", "HITS");

    for (i = 0; i < count; i++) {
        seq_printf(m, "%-4u %pI4 %-6s %-20llu\n",
                   i,
                   &snapshot[i].src_ip,
                   mfw_action_name(snapshot[i].action),
                   snapshot[i].hits);
    }

    return 0;
}

static int mfw_debug_rules_open(struct inode *inode, struct file *file)
{
    return single_open(file, mfw_debug_rules_show, inode->i_private);
}

static const struct file_operations mfw_debug_rules_fops = {
    .owner = THIS_MODULE,
    .open = mfw_debug_rules_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init mfw_init(void)
{
    int ret;

    spin_lock_init(&mfw_rules_lock);

    ret = misc_register(&mfw_miscdev);
    if (ret) {
        pr_err("mfw: failed to register /dev/%s: %d\n", MFW_DEVICE_NAME, ret);
        return ret;
    }

    mfw_debugfs_dir = debugfs_create_dir(MFW_DEBUGFS_DIR, NULL);
    if (IS_ERR_OR_NULL(mfw_debugfs_dir)) {
        ret = mfw_debugfs_dir ? PTR_ERR(mfw_debugfs_dir) : -ENOMEM;
        pr_err("mfw: failed to create debugfs dir: %d\n", ret);
        misc_deregister(&mfw_miscdev);
        return ret;
    }

    debugfs_create_file("rules", 0444, mfw_debugfs_dir, NULL, &mfw_debug_rules_fops);

    ret = nf_register_net_hook(&init_net, &mfw_nfho);
    if (ret) {
        pr_err("mfw: failed to register netfilter hook: %d\n", ret);
        debugfs_remove_recursive(mfw_debugfs_dir);
        misc_deregister(&mfw_miscdev);
        return ret;
    }

    pr_info("mfw: loaded. device=/dev/%s debugfs=/sys/kernel/debug/%s/rules hook=LOCAL_IN\n",
            MFW_DEVICE_NAME, MFW_DEBUGFS_DIR);
    return 0;
}

static void __exit mfw_exit(void)
{
    nf_unregister_net_hook(&init_net, &mfw_nfho);
    debugfs_remove_recursive(mfw_debugfs_dir);
    misc_deregister(&mfw_miscdev);
    pr_info("mfw: unloaded\n");
}

module_init(mfw_init);
module_exit(mfw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Educational project scaffold");
MODULE_DESCRIPTION("Mini Linux Firewall Framework using Netfilter + ioctl + debugfs");
MODULE_VERSION("0.1");
