// user/src/main.c

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mfw_user.h"

static void print_usage(const char *program_name)
{
    printf("Usage:\n");
    printf("  %s list\n", program_name);
    printf("  %s clear\n", program_name);
    printf("  %s add <src_ip> <drop|pass>\n", program_name);
    printf("  %s del <src_ip>\n", program_name);
}

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

static int parse_action(const char *text, __u8 *action)
{
    if (strcmp(text, "drop") == 0) {
        *action = MFW_ACTION_DROP;
        return 0;
    }

    if (strcmp(text, "pass") == 0 || strcmp(text, "accept") == 0) {
        *action = MFW_ACTION_PASS;
        return 0;
    }

    return -1;
}

static int parse_ipv4(const char *text, __u32 *out_ip)
{
    struct in_addr addr;

    if (inet_pton(AF_INET, text, &addr) != 1) {
        return -1;
    }

    *out_ip = addr.s_addr;

    return 0;
}

static void print_rules(const struct mfw_rules_dump *dump)
{
    __u32 i;

    printf("Rule count: %u\n", dump->count);

    if (dump->count == 0) {
        printf("No rules found.\n");
        return;
    }

    printf("%-5s %-16s %-8s %-8s %-10s\n",
           "ID",
           "SRC",
           "ACTION",
           "ENABLED",
           "HITS");

    for (i = 0; i < dump->count; i++) {
        const struct mfw_rule *rule = &dump->rules[i];
        struct in_addr addr;
        char ip_text[INET_ADDRSTRLEN];

        addr.s_addr = rule->src_ip;

        if (inet_ntop(AF_INET, &addr, ip_text, sizeof(ip_text)) == NULL) {
            snprintf(ip_text, sizeof(ip_text), "invalid");
        }

        printf("%-5u %-16s %-8s %-8u %-10llu\n",
               i,
               ip_text,
               action_to_string(rule->action),
               rule->enabled,
               (unsigned long long)rule->hits);
    }
}

static int handle_list(void)
{
    struct mfw_rules_dump dump;

    memset(&dump, 0, sizeof(dump));

    if (mfw_client_get_rules(&dump) != 0) {
        perror("mfwctl: list failed");
        return 1;
    }

    print_rules(&dump);

    return 0;
}

static int handle_clear(void)
{
    if (mfw_client_clear_rules() != 0) {
        perror("mfwctl: clear failed");
        return 1;
    }

    printf("Rules cleared.\n");

    return 0;
}

static int handle_add(int argc, char **argv)
{
    struct mfw_rule rule;

    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    memset(&rule, 0, sizeof(rule));

    if (parse_ipv4(argv[2], &rule.src_ip) != 0) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", argv[2]);
        return 1;
    }

    if (parse_action(argv[3], &rule.action) != 0) {
        fprintf(stderr, "Invalid action: %s\n", argv[3]);
        fprintf(stderr, "Expected: drop or pass\n");
        return 1;
    }

    rule.enabled = 1;
    rule.hits = 0;

    if (mfw_client_add_rule(&rule) != 0) {

        perror("mfwctl: add failed");
        return 1;
    }

    printf("Rule added.\n");

    return 0;
}

static int handle_del(int argc, char **argv)
{
    __u32 src_ip;

    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    if (parse_ipv4(argv[2], &src_ip) != 0) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", argv[2]);
        return 1;
    }

    if (mfw_client_delete_rule(src_ip) != 0) {

        perror("mfwctl: del failed");
        return 1;
    }

    printf("Rule deleted.\n");

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "list") == 0) {
        return handle_list();
    }

    if (strcmp(argv[1], "clear") == 0) {
        return handle_clear();
    }

    if (strcmp(argv[1], "add") == 0) {
        return handle_add(argc, argv);
    }

    if (strcmp(argv[1], "del") == 0) {
        return handle_del(argc, argv);
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage(argv[0]);

    return 1;
}