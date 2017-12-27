#include "SysTick_OS.h"
#include "includes.h"  // uCOSII support

#define SYSTICK_CONFIG_CORTEX 0

void SysTick_Init(void)
{
#if SYSTICK_CONFIG_CORTEX==1
	// SystemCoreClock          72_000_000
	// SystemCoreClock/1        SysTick时基1s
	// SystemCoreClock/1000     SysTick时基1ms
	// SystemCoreClock/1000000  SysTick时基1us
	while(SysTick_Config(SystemCoreClock/OS_TICKS_PER_SEC) == 1);  //T = 1/OS_TICKS_PER_SEC
#else
	/* set Priority for Cortex-M0 System Interrupts */
	//prio = 0x0000_FFFF,STM32 use high nibbles as prio,that means the highest prio.
//	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  // Lowest Priority
  NVIC_SetPriority(SysTick_IRQn, 0); // Higest Priority
	while(SystemCoreClock/OS_TICKS_PER_SEC > SysTick_LOAD_RELOAD_Msk);
	SysTick->LOAD = SystemCoreClock/OS_TICKS_PER_SEC - 1;
	SysTick->VAL = 0;
	SysTick->CTRL |= (1ul<<0) |  //Enable SysTick
                   (1ul<<1) |  //使能SysTick异常
	                 (1ul<<2);   //选择时钟源为HCLK(FCLK)
#endif /* SYSTICK_CONFIG_CORTEX */
}

