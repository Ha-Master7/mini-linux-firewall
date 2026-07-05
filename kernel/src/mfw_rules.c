// kernel/src/mfw_rules.c

#define pr_fmt(fmt) "mfw_rules: " fmt

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include "mfw_all.h"
#include "mfw_uapi.h"

/*
 * The rule table is kept inside the kernel module memory.
 *
 * Stage 5 uses a simple fixed-size array.
 * This is easy to understand and good enough for a first firewall version.
 */
static struct mfw_rule rules[MFW_MAX_RULES];
static __u32 rule_count;

/*
 * This lock protects the rule table.
 *
 * Why do we need it?
 *
 * In the future, rules can be accessed from:
 * 1. ioctl context   - user-space add/delete/list
 * 2. Netfilter hook  - packet path
 *
 * So the rule table is shared state.
 */
static spinlock_t rules_lock;

static int mfw_rule_is_valid(const struct mfw_rule *rule)
{
    if (rule == NULL) {
        return 0;
    }

    if (rule->action != MFW_ACTION_DROP &&
        rule->action != MFW_ACTION_PASS) {
        return 0;
    }

    if (rule->enabled != 0 && rule->enabled != 1) {
        return 0;
    }

    return 1;
}

int mfw_rules_init(void)
{
    unsigned long flags;

    spin_lock_init(&rules_lock);

    spin_lock_irqsave(&rules_lock, flags);
    memset(rules, 0, sizeof(rules));
    rule_count = 0;
    spin_unlock_irqrestore(&rules_lock, flags);

    pr_info("rule table initialized, max rules=%d\n", MFW_MAX_RULES);

    return 0;
}

void mfw_rules_exit(void)
{
    unsigned long flags;

    spin_lock_irqsave(&rules_lock, flags);
    memset(rules, 0, sizeof(rules));
    rule_count = 0;
    spin_unlock_irqrestore(&rules_lock, flags);

    pr_info("rule table cleaned up\n");
}

int mfw_rules_add(const struct mfw_rule *rule)
{
    __u32 i;
    struct mfw_rule new_rule;
    unsigned long flags;

    if (!mfw_rule_is_valid(rule)) {
        return -EINVAL;
    }

    /*
     * Make a local copy first.
     *
     * The caller may pass a stack object, and we do not want to keep
     * a pointer to it. We store only the value.
     */
    new_rule = *rule;
    new_rule.hits = 0;

    spin_lock_irqsave(&rules_lock, flags);

    for (i = 0; i < rule_count; i++) {
        if (rules[i].src_ip == new_rule.src_ip) {
            spin_unlock_irqrestore(&rules_lock, flags);
            return -EEXIST;
        }
    }

    if (rule_count >= MFW_MAX_RULES) {
        spin_unlock_irqrestore(&rules_lock, flags);
        return -ENOSPC;
    }

    rules[rule_count] = new_rule;
    rule_count++;

    spin_unlock_irqrestore(&rules_lock, flags);

    pr_info("rule added: src=%pI4 action=%u enabled=%u\n",
            &new_rule.src_ip,
            new_rule.action,
            new_rule.enabled);

    return 0;
}

int mfw_rules_delete(__u32 src_ip)
{
    __u32 i;
    __u32 j;
    unsigned long flags;

    spin_lock_irqsave(&rules_lock, flags);

    for (i = 0; i < rule_count; i++) {
        if (rules[i].src_ip == src_ip) {
            for (j = i; j + 1 < rule_count; j++) {
                rules[j] = rules[j + 1];
            }

            memset(&rules[rule_count - 1], 0, sizeof(rules[rule_count - 1]));
            rule_count--;

            spin_unlock_irqrestore(&rules_lock, flags);

            pr_info("rule deleted: src=%pI4\n", &src_ip);

            return 0;
        }
    }

    spin_unlock_irqrestore(&rules_lock, flags);

    return -ENOENT;
}

void mfw_rules_clear(void)
{
    unsigned long flags;

    spin_lock_irqsave(&rules_lock, flags);
    memset(rules, 0, sizeof(rules));
    rule_count = 0;
    spin_unlock_irqrestore(&rules_lock, flags);

    pr_info("all rules cleared\n");
}

void mfw_rules_snapshot(struct mfw_rules_dump *dump)
{
    __u32 i;
    unsigned long flags;

    if (dump == NULL) {
        return;
    }

    memset(dump, 0, sizeof(*dump));

    spin_lock_irqsave(&rules_lock, flags);

    dump->count = rule_count;

    for (i = 0; i < rule_count; i++) {
        dump->rules[i] = rules[i];
    }

    spin_unlock_irqrestore(&rules_lock, flags);
}

int mfw_rules_match_src_ip(__u32 src_ip, struct mfw_rule *out_rule)
{
    __u32 i;
    unsigned long flags;

    spin_lock_irqsave(&rules_lock, flags);

    for (i = 0; i < rule_count; i++) {
        if (rules[i].enabled == 1 && rules[i].src_ip == src_ip) {
            rules[i].hits++;

            if (out_rule != NULL) {
                *out_rule = rules[i];
            }

            spin_unlock_irqrestore(&rules_lock, flags);
            return 1;
        }
    }

    spin_unlock_irqrestore(&rules_lock, flags);

    return 0;
}
