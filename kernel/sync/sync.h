#ifndef __KERNEL_SYNC_SYNC_H__
#define __KERNEL_SYNC_SYNC_H__

#include "riscv.h"
#include "intr.h"


static inline bool local_intr_save() {
    if (CSRR(sstatus) & SSTATUS_SIE) {
        intr_disable();
        return true;
    }
    return false;
}

static inline void local_intr_restore(bool flag) {
    if (flag) intr_enable();
}

#endif // __KERNEL_SYNC_SYNC_H__
