/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __SYS_C64_H__
#define __SYS_C64_H__

#include "includes.h"

#define SYS_MBOX_NULL       (sys_mbox_t)0
#define SYS_SEM_NULL        (sys_sem_t)0
#define TOTAL_MBOX_NUM      10
#define MAX_MSG_IN_MBOX     10
#define MAX_LWIP_THREADS    2
#define THREAD_STK_SIZE     128


struct sys_mbox {
  OS_EVENT *queue;
  void *MsgBox[MAX_MSG_IN_MBOX];
};

typedef OS_EVENT *sys_sem_t;
typedef struct sys_mbox *sys_mbox_t;
typedef INT8U sys_thread_t;

/* LwIP critical region protect when SYS_LIGHTWEIGHT_PROT==1 */
typedef OS_CPU_SR sys_prot_t;
#if OS_CRITICAL_METHOD == 3
#define sys_arch_protect(x)     OS_CPU_SR_Save(x)
#define sys_arch_unprotect(x)   OS_CPU_SR_Restore(x)   
#else
#define sys_arch_protect(x)
#define sys_arch_unprotect(x)
#endif /* CRITICAL_METHOD == 3 */


#endif /* __SYS_C64_H__ */
