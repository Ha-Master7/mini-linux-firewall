#ifndef _LINUX_SKBUFF_H
#define _LINUX_SKBUFF_H

/*
 * Minimal user-space unit-test stub for Linux kernel struct sk_buff.
 *
 * The real struct sk_buff is defined inside the Linux kernel.
 * In unit tests, we only need the type to exist so headers that use
 * struct sk_buff * can compile.
 */
struct sk_buff {
    int dummy;
};

#endif /* _LINUX_SKBUFF_H */
