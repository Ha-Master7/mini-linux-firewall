/* mfw_device.h - skeleton */

#ifndef MFW_DEVICE_H
#define MFW_DEVICE_H

/*
 * Initialize the /dev/mfw device.
 *
 * This creates a character device that user-space can open
 * and communicate with using ioctl.
 */
int mfw_device_init(void);

/*
 * Remove the /dev/mfw device.
 *
 * Called when the kernel module is unloaded.
 */
void mfw_device_exit(void);

#endif /* MFW_DEVICE_H */
