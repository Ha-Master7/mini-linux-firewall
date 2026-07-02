// SPDX-License-Identifier: MIT
/* User-space CLI for Mini Linux Firewall Framework. */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "mfw_uapi.h"

#define MFW_DEV_PATH "/dev/mfw"

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s add <src-ip> <drop|pass>\n"
            "  %s del <src-ip>\n"
            "  %s list\n"
            "  %s clear\n\n"
            "Examples:\n"
            "  sudo %s add 192.168.56.1 drop\n"
            "  sudo %s list\n"
            "  sudo %s del 192.168.56.1\n",
            prog, prog, prog, prog, prog, prog, prog);
}

static int parse_ipv4(const char *text, __u32 *out)
{
    struct in_addr addr;

    if (inet_pton(AF_INET, text, &addr) != 1)
        return -1;

    *out = addr.s_addr; /* network byte order */
    return 0;
}

static int parse_action(const char *text, __u8 *out)
{
    if (strcmp(text, "drop") == 0 || strcmp(text, "DROP") == 0) {
        *out = MFW_ACTION_DROP;
        return 0;
    }

    if (strcmp(text, "pass") == 0 || strcmp(text, "PASS") == 0 ||
        strcmp(text, "accept") == 0 || strcmp(text, "ACCEPT") == 0) {
        *out = MFW_ACTION_PASS;
        return 0;
    }

    return -1;
}

static const char *action_name(__u8 action)
{
    switch (action) {
    case MFW_ACTION_DROP:
        return "DROP";
    case MFW_ACTION_PASS:
        return "PASS";
    default:
        return "UNKNOWN";
    }
}

static int open_dev(void)
{
    int fd = open(MFW_DEV_PATH, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "failed to open %s: %s\n", MFW_DEV_PATH, strerror(errno));
        fprintf(stderr, "is the kernel module loaded? try: sudo insmod kernel/mfw.ko\n");
    }

    return fd;
}

static int cmd_add(int argc, char **argv)
{
    struct mfw_rule rule;
    int fd;
    int ret;

    if (argc != 4) {
        usage(argv[0]);
        return 2;
    }

    memset(&rule, 0, sizeof(rule));
    if (parse_ipv4(argv[2], &rule.src_ip) != 0) {
        fprintf(stderr, "invalid IPv4 address: %s\n", argv[2]);
        return 2;
    }

    if (parse_action(argv[3], &rule.action) != 0) {
        fprintf(stderr, "invalid action: %s (expected drop/pass)\n", argv[3]);
        return 2;
    }

    rule.enabled = 1;

    fd = open_dev();
    if (fd < 0)
        return 1;

    ret = ioctl(fd, MFW_IOCTL_ADD_RULE, &rule);
    if (ret < 0) {
        fprintf(stderr, "ADD_RULE failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("added/updated rule: src=%s action=%s\n", argv[2], action_name(rule.action));
    close(fd);
    return 0;
}

static int cmd_del(int argc, char **argv)
{
    __u32 src_ip;
    int fd;
    int ret;

    if (argc != 3) {
        usage(argv[0]);
        return 2;
    }

    if (parse_ipv4(argv[2], &src_ip) != 0) {
        fprintf(stderr, "invalid IPv4 address: %s\n", argv[2]);
        return 2;
    }

    fd = open_dev();
    if (fd < 0)
        return 1;

    ret = ioctl(fd, MFW_IOCTL_DEL_RULE, &src_ip);
    if (ret < 0) {
        fprintf(stderr, "DEL_RULE failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("deleted rule: src=%s\n", argv[2]);
    close(fd);
    return 0;
}

static int cmd_clear(int argc, char **argv)
{
    int fd;
    int ret;

    if (argc != 2) {
        usage(argv[0]);
        return 2;
    }

    fd = open_dev();
    if (fd < 0)
        return 1;

    ret = ioctl(fd, MFW_IOCTL_CLEAR);
    if (ret < 0) {
        fprintf(stderr, "CLEAR failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("cleared all rules\n");
    close(fd);
    return 0;
}

static int cmd_list(int argc, char **argv)
{
    struct mfw_rules_dump dump;
    char ip[INET_ADDRSTRLEN];
    int fd;
    int ret;
    __u32 i;

    if (argc != 2) {
        usage(argv[0]);
        return 2;
    }

    fd = open_dev();
    if (fd < 0)
        return 1;

    memset(&dump, 0, sizeof(dump));
    ret = ioctl(fd, MFW_IOCTL_GET_RULES, &dump);
    if (ret < 0) {
        fprintf(stderr, "GET_RULES failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("%-4s %-15s %-6s %-20s\n", "ID", "SRC", "ACTION", "HITS");
    for (i = 0; i < dump.count; i++) {
        struct in_addr addr = { .s_addr = dump.rules[i].src_ip };
        if (!inet_ntop(AF_INET, &addr, ip, sizeof(ip)))
            snprintf(ip, sizeof(ip), "<bad-ip>");

        printf("%-4u %-15s %-6s %-20llu\n",
               i,
               ip,
               action_name(dump.rules[i].action),
               (unsigned long long)dump.rules[i].hits);
    }

    close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "add") == 0)
        return cmd_add(argc, argv);
    if (strcmp(argv[1], "del") == 0)
        return cmd_del(argc, argv);
    if (strcmp(argv[1], "list") == 0)
        return cmd_list(argc, argv);
    if (strcmp(argv[1], "clear") == 0)
        return cmd_clear(argc, argv);

    usage(argv[0]);
    return 2;
}
