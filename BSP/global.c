#include "global.h"
#include "stdlib.h"
#include "includes.h"

#ifdef OS_uCOS_II_H
OS_EVENT *memory_alloc = 0;
#endif

void *malloc_safe(size_t size)
{
  u8 err;
  void *p;
#ifdef OS_uCOS_II_H
  if(!memory_alloc) {
    memory_alloc = OSMutexCreate(MEMORY_ALLOC_TASK_PRIO, &err);
    if (!memory_alloc || err != OS_ERR_NONE) {
      return 0;
    }
  }
  OSMutexPend(memory_alloc, 0, &err);
#endif
  p = malloc(size);
#ifdef OS_uCOS_II_H
  OSMutexPost(memory_alloc);
#endif
  return p;
}

void *calloc_safe(size_t number, size_t size)
{
  u8 err;
  void *p;
#ifdef OS_uCOS_II_H
  if(!memory_alloc) {
    memory_alloc = OSMutexCreate(MEMORY_ALLOC_TASK_PRIO, &err);
    if (!memory_alloc || err != OS_ERR_NONE) {
      return 0;
    }
  }
  OSMutexPend(memory_alloc, 0, &err);
#endif
  p = calloc(number, size);
#ifdef OS_uCOS_II_H
  OSMutexPost(memory_alloc);
#endif
  return p;
}

void free_safe(void *p)
{
  u8 err;
#ifdef OS_uCOS_II_H
  if(!memory_alloc) {
    memory_alloc = OSMutexCreate(MEMORY_ALLOC_TASK_PRIO, &err);
    if (!memory_alloc || err != OS_ERR_NONE) {
      return;
    }
  }
  OSMutexPend(memory_alloc, 0, &err);
#endif
  free(p);
#ifdef OS_uCOS_II_H
  OSMutexPost(memory_alloc);
#endif
  return;
}
