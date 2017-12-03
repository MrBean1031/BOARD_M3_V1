#include "stm32f10x.h"
#include "SysTick.h"
#include "timer6.h"
#include "led.h"
#include "usart.h"
#include "timer_event.h"


void led1_callback(void *arg)
{
  static u8 status = ON;
  int id;
  status = (status == ON ? OFF : ON);
  LED0(status);
  id = timer_event_add(0, led1_callback, 1000);  //中断中添加节点测试
  //timer_event_del(id);  //中断中删除测试
}

void led_timer_test(void)
{
  int id;
  LED0(ON);
  id = timer_event_add(0, led1_callback, 1000);
  //timer_event_del(id);  //中断外删除测试
}

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
	SysTick_Init();
  LED_Config();
  USART_Config();
  timer_event_init();
  TIM6_TimeBaseInit();
  NVIC_Configuration();

  led_timer_test();
  puts("aaaaa\r\n");
	while(1)
	{
	}
}
