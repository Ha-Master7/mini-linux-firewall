// kernel/src/mfw_engine.c

#define pr_fmt(fmt) "mfw_engine: " fmt

#include <linux/kernel.h>
#include <linux/netfilter.h>

#include "mfw_all.h"
#include "mfw_uapi.h"

/*
 * mfw_engine_decide
 *
 * This is the core firewall decision logic.
 *
 * It receives normalized packet information, not raw sk_buff.
 * This keeps the engine independent from the Netfilter details.
 *
 * Stage 7 behavior:
 * 1. Match by source IPv4 address.
 * 2. If no rule matches -> ACCEPT.
 * 3. If rule action is PASS -> ACCEPT.
 * 4. If rule action is DROP -> DROP.
 */
unsigned int mfw_engine_decide(const struct mfw_packet_info *info)
{
    struct mfw_rule matched_rule;
    int matched;

    if (info == NULL) {
        return NF_ACCEPT;
    }

    matched = mfw_rules_match_src_ip(info->src_ip, &matched_rule);
    if (!matched) {
        return NF_ACCEPT;
    }

    if (matched_rule.action == MFW_ACTION_DROP) {
        pr_info_ratelimited("DROP packet: src=%pI4 dst=%pI4 proto=%u hits=%llu\n",
                            &info->src_ip,
                            &info->dst_ip,
                            info->protocol,
                            (unsigned long long)matched_rule.hits);

        return NF_DROP;
    }

    if (matched_rule.action == MFW_ACTION_PASS) {
        pr_info_ratelimited("PASS packet: src=%pI4 dst=%pI4 proto=%u hits=%llu\n",
                            &info->src_ip,
                            &info->dst_ip,
                            info->protocol,
                            (unsigned long long)matched_rule.hits);

        return NF_ACCEPT;
    }

    /*
     * Defensive default:
     * if action is somehow invalid, do not break networking.
     */
    return NF_ACCEPT;
}