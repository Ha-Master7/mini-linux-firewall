# Flowchart

## Overview

This file describes the main project flow and how the different parts connect.

## Project components

- `kernel/`: Linux kernel module.
- `user/`: user-space command-line client.
- `include/`: shared API definitions.
- `scripts/`: helper scripts for module actions.
- `docs/`: architecture and planning documents.

## Main flow

1. Build the project using the top-level `Makefile`.
2. Load the kernel module from `kernel/`.
3. Run the CLI from `user/`.
4. The CLI communicates with the kernel module through `/dev/mfw` using `ioctl`.
5. The kernel module filters packets using Netfilter.
6. Internal state is exposed through debugfs under `/sys/kernel/debug/mfw/rules`.

## ASCII flowchart

```

┌──────────────────────────────────────────────┐
│                 User Space                   │
│                                              │
│  main.c                                      │
│  - parse command                             │
│  - parse IP/action                           │
│  - print rules                               │
│        |                                     │
│        v                                     │
│  mfw_client.c                                │
│  - open /dev/mfw                             │
│  - ioctl                                     │
│  - close                                     │
└───────────────|──────────────────────────────┘
                |
                | ioctl
                v
┌──────────────────────────────────────────────┐
│                Kernel Space                  │
│                                              │
│  mfw_device.c                                │
│  - receives ioctl commands                   │
│  - copy_from_user                            │
│  - copy_to_user                              │
│        |                                     │
│        v                                     │
│  mfw_rules.c                                 │
│  - stores rules                              │
│  - add/delete/list/clear                     │
│  - match packet source IP                    │
│        ^                                     │
│        |                                     │
│  mfw_netfilter.c                             │
│  - receives packets                          │
│  - extracts source IP                        │
│  - returns NF_ACCEPT/NF_DROP                 │
│                                              │
│  mfw_debugfs.c                               │
│  - exposes debug/counters view               │
└──────────────────────────────────────────────┘



### manage policy
User
 |
 v
mfwctl command
 |
 v
CLI Parser
 |
 v
CLI Transport
 |
 v
ioctl
 |
 v
Kernel Device Layer
 |
 v
Rule Service
 |
 v
Rule Store


#####  flow of packet flow(with the policy)

Incoming Packet
 |
 v
Netfilter Adapter
 |
 v
Packet Parser
 |
 v
Firewall Engine
 |
 v
Rule Store
 |
 v
Decision: PASS / DROP
 |
 v
Counters + Logs + debugfs


####

Layer 1: Shared API
-------------------
include/mfw_uapi.h

Layer 2: User Management Layer
------------------------------
user/src/main.c
user/src/cli_parser.c
user/src/mfw_client.c
user/src/printer.c

Layer 3: Kernel Control Layer
-----------------------------
kernel/src/mfw_main.c
kernel/src/mfw_device.c
kernel/src/mfw_rules.c

Layer 4: Kernel Packet Layer
----------------------------
kernel/src/mfw_netfilter.c
kernel/src/mfw_packet.c
kernel/src/mfw_engine.c

Layer 5: Observability Layer
----------------------------
kernel/src/mfw_debugfs.c
docs/
scripts/
```