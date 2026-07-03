/* mfw_rules.h - skeleton */
#ifndef MFW_RULES_H
#define MFW_RULES_H

#include "mfw_uapi.h"

/*
 * Initialize the kernel rule table.
 */
int mfw_rules_init(void);

/*
 * Cleanup the kernel rule table.
 */
void mfw_rules_exit(void);

/*
 * Add a new firewall rule.
 *
 * Returns:
 *  0         on success
 * -EINVAL    invalid rule
 * -EEXIST    rule for this IP already exists
 * -ENOSPC    rule table is full
 */
int mfw_rules_add(const struct mfw_rule *rule);

/*
 * Delete a firewall rule by source IP.
 *
 * src_ip is stored in network byte order.
 */
int mfw_rules_delete(__u32 src_ip);

/*
 * Remove all firewall rules.
 */
void mfw_rules_clear(void);

/*
 * Copy the current rules into a dump structure.
 *
 * This gives user-space a safe snapshot of the current table.
 */
void mfw_rules_snapshot(struct mfw_rules_dump *dump);

/*
 * Future helper for the firewall engine.
 *
 * It searches for a matching enabled rule by source IP.
 * If found, it increments the hit counter and optionally copies
 * the matched rule into out_rule.
 *
 * Returns:
 *  1 if a matching rule was found
 *  0 otherwise
 */
int mfw_rules_match_src_ip(__u32 src_ip, struct mfw_rule *out_rule);

#endif /* MFW_RULES_H */