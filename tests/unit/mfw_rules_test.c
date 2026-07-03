#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "mfw_rules.h"

#include "../../kernel/src/mfw_rules.c"

static int expect_int(const char *name, long actual, long expected)
{
    if (actual != expected) {
        fprintf(stderr, "[FAIL] %s: expected %ld, got %ld\n",
                name, expected, actual);
        return 1;
    }

    return 0;
}

static __u32 ipv4(const char *text)
{
    struct in_addr addr;

    if (inet_pton(AF_INET, text, &addr) != 1) {
        return 0;
    }

    return addr.s_addr;
}

static struct mfw_rule make_rule(const char *src_ip, __u8 action)
{
    struct mfw_rule rule;

    memset(&rule, 0, sizeof(rule));
    rule.src_ip = ipv4(src_ip);
    rule.action = action;
    rule.enabled = 1;
    rule.hits = 99;

    return rule;
}

int main(void)
{
    struct mfw_rule rule;
    struct mfw_rule matched;
    struct mfw_rules_dump dump;
    int failures = 0;

    failures += expect_int("rules init", mfw_rules_init(), 0);

    mfw_rules_snapshot(&dump);
    failures += expect_int("empty count after init", dump.count, 0);

    rule = make_rule("192.168.1.5", MFW_ACTION_DROP);
    failures += expect_int("add first rule", mfw_rules_add(&rule), 0);

    mfw_rules_snapshot(&dump);
    failures += expect_int("count after add", dump.count, 1);
    failures += expect_int("hits reset on add", dump.rules[0].hits, 0);

    failures += expect_int("duplicate rejected",
                           mfw_rules_add(&rule),
                           -EEXIST);

    memset(&matched, 0, sizeof(matched));
    failures += expect_int("match enabled source",
                           mfw_rules_match_src_ip(ipv4("192.168.1.5"), &matched),
                           1);
    failures += expect_int("matched action", matched.action, MFW_ACTION_DROP);
    failures += expect_int("hit counter incremented", matched.hits, 1);

    failures += expect_int("missing source does not match",
                           mfw_rules_match_src_ip(ipv4("10.0.0.8"), NULL),
                           0);

    failures += expect_int("delete existing rule",
                           mfw_rules_delete(ipv4("192.168.1.5")),
                           0);

    mfw_rules_snapshot(&dump);
    failures += expect_int("count after delete", dump.count, 0);

    failures += expect_int("delete missing rule",
                           mfw_rules_delete(ipv4("192.168.1.5")),
                           -ENOENT);

    rule = make_rule("10.0.0.8", 99);
    failures += expect_int("invalid action rejected",
                           mfw_rules_add(&rule),
                           -EINVAL);

    rule = make_rule("10.0.0.8", MFW_ACTION_PASS);
    rule.enabled = 2;
    failures += expect_int("invalid enabled value rejected",
                           mfw_rules_add(&rule),
                           -EINVAL);

    failures += expect_int("null rule rejected",
                           mfw_rules_add(NULL),
                           -EINVAL);

    rule = make_rule("10.0.0.8", MFW_ACTION_PASS);
    mfw_rules_add(&rule);
    mfw_rules_clear();
    mfw_rules_snapshot(&dump);
    failures += expect_int("count after clear", dump.count, 0);

    mfw_rules_exit();

    if (failures != 0) {
        return 1;
    }

    printf("[PASS] kernel rule-table logic is valid in unit tests\n");
    return 0;
}
