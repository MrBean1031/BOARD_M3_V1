#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include "stm32f10x.h"
#include <stdio.h>
#include "ff.h"

#define MEMORY_ALLOC_TASK_PRIO 1

void *malloc_safe(size_t);
void *calloc_safe(size_t, size_t);
void free_safe(void *);

/*-- file system --*/
extern FATFS fs_sd;
extern FATFS fs_flash;

#endif //__GLOBAL_H_
