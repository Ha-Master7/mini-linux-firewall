/* mfw_client.h - skeleton */
#ifndef MFW_CLIENT_H
#define MFW_CLIENT_H

#include "mfw_uapi.h"

#define MFW_DEVICE_PATH "/dev/mfw"

int mfw_client_add_rule(const struct mfw_rule *rule);
int mfw_client_delete_rule(__u32 src_ip);
int mfw_client_get_rules(struct mfw_rules_dump *dump);
int mfw_client_clear_rules(void);

#endif /* MFW_CLIENT_H */