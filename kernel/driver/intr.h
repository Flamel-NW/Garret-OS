#ifndef __KERNEL_DRIVER_INTR_H__
#define __KERNEL_DRIVER_INTR_H__

#include "defs.h"
#include "riscv.h"

// enable irq interrupt
static inline void intr_enable() {
    CSRRS(sstatus, SSTATUS_SIE);
}

// disable irq interrupt
static inline void intr_disable() {
    CSRRC(sstatus, SSTATUS_SIE);
}

static inline bool local_intr_save() {
    if (CSRR(sstatus) & SSTATUS_SIE) {
        return 1;
    }
    return 0;
}

static inline void local_intr_restore(bool flag) {
    if (flag) intr_enable();
}

#endif // __KERNEL_DRIVER_INTR_H__
