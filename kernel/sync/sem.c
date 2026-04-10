#include "sem.h"

#include "schedule.h"
#include "sync.h"
#include "wait.h"


void init_sem(struct semaphore* sem, i32 value) {
    sem->value = value;
    init_list(&(sem->wait_queue));
}

void __attribute__((noinline)) up(struct semaphore* sem) {
    bool intr_flag = local_intr_save();
    {
        struct wait* wait = wait_queue_first(&(sem->wait_queue));
        if (!wait) {
            sem->value++;
        } else {
            ASSERT(wait->proc->wait_state == WAIT_SEM);
            wakeup_wait(&(sem->wait_queue), wait, WAIT_SEM, true);
        }
    }
    local_intr_restore(intr_flag);
}

void __attribute__((noinline)) down(struct semaphore* sem) {
    struct wait wait;
    bool intr_flag = local_intr_save();
    {
        if (sem->value > 0) {
            sem->value--;
            local_intr_restore(intr_flag);
            return;
        }
        add_curr_wait(&(sem->wait_queue), &wait, WAIT_SEM);
    }
    local_intr_restore(intr_flag);

    schedule();

    intr_flag = local_intr_save();
    {
        del_curr_wait(&(sem->wait_queue), &wait);
    }
    local_intr_restore(intr_flag);

    ASSERT(wait.wakeup_state == WAIT_SEM);
}

bool try_down(struct semaphore* sem) {
    bool ret = false;
    bool intr_flag = local_intr_save();
    {
        if (sem->value > 0) {
            sem->value--;
            ret = true;
        }
    }
    local_intr_restore(intr_flag);
    return ret;
}
