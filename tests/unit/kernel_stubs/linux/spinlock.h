#ifndef TEST_KERNEL_STUB_LINUX_SPINLOCK_H
#define TEST_KERNEL_STUB_LINUX_SPINLOCK_H

/*
 * User-space unit-test stub for Linux spinlocks.
 *
 * In the real kernel, spinlocks protect shared data from concurrent access.
 * In these unit tests, we compile kernel code with normal gcc in user-space,
 * so these functions/macros only need to exist and behave as no-ops.
 */

typedef int spinlock_t;

#define spin_lock_init(lock)        \
    do {                            \
        *(lock) = 0;                \
    } while (0)

#define spin_lock(lock)             \
    do {                            \
        (void)(lock);               \
    } while (0)

#define spin_unlock(lock)           \
    do {                            \
        (void)(lock);               \
    } while (0)

/*
 * In the real kernel:
 * spin_lock_irqsave(lock, flags)
 *   - saves interrupt state into flags
 *   - disables interrupts
 *   - locks the spinlock
 *
 * In the unit test:
 *   - there are no kernel interrupts
 *   - there is no real concurrency
 *   - so we only silence unused warnings and set flags to 0
 */
#define spin_lock_irqsave(lock, flags)      \
    do {                                    \
        (void)(lock);                       \
        (flags) = 0;                        \
    } while (0)

/*
 * In the real kernel:
 * spin_unlock_irqrestore(lock, flags)
 *   - unlocks the spinlock
 *   - restores the previous interrupt state
 *
 * In the unit test this is just a no-op.
 */
#define spin_unlock_irqrestore(lock, flags) \
    do {                                    \
        (void)(lock);                       \
        (void)(flags);                      \
    } while (0)

#endif /* TEST_KERNEL_STUB_LINUX_SPINLOCK_H */