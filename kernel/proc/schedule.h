#ifndef __KERNEL_PROC_SCHEDULE_H__
#define __KERNEL_PROC_SCHEDULE_H__

#include "proc.h"

#define MAX_TIME_SLICE 5

struct run_queue {
    struct list run_list;
    u32 num_proc;
    i32 max_time_slice;
    struct skew_heap* run_pool;
};

// The introduction of scheduling classes is borrowed from linux, and makes the
// core scheduler quite extensible. These classes (the cheduler modules) encapsulate
// the scheduling policies
struct scheduler {
    // the name of scheduler
    const char* name;

    // Init the run queue
    void (*init) (struct run_queue* run_queue);
    
    // put the proc into run queue, and this function must be called with rq_lock
    void (*enqueue) (struct run_queue* run_queue, struct pcb* proc);

    // get the proc out run queue, and this function must be called with rq_lock
    void (*dequeue) (struct run_queue*rq, struct pcb* proc);

    // choose the next runnable task
    struct pcb* (*pick_next) (struct run_queue* run_queue);

    // dealer of the time-tick
    void (*proc_tick) (struct run_queue* run_queue, struct pcb* proc);
};

void init_scheduler();
void scheduler_proc_tick(struct pcb* proc);

void wakeup_proc(struct pcb *proc);

void schedule();

#endif // __KERNEL_PROC_SCHEDULE_H__
