/* mfw_engine.h - skeleton */
#ifndef MFW_ENGINE_H
#define MFW_ENGINE_H

#include <linux/netfilter.h>

#include "mfw_packet.h"

/*
 * Decide what to do with a parsed packet.
 *
 * Returns:
 *   NF_ACCEPT
 *   NF_DROP
 */
unsigned int mfw_engine_decide(const struct mfw_packet_info *info);

#endif /* MFW_ENGINE_H */