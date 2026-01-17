#ifndef __KERNEL_PROC_SCHED_H__
#define __KERNEL_PROC_SCHED_H__

#include "proc.h"


void wakeup_proc(struct pcb *proc);

void schedule();

#endif // __KERNEL_PROC_SCHED_H__
