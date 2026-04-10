#include "wait.h"

#include "schedule.h"


void init_wait(struct wait* wait, struct pcb* proc) {
    wait->proc = proc;
    wait->wakeup_state = WAIT_INT;
    init_list(&(wait->wait_link));
}

void init_wait_queue(struct list* wait_queue) {
    init_list(wait_queue);
}

void add_wait_queue(struct list* wait_queue, struct wait* wait) {
    ASSERT(list_empty(&(wait->wait_link)) && wait->proc != NULL);
    wait->wait_queue = wait_queue;
    add_list(wait_queue, wait_queue->prev, &(wait->wait_link));
}

void del_wait_queue(struct list* wait_queue, struct wait* wait) {
    ASSERT(!list_empty(&(wait->wait_link)) && wait->wait_queue == wait_queue);
    del_list(&(wait->wait_link));
    init_list(&(wait->wait_link));
}

struct wait* wait_queue_next(struct list* wait_queue, struct wait* wait) {
    ASSERT(!list_empty(&(wait->wait_link)) && wait->wait_queue == wait_queue);
    struct list* list = wait->wait_link.next;
    if (list != wait_queue)
        return LIST2WAIT(wait_link, list);
    return NULL;
}

struct wait* wait_queue_prev(struct list* wait_queue, struct wait* wait) {
    ASSERT(!list_empty(&(wait->wait_link)) && wait->wait_queue == wait_queue);
    struct list* list = wait->wait_link.prev;
    if (list != wait_queue)
        return LIST2WAIT(wait_link, list);
    return NULL;
}

struct wait* wait_queue_first(struct list* wait_queue) {
    struct list* list = wait_queue->next;
    if (list != wait_queue)
        return LIST2WAIT(wait_link, list);
    return NULL;
}

struct wait* wait_queue_last(struct list* wait_queue) {
    struct list* list = wait_queue->prev;
    if (list != wait_queue)
        return LIST2WAIT(wait_link, list);
    return NULL;
}

bool wait_queue_empty(struct list* wait_queue) {
    return list_empty(wait_queue);
}

void wakeup_wait(struct list* wait_queue, struct wait* wait, u32 wakeup_state, bool del) {
    if (del)
        del_wait_queue(wait_queue, wait);
    wait->wakeup_state = wakeup_state;
    wakeup_proc(wait->proc);
}

void wakeup_first(struct list* wait_queue, u32 wakeup_state, bool del) {
    struct wait* wait = wait_queue_first(wait_queue);
    if (wait)
        wakeup_wait(wait_queue, wait, wakeup_state, del);
}

void wakeup_queue(struct list* wait_queue, u32 wakeup_state, bool del) {
    struct wait* wait;
    struct wait* next_wait;
    for (wait = wait_queue_first(wait_queue) ; wait; wait = next_wait) {
        next_wait = del ? wait_queue_first(wait_queue) : wait_queue_next(wait_queue, wait);
        wakeup_wait(wait_queue, wait, wakeup_state, del);
    }
}

void add_curr_wait(struct list* wait_queue, struct wait* wait, u32 wait_state) {
    ASSERT(g_curr_proc != NULL);
    init_wait(wait, g_curr_proc);
    g_curr_proc->state = PROC_SLEEPING;
    g_curr_proc->wait_state = wait_state;
    add_wait_queue(wait_queue, wait);
}

void del_curr_wait(struct list* wait_queue, struct wait* wait) {
    if (!list_empty(&(wait->wait_link)))
        del_wait_queue(wait_queue, wait);
}

