#ifndef __TIMER_EVENT_H_
#define __TIMER_EVENT_H_

#include "stm32f10x.h"

#define PROT_METHOD_MUTEX     1
#define PROT_METHOD_CRITICAL  2
#define PROT_METHOD_NONE      0xff
#define PROT_METHOD           PROT_METHOD_NONE

#if PROT_METHOD == PROT_METHOD_MUTEX
#define TIMER_EVENT_MUTEX_PRIO    1
#endif

#define NODE_MALLOC_METHOD_STATIC     0
#define NODE_MALLOC_METHOD_HEAP       1 
#define NODE_MALLOC_METHOD            NODE_MALLOC_METHOD_STATIC

#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
#define TIMER_EVENT_ARRAY_SIZE  10
#endif

void timer_event_init(void);
int timer_event_add(void *arg, void (*func)(void *arg), u32 msec);
void timer_event_del(int id);
void timer_event_schedule(u32 ms_unit);

#endif //__TIMER_EVENT_H_
