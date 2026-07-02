# Interview Notes

## Concepts covered

### Kernel module

You build an out-of-tree Linux kernel module and load it with `insmod`.

### Netfilter hook

The module registers a hook in the IPv4 packet path. This project uses `NF_INET_LOCAL_IN`, meaning it sees packets destined for the local machine.

### User/kernel communication

The CLI talks to the kernel module through `/dev/mfw` using `ioctl`.

### Rule table

Rules are stored inside the kernel module in a fixed-size array. This is simple and predictable for a first version.

### Synchronization

The rule table is protected with a spinlock because the packet hook cannot sleep.

### Counters

Each rule has a hit counter. When a packet matches, the counter increases.

### debugfs

The module exposes human-readable internal state at `/sys/kernel/debug/mfw/rules`.

### Debugging

Use `dmesg`, `strace`, `tcpdump`, `gdb`, and `modinfo`.

## Good interview explanation

"I built a small Linux firewall framework as an educational systems project. The kernel module registers a Netfilter hook at the IPv4 LOCAL_IN path, keeps an in-kernel rule table, and exposes a character-device ioctl interface to a user-space CLI. The CLI can add, delete, list, and clear source-IP rules. The packet path checks source IPs, updates counters, and can drop matching packets. I exposed debug state with debugfs and used dmesg, strace, and tcpdump for debugging. The first version is intentionally simple, and next steps would be CIDR, port matching, Netlink, RCU, per-CPU counters, and stateful connection tracking."
