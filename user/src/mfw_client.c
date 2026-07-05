// user/src/mfw_client.c

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "mfw_user.h"

static int mfw_open_device(void)
{
    return open(MFW_DEVICE_PATH, O_RDWR);
}

int mfw_client_add_rule(const struct mfw_rule *rule)
{
    int fd;
    int ret;
    int saved_errno;

    fd = mfw_open_device();
    if (fd < 0) {
        return -1;
    }

    ret = ioctl(fd, MFW_IOCTL_ADD_RULE, rule);
    saved_errno = errno;

    close(fd);
    errno = saved_errno;

    return ret;
}

int mfw_client_delete_rule(__u32 src_ip)
{
    int fd;
    int ret;
    int saved_errno;

    fd = mfw_open_device();
    if (fd < 0) {
        return -1;
    }

    ret = ioctl(fd, MFW_IOCTL_DEL_RULE, &src_ip);
    saved_errno = errno;

    close(fd);
    errno = saved_errno;

    return ret;
}

int mfw_client_get_rules(struct mfw_rules_dump *dump)
{
    int fd;
    int ret;
    int saved_errno;

    fd = mfw_open_device();
    if (fd < 0) {
        return -1;
    }

    ret = ioctl(fd, MFW_IOCTL_GET_RULES, dump);
    saved_errno = errno;

    close(fd);
    errno = saved_errno;

    return ret;
}

int mfw_client_clear_rules(void)
{
    int fd;
    int ret;
    int saved_errno;

    fd = mfw_open_device();
    if (fd < 0) {
        return -1;
    }

    ret = ioctl(fd, MFW_IOCTL_CLEAR);
    saved_errno = errno;

    close(fd);
    errno = saved_errno;

    return ret;
}