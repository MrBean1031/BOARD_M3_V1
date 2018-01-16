#ifndef __DELAY_TIM_
#define __DELAY_TIM_

#include "stm32f10x.h"



/* Exported Variables Declaration */

/* Exported Funciton Prototypes */
void TIM7_TimeBase_Config(void);
void Delay_us(uint32_t xus);
uint32_t TimeMeasure(void);
void TIM7_IRQServer(void);

#endif /* __DELAY_TIM_ */
