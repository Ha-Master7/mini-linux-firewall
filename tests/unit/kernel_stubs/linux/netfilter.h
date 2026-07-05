#ifndef TEST_KERNEL_STUB_LINUX_NETFILTER_H
#define TEST_KERNEL_STUB_LINUX_NETFILTER_H

#define NF_ACCEPT 1
#define NF_DROP 0
#define NF_INET_LOCAL_IN 0
#define NF_IP_PRI_FIRST 0

struct nf_hook_state;

struct nf_hook_ops {
    unsigned int (*hook)(void *priv, struct nf_hook_state *state);
    int hooknum;
    int priority;
};

#endif /* TEST_KERNEL_STUB_LINUX_NETFILTER_H */
