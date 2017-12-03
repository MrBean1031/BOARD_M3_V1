#ifndef __DELAY_TIM_
#define __DELAY_TIM_

#include "stm32f10x.h"


#define MEASURE_STATE_WORKING 0x01
#define MEASURE_STATE_STOP    0x02

/* Exported Variables Declaration */


/* Exported Funciton Prototypes */
void TIM6_TimeBase_Config(void);
void Delay_us(uint32_t xus);
uint32_t TimeMeasure(void);

#endif /* __DELAY_TIM_ */
