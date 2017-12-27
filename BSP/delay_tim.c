/*
 *File Description: �����������ʱ����ʱ���������
 *Pin Mapping:      None
 *Author:           Mr.Bean
 *Date:             2017/2/16
 *Attention:        1.�õ���ʱ��7��Update�жϣ���Ҫ��дNVIC���������ж����ȼ���ʹ���ж�
                    2.��Ҫ��д�жϷ����ӳ���ISR��
										3.TimeMeasure()��Ϊ��ʱ������ľ��ȣ�û��ʹ������ͬ�����ƣ�ע�ⲻҪ����
 */
 
#include "delay_tim.h"
#include "includes.h"


uint32_t timerticks;
uint8_t MeasureState;  //����״̬��
uint8_t MeasureCnt;    //��������

/*-----------------------------------------------------
 - Function Name: TIM6_TimeBase_Config()
 - Description:   ʹ�ö�ʱ��6���ʱ��Ϊ10us����ʱ����ʱ���������
 - Input:         None
 - Output:        None
 - Return:        None
 - Attention:     None
-----------------------------------------------------*/
void TIM7_TimeBase_Config(void)
{
	TIM_TimeBaseInitTypeDef TimeBaseStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	// ��ʱ����ʱ��Ƶ�� fCKCNT=TIMxCLK/(1+TIM_Prescaler)
	TimeBaseStruct.TIM_Prescaler = 0;
	// TIM6��TIM7ֻ֧�����ϼ���ģʽ
	TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// ������� T=TIM_Period+1��10us
	TimeBaseStruct.TIM_Period = 719;
	// �����˲�����Ƶϵ��������û���õ�
	TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM7,&TimeBaseStruct);
	
	// ʹ��Ԥװ�ؼĴ���
	TIM_ARRPreloadConfig(TIM7, ENABLE);
	// ʹ��TIM7�����ж�
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	// ʹ��TIM7
	TIM_Cmd(TIM7, ENABLE);
	MeasureState = MEASURE_STATE_STOP;
	MeasureCnt = 0;
	timerticks = 0;
}
 
/*-----------------------------------------------------
 - Function Name: Delay_us()
 - Description:   us����ʱ����
 - Input:         xus��λΪ1us
 - Output:        None
 - Return:        None
 - Attention:     ��ʱʱ����10usΪ����
-----------------------------------------------------*/
void Delay_us(uint32_t xus)
{
#ifdef OS_uCOS_II_H
	OSSchedLock();  // ��������,��ֹ�߳��л�������ʱ��׼ȷ
#endif
	timerticks = xus/10;
	while(timerticks>0);
#ifdef OS_uCOS_II_H
	OSSchedUnlock();  //����������
#endif
}

/*-----------------------------------------------------
 - Function Name: TimeMeasure()
 - Description:   ʱ���������
 - Input:         NewState ����ֹͣ״̬
 - Output:        None
 - Return:        time     ����ʱ��
 - Attention:     ���صĲ���ʱ����1usλ��λ��10usΪ����
-----------------------------------------------------*/
uint32_t TimeMeasure(void)
{
	uint32_t time;

	switch(MeasureState)
  {
    case MEASURE_STATE_STOP:
      MeasureState = MEASURE_STATE_WORKING;
      time = 0;
      break;
    case MEASURE_STATE_WORKING:
      MeasureState = MEASURE_STATE_STOP;
      time = MeasureCnt *10;
      break;
    default:
      MeasureState = MEASURE_STATE_STOP;
      time = 0;
  }
	return time;
}

/* Example Codes

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void TIM7_IRQHandler(void)
{
#ifdef OS_uCOS_II_H
	OSSchedLock();  // ��������,��ֹOSIntExit()���߳����HardFault
#endif
	if(TIM_GetITStatus(TIM7, TIM_IT_Update)==SET)
	{
		TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
		if(timerticks>0)
			timerticks--;
		if(MeasureState == MEASURE_STATE_WORKING)
			MeasureCnt++;
	}
#ifdef OS_uCOS_II_H
	OSSchedUnlock();  //����������
#endif
}

*/
