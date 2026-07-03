#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "mfw_uapi.h"
/*
 * Convert a rule action value to a printable string.
 */
static const char *action_to_string(__u8 action)
{
    if (action == MFW_ACTION_DROP) {
        return "DROP";
    }

    if (action == MFW_ACTION_PASS) {
        return "PASS";
    }

    return "UNKNOWN";
}

int main(void)
{
    struct mfw_rule rule;
    struct in_addr addr;

    /*
     * Clear the rule before filling it, including reserved bytes.
     */
    memset(&rule, 0, sizeof(rule));

    /*
     * Convert textual IPv4 address to binary network byte order.
     * This matches the format used by iphdr->saddr in the kernel.
     */
    if (inet_pton(AF_INET, "192.168.1.10", &addr) != 1) {
        printf("Invalid IP address\n");
        return 1;
    }

    rule.src_ip = addr.s_addr;
    rule.action = MFW_ACTION_DROP;
    rule.enabled = 1;
    rule.hits = 0;

    printf("MFW_MAX_RULES = %d\n", MFW_MAX_RULES);
    printf("sizeof(struct mfw_rule) = %zu bytes\n", sizeof(struct mfw_rule));
    printf("sizeof(struct mfw_rules_dump) = %zu bytes\n", sizeof(struct mfw_rules_dump));

    printf("Rule created successfully:\n");
    printf("  IP      = %s\n", inet_ntoa(addr));
    printf("  Action  = %s\n", action_to_string(rule.action));
    printf("  Enabled = %u\n", rule.enabled);
    printf("  Hits    = %llu\n", (unsigned long long)rule.hits);

    printf("\nioctl numbers:\n");
    printf("  ADD_RULE  = %lu\n", (unsigned long)MFW_IOCTL_ADD_RULE);
    printf("  DEL_RULE  = %lu\n", (unsigned long)MFW_IOCTL_DEL_RULE);
    printf("  GET_RULES = %lu\n", (unsigned long)MFW_IOCTL_GET_RULES);
    printf("  CLEAR     = %lu\n", (unsigned long)MFW_IOCTL_CLEAR);

    return 0;
}