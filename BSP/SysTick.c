#include "SysTick.h"

//#define USER_SYSTICK_INIT

__IO uint32_t nTime;
int SysTick_DelayFlag=0;

void SysTick_Init(void)
{
	#ifndef USER_SYSTICK_INIT
	//SystemCoreClock  72 000 000
	//SystemCoreClock/1        SysTickʱ��1s
	//SystemCoreClock/1000     SysTickʱ��1ms
	//SystemCoreClock/1000000  SysTickʱ��1us
	while(SysTick_Config(SystemCoreClock/1000000) == 1);  //ʱ��1us,ʧ���������ѭ��
	
	#else
	/* set Priority for Cortex-M0 System Interrupts */
	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  
	SysTick->LOAD = 9-1;
	SysTick->VAL = 0;
	SysTick->CTRL |= (1ul<<0) |  //Enable SysTick
                   (1ul<<1) |  //ʹ��SysTick�쳣
	                 (1ul<<2);   //ѡ��ʱ��ԴΪHCLK(FCLK)
	#endif /*USER_SYSTICK_INIT*/
}

void Delay_us(uint32_t xus)
{
	nTime=xus;
	SysTick_DelayFlag = 1;
	while(nTime != 0);  //�ȴ�nTime��SysTick�쳣������������
	SysTick_DelayFlag = 0;
}


/*-----------------------------
  �쳣�����ӳ���
  -����SysTick_Handler(void)��
	-�������ж�SysTick_DelayFlag == 1
	 �Ƿ������������ִ�и��ӳ���
------------------------------*/
void SysTick_DelayServer(void)
{
	if(nTime != 0)
		nTime--;
}
