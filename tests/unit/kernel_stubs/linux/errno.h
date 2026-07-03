#ifndef TEST_KERNEL_STUB_LINUX_ERRNO_H
#define TEST_KERNEL_STUB_LINUX_ERRNO_H

#include_next <errno.h>

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef EEXIST
#define EEXIST 17
#endif

#ifndef ENOSPC
#define ENOSPC 28
#endif

#ifndef ENOENT
#define ENOENT 2
#endif

#endif
