/* mfw_debugfs.h - skeleton */

#ifndef MFW_DEBUGFS_H
#define MFW_DEBUGFS_H

/*
 * Initialize debugfs entries.
 *
 * Creates:
 * /sys/kernel/debug/mfw/
 * /sys/kernel/debug/mfw/rules
 */
int mfw_debugfs_init(void);

/*
 * Remove debugfs entries.
 */
void mfw_debugfs_exit(void);

#endif /* MFW_DEBUGFS_H */