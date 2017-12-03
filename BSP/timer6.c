#include "timer6.h"

void TIM6_TimeBaseInit(void)
{
	TIM_TimeBaseInitTypeDef TimeBaseStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	// ��ʱ����ʱ��Ƶ�� fCKCNT=TIMxCLK/(1+TIM_Prescaler)
	TimeBaseStruct.TIM_Prescaler = 719;  //100kHz
	// TIM6��TIM7ֻ֧�����ϼ���ģʽ
	TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// ������� T=TIM_Period+1
	TimeBaseStruct.TIM_Period = 499;  //5ms
	// �����˲�����Ƶϵ��������û���õ�
	TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM6,&TimeBaseStruct);
	
	// ʹ��Ԥװ�ؼĴ���
	TIM_ARRPreloadConfig(TIM6, ENABLE);
	// ʹ��TIM6�����ж�
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	// ʹ��TIM6
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
