#include "monitor.h"

#include "assert.h"
#include "pmm.h"

// Initialize variables in monitor.
void init_monitor(struct monitor* monitor, u64 num_cond) {
    ASSERT(num_cond > 0);
    monitor->next_count = 0;
    monitor->cond = NULL;
    init_sem(&(monitor->mutex), 1); // unlocked
    init_sem(&(monitor->next), 0);
    monitor->cond = (struct cond*) malloc(sizeof(struct cond) * num_cond);
    ASSERT(monitor->cond != NULL);
    for (i32 i = 0; i < num_cond; i++) {
        monitor->cond[i].count = 0;
        init_sem(&(monitor->cond[i].sem), 0);
        monitor->cond[i].owner = monitor;
    }
}

// Unlock one of threads waiting on the cond var
void signal_cond(struct cond* cond) {
    if (cond->count > 0) {
        struct monitor* monitor = cond->owner;
        monitor->next_count++;
        up(&(cond->sem));
        down(&(monitor->next));
        monitor->next_count--;
    }
}

// Suspend calling thread on a condition variable waiting for condition atomically unlock mutex in monitor,
// and suspends calling threads on cond var after waking up locks mutex
void wait_cond(struct cond* cond) {
    cond->count++;
    struct monitor* monitor = cond->owner;
    if (monitor->next_count > 0)
        up(&(monitor->next));
    else
        up(&(monitor->mutex));
    down(&(cond->sem));
    cond->count--;
}
