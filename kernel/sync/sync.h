#ifndef __KERNEL_SYNC_SYNC_H__
#define __KERNEL_SYNC_SYNC_H__

#include "riscv.h"
#include "intr.h"
#include "atomic.h"
#include "schedule.h"


static inline bool local_intr_save() {
    if (CSRR(sstatus) & SSTATUS_SIE) {
        return true;
    }
    return false;
}

static inline void local_intr_restore(bool flag) {
    if (flag) intr_enable();
}

// amoor.d (Atomic Memory Operation Double-word)
// 这条指令强制要求操作 64 位 (8 字节) 的数据。
// 这条指令强制要求内存地址必须 8 字节对齐。
typedef volatile u64 lock_t;

static inline void init_lock(lock_t* lock) {
    *lock = 0;
}

static inline bool try_lock(lock_t* lock) {
    return set_bit((void*) lock, 0);
} 

static inline void lock(lock_t* lock) {
    while (!try_lock(lock))
        schedule();
}

static inline void unlock(lock_t* lock) {
    if (!clear_bit((void*) lock, 0))
        PANIC("Unlock failed.");
}

#endif // __KERNEL_SYNC_SYNC_H__
