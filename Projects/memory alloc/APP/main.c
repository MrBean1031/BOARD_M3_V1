#include "stm32f10x.h"
#include "SysTick.h"
#include "timer_event.h"
#include "led.h"
#include "usart.h"
#include "timer6.h"
#include <stdlib.h>

/* Function Declaration */
void NVIC_Configuration(void);

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

int main(void)
{	
  u8 *ptr1, *ptr2;
  
	NVIC_Configuration();
	LED_Config();
  USART_Config();
  //TIM6_TimeBaseInit();
  //timer_event_init();
  SysTick_Init();
  
  ptr1 = (u8 *)malloc(256);
  if (ptr1 == NULL) {
    puts("malloc fail 1\r\n");
  }
  free(ptr1);
  ptr1 = NULL;
  ptr1 = (u8 *)malloc(240);
  if (ptr1 == NULL) {
    puts("malloc fail 2\r\n");
  }
  ptr2 = (u8 *)malloc(240);
  if (ptr2 == NULL) {
    puts("malloc fail 3\r\n");
  }
  
  while(1);
}