#include "SysTick.h"

//#define USER_SYSTICK_INIT

__IO uint32_t nTime;
int SysTick_DelayFlag=0;

void SysTick_Init(void)
{
	#ifndef USER_SYSTICK_INIT
	//SystemCoreClock  72 000 000
	//SystemCoreClock/1        SysTick时基1s
	//SystemCoreClock/1000     SysTick时基1ms
	//SystemCoreClock/1000000  SysTick时基1us
	while(SysTick_Config(SystemCoreClock/1000000) == 1);  //时基1us,失败则进入死循环
	
	#else
	/* set Priority for Cortex-M0 System Interrupts */
	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  
	SysTick->LOAD = 9-1;
	SysTick->VAL = 0;
	SysTick->CTRL |= (1ul<<0) |  //Enable SysTick
                   (1ul<<1) |  //使能SysTick异常
	                 (1ul<<2);   //选择时钟源为HCLK(FCLK)
	#endif /*USER_SYSTICK_INIT*/
}

void Delay_us(uint32_t xus)
{
	nTime=xus;
	SysTick_DelayFlag = 1;
	while(nTime != 0);  //等待nTime在SysTick异常服务程序减至零
	SysTick_DelayFlag = 0;
}


/*-----------------------------
  异常服务子程序
  -放在SysTick_Handler(void)里
	-必须先判断SysTick_DelayFlag == 1
	 是否成立，成立才执行该子程序
------------------------------*/
void SysTick_DelayServer(void)
{
	if(nTime != 0)
		nTime--;
}
