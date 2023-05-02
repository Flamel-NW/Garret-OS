#ifndef __KERNEL_PROC_SCHED_H__
#define __KERNEL_PROC_SCHED_H__

#include "proc.h"

static inline void wakeup_proc(struct pcb* proc) {
    ASSERT(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE;
}

void schedule();

#endif // __KERNEL_PROC_SCHED_H__
