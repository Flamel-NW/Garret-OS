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
        struct pcb* next_proc = scheduler_pick_next();
        if (next_proc)
            scheduler_dequeue(next_proc);
        else
            next_proc = g_idle_proc;

        next_proc->run_times++;
        if (next_proc != g_curr_proc)
            run_proc(next_proc);
    }
    local_intr_restore(intr_flag);
}

// add timer to g_timer_list
void add_timer(struct timer *timer) {
    bool intr_flag = local_intr_save();
    {
        ASSERT(timer->expires > 0 && timer->proc != NULL);
        ASSERT(list_empty(&(timer->timer_link)));
        struct list* list = g_timer_list.next;
        while (list != &g_timer_list) {
            struct timer* next_timer = LIST2TIMER(timer_link, list);
            if (timer->expires < next_timer->expires) {
                next_timer->expires -= timer->expires;
                break;
            }
            timer->expires -= next_timer->expires;
            list = list->next;
        }
        add_list(list->prev, list, &(timer->timer_link));
    }
    local_intr_restore(intr_flag);
}

// del timer from g_timer_list
void del_timer(struct timer* timer) {
    bool intr_flag = local_intr_save();
    {
        if (!list_empty(&(timer->timer_link))) {
            if (timer->expires) {
                struct list* list = timer->timer_link.next;
                if (list != &g_timer_list) {
                    struct timer* next_timer = LIST2TIMER(timer_link, list);
                    next_timer->expires += timer->expires;
                }
            }
            del_list(&(timer->timer_link));
            init_list(&(timer->timer_link));
        }
    }
    local_intr_restore(intr_flag);
}

// call scheduler to update tick related info, and check the timer is expired? if expired, then wakeup proc
void run_timers() {
    bool intr_flag = local_intr_save();
    {
        struct list* list = g_timer_list.next;
        if (list != &g_timer_list) {
            struct timer* timer = LIST2TIMER(timer_link, list);
            ASSERT(timer->expires != 0);
            timer->expires--;
            while (!timer->expires) {
                list = list->next;
                struct pcb* proc = timer->proc;
                if (proc->wait_state) {
                    ASSERT(proc->wait_state & WAIT_INT);
                }
                else {
                    putstr("pid = "); put_i64(proc->pid, 10); putch('\n');
                    WARN("process's wait_state == 0.\n");
                }
                wakeup_proc(proc);
                del_timer(timer);
                if (list == &g_timer_list)
                    break;
                timer = LIST2TIMER(timer_link, list);
            }
        }
        scheduler_proc_tick(g_curr_proc);
    }
    local_intr_restore(intr_flag);
}
