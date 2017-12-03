#include "timer6.h"

void TIM6_TimeBaseInit(void)
{
	TIM_TimeBaseInitTypeDef TimeBaseStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	// 定时器的时钟频率 fCKCNT=TIMxCLK/(1+TIM_Prescaler)
	TimeBaseStruct.TIM_Prescaler = 719;  //100kHz
	// TIM6，TIM7只支持向上计数模式
	TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// 溢出周期 T=TIM_Period+1
	TimeBaseStruct.TIM_Period = 499;  //5ms
	// 输入滤波器分频系数，这里没有用到
	TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM6,&TimeBaseStruct);
	
	// 使能预装载寄存器
	TIM_ARRPreloadConfig(TIM6, ENABLE);
	// 使能TIM6更新中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	// 使能TIM6
	TIM_Cmd(TIM6, ENABLE);
}

/* example codes

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update)==SET)
	{
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);

	}
}
*/
