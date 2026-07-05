/* mfw_packet.h - skeleton */

#ifndef MFW_PACKET_H
#define MFW_PACKET_H

#include <linux/skbuff.h>
#include <linux/types.h>

/*
 * Normalized packet information.
 *
 * The Netfilter hook receives a raw struct sk_buff.
 * The firewall engine should not need to understand sk_buff directly.
 *
 * This struct contains only the packet fields we care about.
 */
struct mfw_packet_info {
    __u32 src_ip;
    __u32 dst_ip;
    __u8 protocol;
};

/*
 * Parse an IPv4 packet from skb into mfw_packet_info.
 *
 * Returns:
 *  0 on success
 * -EINVAL if skb is invalid or not a valid IPv4 packet
 */
int mfw_packet_parse(struct sk_buff *skb, struct mfw_packet_info *info);

#endif /* MFW_PACKET_H */