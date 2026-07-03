#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "mfw_client.h"

static int fake_fd = 42;
static int fake_open_rc = 42;
static int fake_ioctl_rc = 0;
static int fake_ioctl_errno = 0;
static int close_called;
static unsigned long last_request;
static void *last_arg;

int test_open(const char *path, int flags, ...)
{
    (void)flags;

    if (strcmp(path, MFW_DEVICE_PATH) != 0) {
        errno = ENOENT;
        return -1;
    }

    return fake_open_rc;
}

int test_ioctl(int fd, unsigned long request, ...)
{
    va_list ap;

    if (fd != fake_fd) {
        errno = EBADF;
        return -1;
    }

    va_start(ap, request);
    last_arg = va_arg(ap, void *);
    va_end(ap);

    last_request = request;
    errno = fake_ioctl_errno;

    return fake_ioctl_rc;
}

int test_close(int fd)
{
    if (fd == fake_fd) {
        close_called++;
        return 0;
    }

    errno = EBADF;
    return -1;
}

#define open test_open
#define ioctl test_ioctl
#define close test_close
#include "../../user/src/mfw_client.c"
#undef open
#undef ioctl
#undef close

static void reset_fake_syscalls(void)
{
    fake_open_rc = fake_fd;
    fake_ioctl_rc = 0;
    fake_ioctl_errno = 0;
    close_called = 0;
    last_request = 0;
    last_arg = NULL;
    errno = 0;
}

static int expect_int(const char *name, long actual, long expected)
{
    if (actual != expected) {
        fprintf(stderr, "[FAIL] %s: expected %ld, got %ld\n",
                name, expected, actual);
        return 1;
    }

    return 0;
}

int main(void)
{
    struct mfw_rule rule;
    struct mfw_rules_dump dump;
    __u32 src_ip = 0x01020304;
    int failures = 0;

    memset(&rule, 0, sizeof(rule));
    memset(&dump, 0, sizeof(dump));

    reset_fake_syscalls();
    failures += expect_int("add returns ioctl result",
                           mfw_client_add_rule(&rule),
                           0);
    failures += expect_int("add ioctl request",
                           last_request,
                           MFW_IOCTL_ADD_RULE);
    failures += expect_int("add closes fd", close_called, 1);
    if (last_arg != &rule) {
        fprintf(stderr, "[FAIL] add passed the wrong ioctl argument\n");
        failures++;
    }

    reset_fake_syscalls();
    failures += expect_int("delete returns ioctl result",
                           mfw_client_delete_rule(src_ip),
                           0);
    failures += expect_int("delete ioctl request",
                           last_request,
                           MFW_IOCTL_DEL_RULE);
    failures += expect_int("delete closes fd", close_called, 1);

    reset_fake_syscalls();
    failures += expect_int("get rules returns ioctl result",
                           mfw_client_get_rules(&dump),
                           0);
    failures += expect_int("get rules ioctl request",
                           last_request,
                           MFW_IOCTL_GET_RULES);
    if (last_arg != &dump) {
        fprintf(stderr, "[FAIL] get_rules passed the wrong ioctl argument\n");
        failures++;
    }

    reset_fake_syscalls();
    failures += expect_int("clear returns ioctl result",
                           mfw_client_clear_rules(),
                           0);
    failures += expect_int("clear ioctl request",
                           last_request,
                           MFW_IOCTL_CLEAR);

    reset_fake_syscalls();
    fake_open_rc = -1;
    errno = ENOENT;
    failures += expect_int("open failure is returned",
                           mfw_client_clear_rules(),
                           -1);
    failures += expect_int("open failure does not close fd",
                           close_called,
                           0);

    reset_fake_syscalls();
    fake_ioctl_rc = -1;
    fake_ioctl_errno = EOPNOTSUPP;
    failures += expect_int("ioctl failure is returned",
                           mfw_client_clear_rules(),
                           -1);
    failures += expect_int("ioctl errno is preserved",
                           errno,
                           EOPNOTSUPP);
    failures += expect_int("ioctl failure still closes fd",
                           close_called,
                           1);

    if (failures != 0) {
        return 1;
    }

    printf("[PASS] mfw_client ioctl wrapper logic is valid\n");
    return 0;
}
