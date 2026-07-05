/* mfw_debugfs.c - skeleton (no implementation) */
// kernel/src/mfw_debugfs.c

#define pr_fmt(fmt) "mfw_debugfs: " fmt

#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include "mfw_all.h"
#include "mfw_uapi.h"

static struct dentry *mfw_debugfs_dir;
static struct dentry *mfw_debugfs_rules_file;

static const char *mfw_action_to_string(__u8 action)
{
    if (action == MFW_ACTION_DROP) {
        return "DROP";
    }

    if (action == MFW_ACTION_PASS) {
        return "PASS";
    }

    return "UNKNOWN";
}

/*
 * mfw_debugfs_rules_show
 *
 * This function is called when user-space reads:
 *
 * sudo cat /sys/kernel/debug/mfw/rules
 *
 * It prints a snapshot of the current rule table.
 */
static int mfw_debugfs_rules_show(struct seq_file *seq, void *private)
{
    struct mfw_rules_dump dump;
    __u32 i;

    (void)private;

    mfw_rules_snapshot(&dump);

    seq_puts(seq, "Mini Firewall Rules\n\n");
    seq_printf(seq, "Rule count: %u\n\n", dump.count);

    if (dump.count == 0) {
        seq_puts(seq, "No rules found.\n");
        return 0;
    }

    seq_printf(seq,
               "%-5s %-16s %-8s %-8s %-10s\n",
               "ID",
               "SRC",
               "ACTION",
               "ENABLED",
               "HITS");

    for (i = 0; i < dump.count; i++) {
        const struct mfw_rule *rule = &dump.rules[i];

        seq_printf(seq,
                   "%-5u %-16pI4 %-8s %-8u %-10llu\n",
                   i,
                   &rule->src_ip,
                   mfw_action_to_string(rule->action),
                   rule->enabled,
                   (unsigned long long)rule->hits);
    }

    return 0;
}

static int mfw_debugfs_rules_open(struct inode *inode, struct file *file)
{
    return single_open(file, mfw_debugfs_rules_show, inode->i_private);
}

static const struct file_operations mfw_debugfs_rules_fops = {
    .owner = THIS_MODULE,
    .open = mfw_debugfs_rules_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

int mfw_debugfs_init(void)
{
    mfw_debugfs_dir = debugfs_create_dir("mfw", NULL);
    if (mfw_debugfs_dir == NULL) {
        pr_err("failed to create debugfs directory\n");
        return -ENOMEM;
    }

    if (IS_ERR(mfw_debugfs_dir)) {
        pr_err("failed to create debugfs directory: %ld\n",
               PTR_ERR(mfw_debugfs_dir));
        return PTR_ERR(mfw_debugfs_dir);
    }

    mfw_debugfs_rules_file = debugfs_create_file("rules",
                                                 0400,
                                                 mfw_debugfs_dir,
                                                 NULL,
                                                 &mfw_debugfs_rules_fops);
    if (mfw_debugfs_rules_file == NULL) {
        pr_err("failed to create debugfs rules file\n");
        debugfs_remove_recursive(mfw_debugfs_dir);
        mfw_debugfs_dir = NULL;
        return -ENOMEM;
    }

    if (IS_ERR(mfw_debugfs_rules_file)) {
        long err = PTR_ERR(mfw_debugfs_rules_file);

        pr_err("failed to create debugfs rules file: %ld\n", err);

        debugfs_remove_recursive(mfw_debugfs_dir);
        mfw_debugfs_dir = NULL;
        mfw_debugfs_rules_file = NULL;

        return err;
    }

    pr_info("debugfs created at /sys/kernel/debug/mfw\n");

    return 0;
}

void mfw_debugfs_exit(void)
{
    debugfs_remove_recursive(mfw_debugfs_dir);

    mfw_debugfs_rules_file = NULL;
    mfw_debugfs_dir = NULL;

    pr_info("debugfs removed\n");
}