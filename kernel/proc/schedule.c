#include "schedule.h"

#include "intr.h"
#include "sync.h"


void wakeup_proc(struct pcb* proc) {
    ASSERT(proc->state != PROC_ZOMBIE);
    bool intr_flag = local_intr_save();
    {
        if (proc->state != PROC_RUNNABLE) {
            proc->state = PROC_RUNNABLE;
            proc->wait_state = WAIT_NONE;
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
        struct pcb* proc = NULL;
        struct list* list = (g_curr_proc == g_idle_proc) ? &g_proc_list : &g_curr_proc->list_link;
        struct list* iter = list;
        do {
            if ((iter = iter->next) != &g_proc_list) {
                proc = LIST2PCB(list_link, iter);
                if (proc->state == PROC_RUNNABLE)
                    break;
            }
        } while (iter != list);

        if (!proc || proc->state != PROC_RUNNABLE)
            proc = g_idle_proc;
        
        proc->run_times++;
        if (proc != g_curr_proc)
            run_proc(proc);
    }
    local_intr_restore(intr_flag);
}
