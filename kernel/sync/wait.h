#ifndef __KERNEL_SYNC_WAIT_H__
#define __KERNEL_SYNC_WAIT_H__

#include "list.h"
#include "proc.h"

struct wait {
    struct pcb* proc;
    u32 wakeup_state;
    struct list* wait_queue;
    struct list wait_link;
};

#define LIST2WAIT(member, list) \
    TO_STRUCT(struct wait, member, (list))

void init_wait(struct wait* wait, struct pcb* proc);
void init_wait_queue(struct list* wait_queue);
void add_wait_queue(struct list* wait_queue, struct wait* wait);
void del_wait_queue(struct list* wait_queue, struct wait* wait);

struct wait* wait_queue_next(struct list* wait_queue, struct wait* wait);
struct wait* wait_queue_prev(struct list* wait_queue, struct wait* wait);
struct wait* wait_queue_first(struct list* wait_queue);
struct wait* wait_queue_last(struct list* wait_queue);

bool wait_queue_empty(struct list* wait_queue);

void wakeup_wait(struct list* wait_queue, struct wait* wait, u32 wakeup_state, bool del);
void wakeup_first(struct list* wait_queue, u32 wakeup_state, bool del);
void wakeup_queue(struct list* wait_queue, u32 wakeup_state, bool del);

void add_curr_wait(struct list* wait_queue, struct wait* wait, u32 wait_state);
void del_curr_wait(struct list* wait_queue, struct wait* wait);


#endif // __KERNEL_SYNC_WAIT_H__
