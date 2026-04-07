#include "schedule.h"

#include "sync.h"
#include "stride_scheduler.h"

// the list of timer
static struct list g_timer_list;

static struct scheduler* g_scheduler;

static struct run_queue* g_run_queue;

static inline void scheduler_enqueue(struct pcb* proc) {
    if (proc != g_idle_proc)
        g_scheduler->enqueue(g_run_queue, proc);
}

static inline void scheduler_dequeue(struct pcb* proc) {
    g_scheduler->dequeue(g_run_queue, proc);
}

static inline struct pcb* scheduler_pick_next() {
    return g_scheduler->pick_next(g_run_queue);
}

void scheduler_proc_tick(struct pcb* proc) {
    if (proc != g_idle_proc)
        g_scheduler->proc_tick(g_run_queue, proc);
    else
        proc->need_reschedule = true;
}

static struct run_queue g_default_run_queue;

void init_scheduler() {
    init_list(&g_timer_list);

    g_scheduler = &g_stride_scheduler;

    g_run_queue = &g_default_run_queue;
    g_run_queue->max_time_slice = MAX_TIME_SLICE;
    
    g_scheduler->init(g_run_queue);

    putstr("scheduler: "); putstr(g_scheduler->name); putch('\n');
}


void wakeup_proc(struct pcb* proc) {
    ASSERT(proc->state != PROC_ZOMBIE);
    bool intr_flag = local_intr_save();
    {
        if (proc->state != PROC_RUNNABLE) {
            proc->state = PROC_RUNNABLE;
            proc->wait_state = WAIT_NONE;
            if (proc != g_curr_proc)
                scheduler_enqueue(proc);
        } else {
            WARN("wakeup runnable process.\n");
        }
    }
    local_intr_restore(intr_flag);
}

void schedule() {
    bool intr_flag = local_intr_save();
    {
        g_curr_proc->need_reschedule = false;
        if (g_curr_proc->state == PROC_RUNNABLE)
            scheduler_enqueue(g_curr_proc);
        struct pcb* next_proc;
        if ((next_proc = scheduler_pick_next()))
            scheduler_dequeue(next_proc);
        else
            next_proc = g_idle_proc;

        next_proc->run_times++;
        if (next_proc != g_curr_proc)
            run_proc(next_proc);
    }
    local_intr_restore(intr_flag);
}
