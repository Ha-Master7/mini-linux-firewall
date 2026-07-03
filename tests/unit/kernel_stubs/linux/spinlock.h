#ifndef TEST_KERNEL_STUB_LINUX_SPINLOCK_H
#define TEST_KERNEL_STUB_LINUX_SPINLOCK_H

typedef int spinlock_t;

#define spin_lock_init(lock) do { *(lock) = 0; } while (0)
#define spin_lock(lock) do { (void)(lock); } while (0)
#define spin_unlock(lock) do { (void)(lock); } while (0)

#endif
