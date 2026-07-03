# Development Stages

1. include/mfw_uapi.h

2. kernel/src/mfw_main.c
   kernel/Makefile

3. kernel/src/mfw_device.c
   kernel/include/mfw_device.h

4. user/src/main.c
   user/src/mfw_client.c
   user/Makefile

5. user/src/cli_parser.c
   user/src/printer.c

6. kernel/src/mfw_rules.c

7. kernel/src/mfw_netfilter.c

8. kernel/src/mfw_packet.c

9. kernel/src/mfw_engine.c

10. kernel/src/mfw_debugfs.c

11. scripts/

12. docs/

Stage 1:
Build system + kernel module skeleton

Stage 2:
Create /dev/mfw and simple ioctl

Stage 3:
Build user CLI that sends ioctl

Stage 4:
Add rule table in kernel

Stage 5:
Implement add / del / list / clear

Stage 6:
Add Netfilter hook in LOG-only mode

Stage 7:
Connect rule matching to packet DROP

Stage 8:
Add counters, dmesg logs, debugfs

Stage 9:
Test with two VMs

Stage 10:
Write README + interview explanation