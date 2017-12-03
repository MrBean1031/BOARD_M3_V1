#ifndef __SYSTICK_H_
#define __SYSTICK_H_

#include "stm32f10x.h"

extern __IO uint32_t nTime;
extern int SysTick_DelayFlag;

void SysTick_Init(void);
void Delay_us(uint32_t xus);
void SysTick_DelayServer(void);

#endif /*__SYSTICK_H_*/
