// kernel/src/mfw_packet.c

#define pr_fmt(fmt) "mfw_packet: " fmt

#include <linux/errno.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/string.h>

#include "mfw_packet.h"

/*
 * mfw_packet_parse
 *
 * Converts a raw sk_buff into a small packet_info structure.
 *
 * Stage 6:
 * - IPv4 only
 * - extract source IP
 * - extract destination IP
 * - extract protocol
 *
 * No TCP/UDP port parsing yet.
 */
int mfw_packet_parse(struct sk_buff *skb, struct mfw_packet_info *info)
{
    struct iphdr *iph;
    unsigned int ip_header_len;

    if (skb == NULL || info == NULL) {
        return -EINVAL;
    }

    memset(info, 0, sizeof(*info));

    /*
     * Make sure the packet contains at least a minimal IPv4 header.
     */
    if (!pskb_may_pull(skb, sizeof(struct iphdr))) {
        return -EINVAL;
    }

    iph = ip_hdr(skb);
    if (iph == NULL) {
        return -EINVAL;
    }

    if (iph->version != 4) {
        return -EINVAL;
    }

    if (iph->ihl < 5) {
        return -EINVAL;
    }

    ip_header_len = iph->ihl * 4;

    /*
     * Make sure the full IPv4 header is present.
     */
    if (!pskb_may_pull(skb, ip_header_len)) {
        return -EINVAL;
    }

    /*
     * pskb_may_pull may change skb internals, so refresh iph.
     */
    iph = ip_hdr(skb);
    if (iph == NULL) {
        return -EINVAL;
    }

    info->src_ip = iph->saddr;
    info->dst_ip = iph->daddr;
    info->protocol = iph->protocol;

    return 0;
}