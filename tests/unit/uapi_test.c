#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "mfw_uapi.h"

static int expect_int(const char *name, unsigned long actual, unsigned long expected)
{
    if (actual != expected) {
        fprintf(stderr, "[FAIL] %s: expected %lu, got %lu\n",
                name, expected, actual);
        return 1;
    }

    return 0;
}

int main(void)
{
    struct mfw_rule rule;
    struct mfw_rules_dump dump;
    struct in_addr addr;
    int failures = 0;

    failures += expect_int("MFW_MAX_RULES", MFW_MAX_RULES, 64);
    failures += expect_int("MFW_ACTION_DROP", MFW_ACTION_DROP, 0);
    failures += expect_int("MFW_ACTION_PASS", MFW_ACTION_PASS, 1);

    memset(&rule, 0, sizeof(rule));
    memset(&dump, 0, sizeof(dump));

    if (inet_pton(AF_INET, "192.168.1.10", &addr) != 1) {
        fprintf(stderr, "[FAIL] failed to parse IPv4 address\n");
        return 1;
    }

    rule.src_ip = addr.s_addr;
    rule.action = MFW_ACTION_DROP;
    rule.enabled = 1;
    rule.hits = 7;

    if (rule.src_ip != addr.s_addr ||
        rule.action != MFW_ACTION_DROP ||
        rule.enabled != 1 ||
        rule.hits != 7) {
        fprintf(stderr, "[FAIL] struct mfw_rule field assignment failed\n");
        failures++;
    }

    dump.count = 0;
    if (dump.count != 0) {
        fprintf(stderr, "[FAIL] struct mfw_rules_dump count assignment failed\n");
        failures++;
    }

    printf("MFW_MAX_RULES = %d\n", MFW_MAX_RULES);
    printf("sizeof(struct mfw_rule) = %zu\n", sizeof(struct mfw_rule));
    printf("sizeof(struct mfw_rules_dump) = %zu\n", sizeof(struct mfw_rules_dump));
    printf("MFW_IOCTL_ADD_RULE = %lu\n", (unsigned long)MFW_IOCTL_ADD_RULE);
    printf("MFW_IOCTL_DEL_RULE = %lu\n", (unsigned long)MFW_IOCTL_DEL_RULE);
    printf("MFW_IOCTL_GET_RULES = %lu\n", (unsigned long)MFW_IOCTL_GET_RULES);
    printf("MFW_IOCTL_CLEAR = %lu\n", (unsigned long)MFW_IOCTL_CLEAR);

    if (failures != 0) {
        return 1;
    }

    printf("[PASS] UAPI constants, structs and ioctl macros are valid\n");
    return 0;
}
