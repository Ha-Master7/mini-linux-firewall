#ifndef TEST_KERNEL_STUB_LINUX_IOCTL_H
#define TEST_KERNEL_STUB_LINUX_IOCTL_H

#define _IO(type, nr) ((unsigned long)(((type) << 8) | (nr)))
#define _IOR(type, nr, size) _IO(type, nr)
#define _IOW(type, nr, size) _IO(type, nr)

#endif
