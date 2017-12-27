/*
 *File Description: 无任务调度延时服务，时间测量服务
 *Pin Mapping:      None
 *Author:           Mr.Bean
 *Date:             2017/2/16
 *Attention:        1.用到定时器7的Update中断，需要编写NVIC代码设置中断优先级并使能中断
                    2.需要编写中断服务子程序（ISR）
										3.TimeMeasure()，为了时间测量的精度，没有使用任务同步机制，注意不要重入
 */
 
#include "delay_tim.h"
#include "includes.h"


uint32_t timerticks;
uint8_t MeasureState;  //测量状态机
uint8_t MeasureCnt;    //测量计数

/*-----------------------------------------------------
 - Function Name: TIM6_TimeBase_Config()
 - Description:   使用定时器6完成时基为10us的延时服务，时间测量服务
 - Input:         None
 - Output:        None
 - Return:        None
 - Attention:     None
-----------------------------------------------------*/
void TIM7_TimeBase_Config(void)
{
	TIM_TimeBaseInitTypeDef TimeBaseStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	// 定时器的时钟频率 fCKCNT=TIMxCLK/(1+TIM_Prescaler)
	TimeBaseStruct.TIM_Prescaler = 0;
	// TIM6，TIM7只支持向上计数模式
	TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	// 溢出周期 T=TIM_Period+1，10us
	TimeBaseStruct.TIM_Period = 719;
	// 输入滤波器分频系数，这里没有用到
	TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM7,&TimeBaseStruct);
	
	// 使能预装载寄存器
	TIM_ARRPreloadConfig(TIM7, ENABLE);
	// 使能TIM7更新中断
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	// 使能TIM7
	TIM_Cmd(TIM7, ENABLE);
	MeasureState = MEASURE_STATE_STOP;
	MeasureCnt = 0;
	timerticks = 0;
}
 
/*-----------------------------------------------------
 - Function Name: Delay_us()
 - Description:   us级延时服务
 - Input:         xus单位为1us
 - Output:        None
 - Return:        None
 - Attention:     延时时间以10us为粒度
-----------------------------------------------------*/
void Delay_us(uint32_t xus)
{
#ifdef OS_uCOS_II_H
	OSSchedLock();  // 锁调度器,防止线程切换导致延时不准确
#endif
	timerticks = xus/10;
	while(timerticks>0);
#ifdef OS_uCOS_II_H
	OSSchedUnlock();  //解锁调度器
#endif
}

/*-----------------------------------------------------
 - Function Name: TimeMeasure()
 - Description:   时间测量服务
 - Input:         NewState 启动停止状态
 - Output:        None
 - Return:        time     测量时间
 - Attention:     返回的测量时间以1us位单位，10us为粒度
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
	OSSchedLock();  // 锁调度器,防止OSIntExit()切线程造成HardFault
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
	OSSchedUnlock();  //解锁调度器
#endif
}

*/
