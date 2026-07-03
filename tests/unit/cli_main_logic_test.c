#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "mfw_client.h"

int mfw_client_add_rule(const struct mfw_rule *rule)
{
    (void)rule;
    return 0;
}

int mfw_client_delete_rule(__u32 src_ip)
{
    (void)src_ip;
    return 0;
}

int mfw_client_get_rules(struct mfw_rules_dump *dump)
{
    memset(dump, 0, sizeof(*dump));
    return 0;
}

int mfw_client_clear_rules(void)
{
    return 0;
}

#define main mfwctl_program_main
#include "../../user/src/main.c"
#undef main

static int expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "[FAIL] %s: expected %d, got %d\n",
                name, expected, actual);
        return 1;
    }

    return 0;
}

int main(void)
{
    __u8 action = 99;
    __u32 ip = 0;
    struct in_addr expected_addr;
    int failures = 0;

    failures += expect_int("parse drop", parse_action("drop", &action), 0);
    failures += expect_int("drop value", action, MFW_ACTION_DROP);

    failures += expect_int("parse pass", parse_action("pass", &action), 0);
    failures += expect_int("pass value", action, MFW_ACTION_PASS);

    failures += expect_int("parse accept alias", parse_action("accept", &action), 0);
    failures += expect_int("accept alias value", action, MFW_ACTION_PASS);

    failures += expect_int("reject invalid action",
                           parse_action("block", &action),
                           -1);

    failures += expect_int("parse valid IPv4",
                           parse_ipv4("192.168.1.5", &ip),
                           0);

    if (inet_pton(AF_INET, "192.168.1.5", &expected_addr) != 1 ||
        ip != expected_addr.s_addr) {
        fprintf(stderr, "[FAIL] parsed IPv4 was not stored in network byte order\n");
        failures++;
    }

    failures += expect_int("reject invalid IPv4",
                           parse_ipv4("999.1.1.1", &ip),
                           -1);

    if (strcmp(action_to_string(MFW_ACTION_DROP), "DROP") != 0 ||
        strcmp(action_to_string(MFW_ACTION_PASS), "PASS") != 0 ||
        strcmp(action_to_string(200), "UNKNOWN") != 0) {
        fprintf(stderr, "[FAIL] action_to_string returned unexpected text\n");
        failures++;
    }

    if (failures != 0) {
        return 1;
    }

    printf("[PASS] CLI main parsing and formatting logic is valid\n");
    return 0;
}
