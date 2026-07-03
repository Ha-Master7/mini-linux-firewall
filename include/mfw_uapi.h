#ifndef MFW_UAPI_H 
#define MFW_UAPI_H

/*
 * Mini Firewall - shared User/Kernel API.
 *
 * This header is included by both the user-space CLI and the kernel module.
 * Keep this file limited to shared constants, structs, and ioctl numbers.
 */

#include <linux/types.h>
#include <linux/ioctl.h>

/*
 * Version 1 uses a fixed-size rule table in the kernel.
 */
#define MFW_MAX_RULES 64

/*
 * Firewall actions.
 */
#define MFW_ACTION_DROP 0
#define MFW_ACTION_PASS 1

/*
 * A single firewall rule.
 *
 * src_ip is stored in network byte order so it can be compared directly
 * with iphdr->saddr inside the kernel.
 *
 * hits is updated by the kernel when packets match this rule.
 */
struct mfw_rule {
    __u32 src_ip;
    __u8 action;
    __u8 enabled;
    __u8 reserved[2];
    __u64 hits;
};

/*
 * Snapshot of the current rule table.
 *
 * Only rules[0] through rules[count - 1] are valid.
 */
struct mfw_rules_dump {
    __u32 count;
    struct mfw_rule rules[MFW_MAX_RULES];
};

/*
 * ioctl command group for Mini Firewall.
 */
#define MFW_IOCTL_BASE 'm'

/*
 * ioctl commands.
 *
 * _IOW: user-space sends data to the kernel.
 * _IOR: user-space receives data from the kernel.
 * _IO : command without extra data.
 */
#define MFW_IOCTL_ADD_RULE  _IOW(MFW_IOCTL_BASE, 1, struct mfw_rule)
#define MFW_IOCTL_DEL_RULE  _IOW(MFW_IOCTL_BASE, 2, __u32)
#define MFW_IOCTL_GET_RULES _IOR(MFW_IOCTL_BASE, 3, struct mfw_rules_dump)
#define MFW_IOCTL_CLEAR     _IO(MFW_IOCTL_BASE, 4)

#endif /* MFW_UAPI_H */