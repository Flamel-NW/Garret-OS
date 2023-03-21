#ifndef __KERNEL_DRIVER_TIMER_H__
#define __KERNEL_DRIVER_TIMER_H__

#include "defs.h"

extern volatile size_t ticks;

void timer_init();
void timer_next();

#endif
