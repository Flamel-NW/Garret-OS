#include "sched.h"

#include "intr.h"

void schedule() {
    bool intr_flag = local_intr_save();
    {
        struct pcb* proc = NULL;
        struct list* list = &g_curr_proc->list_link;
        do {
            if ((list = list->next) != &g_proc_list) {
                proc = LIST2PCB(list_link, list);
                if (proc->state == PROC_RUNNABLE)
                    break;
            }
        } while (list != &g_curr_proc->list_link);

        if (!proc || proc->state != PROC_RUNNABLE)
            proc = g_idle_proc;
        
        proc->run_times++;
        if (proc != g_curr_proc)
            run_proc(proc);
    }
    local_intr_restore(intr_flag);
}
