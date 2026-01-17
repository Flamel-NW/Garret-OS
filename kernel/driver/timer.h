#ifndef __KERNEL_DRIVER_TIMER_H__
#define __KERNEL_DRIVER_TIMER_H__

#include "defs.h"

extern volatile u64 g_ticks;

void init_timer();
void timer_next();

#endif // __KERNEL_DRIVER_TIMER_H__
