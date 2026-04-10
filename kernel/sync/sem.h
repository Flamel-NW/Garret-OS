#ifndef __KERNEL_SYNC_SEM_H__
#define __KERNEL_SYNC_SEM_H__

#include "defs.h"
#include "list.h"

struct semaphore {
    i32 value;
    struct list wait_queue;
};

void init_sem(struct semaphore* sem, i32 value);
void up(struct semaphore* sem) __attribute__((noinline));
void down(struct semaphore* sem) __attribute__((noinline));
bool try_down(struct semaphore* sem);

#endif // __KERNEL_SYNC_SEM_H__
