#ifndef __DELAY_TIM_
#define __DELAY_TIM_

#include "stm32f10x.h"


#define MEASURE_STATE_WORKING 0x01
#define MEASURE_STATE_STOP    0x02

/* Exported Variables Declaration */
extern uint32_t timerticks;
extern uint8_t MeasureState;  //测量状态机
extern uint8_t MeasureCnt;    //测量计数

/* Exported Funciton Prototypes */
void TIM7_TimeBase_Config(void);
void Delay_us(uint32_t xus);
uint32_t TimeMeasure(void);

#endif /* __DELAY_TIM_ */
