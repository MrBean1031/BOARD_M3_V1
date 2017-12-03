#include "stm32f10x.h"
#include "includes.h"
#include "SysTick_OS.h"
#include "timer_event.h"
#include "led.h"
#include "usart.h"
#include "timer6.h"
#include <string.h>

#define OS_TASK_STK_SIZE 128  //512 Bytes
OS_STK StartTaskStack[OS_TASK_STK_SIZE];
OS_STK Task1Stack[OS_TASK_STK_SIZE];
OS_STK Task2Stack[OS_TASK_STK_SIZE];

/* Function Declaration */
void NVIC_Configuration(void);
void StartTask(void* pdata);
void Task1(void* pdata);
void Task2(void* pdata);

uint32_t LocalTime=0;


int main(void)
{	
	NVIC_Configuration();
	LED_Config();
  USART_Config();
  TIM6_TimeBaseInit();
  timer_event_init();
	OSInit();
	OSTaskCreate(StartTask,(void*)0, &StartTaskStack[OS_TASK_STK_SIZE-1], 0);
	OSStart();
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

void StartTask(void* pdata)
{
	#if OS_CRITICAL_METHOD==3
		OS_CPU_SR cpu_sr;
	#endif
	
	(void)pdata;  //防止编译器警告
	OS_ENTER_CRITICAL();  //关中断
	SysTick_Init();  //初始化uCOS时钟
	OS_EXIT_CRITICAL();  //开中断
	OSStatInit();  //初始化统计任务
	OSTaskCreate(Task1, (void*)0, &Task1Stack[OS_TASK_STK_SIZE-1], 2);
	
	OSTaskDel(OS_PRIO_SELF);
}
	
void led1_callback(void *arg)
{
  int id;
  u8 *data = (u8 *)arg;
  u32 msec;
  
  data[0] = (data[0] == ON ? OFF : ON);
  LED0(data[0]);
  if(data[6] != 0xff) {
    data[6] = data[6] - 1;
  }
  if(data[1] && data[6]) {  // if blink
    memcpy((u8 *)&msec, data + 2, 4);
    id = timer_event_add(data, led1_callback, msec);
    //timer_event_del(id);  //中断中删除测试
  }
}

void led_blink(u8 isblink, u32 msec, u8 times)
{
  int id;
  static u8 data[10];
  
  data[0] = (u8)ON;
  data[1] = isblink;
  memcpy(data + 2, (u8 *)&msec, 4);
  if(times != 0xff) {
    data[6] = (u8)(times * 2 -1);
  } else {
    data[6] = 0xff;
  }
  LED0(data[0]);
  id = timer_event_add(data, led1_callback, msec);
  //timer_event_del(id);  //中断外删除测试
}

void Task1(void* pdata)
{
	(void)pdata;  //防止编译器警告
  puts("task1 is created\r\n");
  led_blink(1, 100, 0xff);
	while(1)
	{
	}
}
