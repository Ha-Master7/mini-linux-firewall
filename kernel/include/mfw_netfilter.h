/* mfw_netfilter.h - skeleton */
#ifndef MFW_NETFILTER_H
#define MFW_NETFILTER_H

/*
 * Register the Netfilter hook.
 */
int mfw_netfilter_init(void);

/*
 * Unregister the Netfilter hook.
 */
void mfw_netfilter_exit(void);

#endif /* MFW_NETFILTER_H */