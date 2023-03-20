#ifndef __KERNEL_DRIVER_INTR_H__
#define __KERNEL_DRIVER_INTR_H__

#include "riscv.h"

// enable irq interrupt
static inline void intr_enable() {
    CSRRS(sstatus, SSTATUS_SIE);
}

// disable irq interrupt
static inline void intr_disable() {
    CSRRC(sstatus, SSTATUS_SIE);
}

#endif // __KERNEL_DRIVER_INTR_H__
