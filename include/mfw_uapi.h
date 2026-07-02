#ifndef MFW_UAPI_H
#define MFW_UAPI_H

/* Shared UAPI header: included by both the kernel module and the user-space CLI. */
#include <linux/types.h>
#include <linux/ioctl.h>

#define MFW_MAX_RULES 64

#define MFW_ACTION_DROP 0
#define MFW_ACTION_PASS 1

/* IPv4 addresses are stored in network byte order, same as struct iphdr::saddr. */
struct mfw_rule {
    __u32 src_ip;
    __u8 action;
    __u8 enabled;
    __u8 reserved[2];
    __u64 hits;
};

struct mfw_rules_dump {
    __u32 count;
    struct mfw_rule rules[MFW_MAX_RULES];
};

#define MFW_IOCTL_BASE      'm'
#define MFW_IOCTL_ADD_RULE  _IOW(MFW_IOCTL_BASE, 1, struct mfw_rule)
#define MFW_IOCTL_DEL_RULE  _IOW(MFW_IOCTL_BASE, 2, __u32)
#define MFW_IOCTL_GET_RULES _IOR(MFW_IOCTL_BASE, 3, struct mfw_rules_dump)
#define MFW_IOCTL_CLEAR     _IO(MFW_IOCTL_BASE, 4)

#endif /* MFW_UAPI_H */
