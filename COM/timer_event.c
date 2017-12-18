#include "timer_event.h"

#ifndef NULL
#define NULL  0
#endif


#if PROT_METHOD == PROT_METHOD_MUTEX 
#include "includes.h"
typedef OS_EVENT *prot_t;

#elif PROT_METHOD == PROT_METHOD_CRITICAL
#include "includes.h"
#if OS_CRITICAL_METHOD == 3
typedef OS_CPU_SR prot_t;
#define pt  cpu_sr
#else
typedef u8 prot_t;
#endif  //OS_CRITICAL_METHOD

#else
typedef u8 prot_t;

#endif

static prot_t pt;  //线程互斥保护

struct timer_event_node {
  int id;
  struct timer_event_node *next;
  void *arg;
  void (*func)(void *arg);
  u32 msec;
  u8 del;
};
static struct timer_event_node *free_list_head, *using_list_head;


#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
static struct timer_event_node timer_event_array[TIMER_EVENT_ARRAY_SIZE];
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
#include "stdlib.h"
#else
#error NODE_MALLOC_METHOD invalid
#endif

//in_irq和stash_list_head是用于中断内timer_event_schedule()调用timer_event_add()添加节点的缓存链表头
u8 in_irq;
static struct timer_event_node *stash_list_head;


static prot_t os_protect_init(void)
{
#if PROT_METHOD == PROT_METHOD_MUTEX
  u8 err;
  prot_t t;
  t = OSMutexCreate(TIMER_EVENT_MUTEX_PRIO, &err);
  if (err == OS_ERR_NONE) {
    return t;
  }
  else {
    return 0;
  }
#elif PROT_METHOD == PROT_METHOD_CRITICAL
  return 0;
#else
  return 0;
#endif
}

static int os_protect_pend(prot_t t)
{
#if PROT_METHOD == PROT_METHOD_MUTEX
  u8 err;
  OSMutexPend(t, 0, &err);
  if(err != OS_ERR_NONE) {
    return err;
  }
  else {
    return 0;
  }
#elif PROT_METHOD == PROT_METHOD_CRITICAL
  OS_ENTER_CRITICAL();
  return 0;
#else
  return 0;
#endif
}

static int os_protect_post(prot_t t)
{
#if PROT_METHOD == PROT_METHOD_MUTEX
  u8 err;
  err = OSMutexPost(t);
  if(err == OS_ERR_NONE)
    return 0;
  else
    return err;
#elif PROT_METHOD == PROT_METHOD_CRITICAL
  OS_EXIT_CRITICAL();
  return 0;
#else
  return 0;
#endif
}

void timer_event_init(void)
{
  int i;
  using_list_head = NULL;
  stash_list_head = NULL;
  in_irq = 0;  //不在中断
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
  for(i = 0; 
      i < sizeof(timer_event_array) / sizeof(timer_event_array[0]) - 1;
      i++) {
    timer_event_array[i].next = &timer_event_array[i + 1];
  }
  timer_event_array[i].next = NULL;
  free_list_head = &timer_event_array[0];  
#else
  free_list_head = NULL;
#endif
  pt = os_protect_init();
}

int timer_event_add(void *arg, void (*func)(void *arg), u32 msec)
{
  struct timer_event_node *p;
  
  if (!func || !msec) {
    return 0;
  }
  os_protect_pend(pt);
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
  if(free_list_head == NULL) {
    os_protect_post(pt);
    return 0;
  }
  p = free_list_head;
  free_list_head = p->next;
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
  p = (struct timer_event_node *)malloc(sizeof(struct timer_event_node));
  if(p == NULL) {
    os_protect_post(pt);
    return 0;
  }
#endif
  p->arg = arg;
  p->func = func;
  p->msec = msec;
  p->del = 0;
  p->id = (int)p;  //节点的唯一标志

  if (in_irq) {
    p->next = stash_list_head;
    stash_list_head = p;
  } else {
    p->next = using_list_head;
    using_list_head = p;
  }
  os_protect_post(pt);
  return p->id;
}

void timer_event_del(int id)
{
  struct timer_event_node *p;

  os_protect_pend(pt);
  for(p = using_list_head; p != NULL; p = p->next) {
    if(p->id == id) {
      p->del = 1;
      break;
    }
  }
  for(p = stash_list_head; p != NULL; p = p->next) {
    if(p->id == id) {
      p->del = 1;
      break;
    }
  }
  os_protect_post(pt);
}

void timer_event_schedule(u32 ms_unit)
{
  struct timer_event_node *p, *q;

  in_irq = 1;
  p = using_list_head;
  while(p != NULL) {
    //处理timer_event_del()的删除指令
    if(p->del) {
      if(p == using_list_head) {
        using_list_head = p->next;  
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
        p->next = free_list_head;
        free_list_head = p;
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
        free(p);
#endif
        p = using_list_head;
      } else {
        q->next = p->next;
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
        p->next = free_list_head;
        free_list_head = p;
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
        free(p);
#endif
        p = q->next;
      }
    }
    //轮询节点，当msec减到0时删除该节点，并执行回调函数
    p->msec = p->msec > ms_unit ? p->msec - ms_unit : 0;
    if (!p->msec) {
      if(p->func) {
        p->func(p->arg);
      }
      if(p == using_list_head) {
        using_list_head = p->next;
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
        p->next = free_list_head;
        free_list_head = p;
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
        free(p);
#endif
        p = using_list_head;
      } else {
        q->next = p->next;
#if NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_STATIC
        p->next = free_list_head;
        free_list_head = p;
#elif NODE_MALLOC_METHOD == NODE_MALLOC_METHOD_HEAP
        free(p);
#endif
        p = q->next;
      }
    } else {
      q = p;
      p = p->next;
    }
  }
  //将stash_list内缓存的节点加进using_list
  while(stash_list_head != NULL) {
    p = stash_list_head;
    stash_list_head = stash_list_head->next;
    p->next = using_list_head;
    using_list_head = p;
  }
  in_irq = 0;
}


