// kernel/src/mfw_netfilter.c

#define pr_fmt(fmt) "mfw_netfilter: " fmt

#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#include "mfw_netfilter.h"
#include "mfw_packet.h"

/*
 * Netfilter hook callback.
 *
 * Stage 6 behavior:
 * - receive IPv4 packets that are destined to the local machine
 * - parse packet information
 * - log source/destination/protocol
 * - always return NF_ACCEPT
 */
static unsigned int mfw_nf_hook(void *priv,
                                struct sk_buff *skb,
                                const struct nf_hook_state *state)
{
    struct mfw_packet_info info;
    int ret;

    (void)priv;
    (void)state;

    ret = mfw_packet_parse(skb, &info);
    if (ret != 0) {
        return NF_ACCEPT;
    }

    /*
     * Decision is delegated to the firewall engine.
     */
    return mfw_engine_decide(&info);
}


/*
 * Hook registration data.
 *
 * NF_INET_LOCAL_IN:
 *   packets that are entering the local machine.
 *
 * NFPROTO_IPV4:
 *   IPv4 only.
 */
static struct nf_hook_ops mfw_nf_ops = {
    .hook = mfw_nf_hook,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FIRST,
};

int mfw_netfilter_init(void)
{
    int ret;

    ret = nf_register_net_hook(&init_net, &mfw_nf_ops);
    if (ret != 0) {
        pr_err("failed to register Netfilter hook: %d\n", ret);
        return ret;
    }

    pr_info("Netfilter hook registered successfully\n");

    return 0;
}

void mfw_netfilter_exit(void)
{
    nf_unregister_net_hook(&init_net, &mfw_nf_ops);
    pr_info("Netfilter hook unregistered\n");
}