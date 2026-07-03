#ifndef TEST_KERNEL_STUB_LINUX_KERNEL_H
#define TEST_KERNEL_STUB_LINUX_KERNEL_H

#include <stdio.h>

#define pr_info(fmt, ...) do { (void)printf(fmt, ##__VA_ARGS__); } while (0)
#define pr_err(fmt, ...) do { (void)fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#endif
