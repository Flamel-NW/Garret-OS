#ifndef __KERNEL_DRIVER_TIMER_H__
#define __KERNEL_DRIVER_TIMER_H__

#include "defs.h"

extern volatile size_t g_ticks;

void init_timer();
void timer_next();

#endif
