#include "stm32f10x.h"
#include "SysTick_OS.h"
#include "timer6.h"
#include "timer_event.h"
#include "delay_tim.h"
#include "led.h"
#include "usart.h"
#include "key.h"
#include "switch_device.h"
#include <stdlib.h>
#include "includes.h"

#define USE_RTOS

/* Function Declaration */
void NVIC_Configuration(void);
void task1(void *arg);
int sw_dev_blink(const char *name, u8 isblink, u32 msec, u8 times);

#define RTOS_STACK_NUM    1
#define RTOS_STACK_SIZE   256
OS_STK os_task_stk[RTOS_STACK_NUM][RTOS_STACK_SIZE];

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
#ifndef USE_RTOS
	NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
#endif
	NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

int main(void)
{	
  u8 *ptr1;
  u8 key_val;
  
	NVIC_Configuration();
  USART_Config();
  Key_Config();
  timer_event_init();
  TIM7_TimeBase_Config();
#ifdef USE_RTOS
  SysTick_Init();
  OSInit();
  OSTaskCreate(task1, (void *)0, &os_task_stk[0][RTOS_STACK_SIZE - 1], 4);
  OSStart();
#else
  TIM6_TimeBaseInit();
#endif

#ifndef USE_RTOS
#if 0
  //memory allocation test
  ptr1 = (u8 *)malloc(3 * 1024);
  if (ptr1 == NULL) {
    puts("malloc fail 1\r\n");
  }
  free(ptr1);
  ptr1 = NULL;
  ptr1 = (u8 *)malloc(3 * 1024);
  if (ptr1 == NULL) {
    puts("malloc fail 2\r\n");
  }
  free(ptr1);
  
  while(1);
#endif

#if 1
  //switch device test
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
  sw_dev_open("led0", GPIOB, GPIO_Pin_8, SW_RESET, SW_ON);
  sw_dev_open("relay", GPIOB, GPIO_Pin_10, SW_SET, SW_OFF);
  sw_dev_open("beep", GPIOE, GPIO_Pin_6, SW_SET, SW_OFF);
  while(1)
  {
    key_val = KeyScan();
    switch (key_val)
    {
      case KEY_WK:
        puts("key wk push down\r\n");
        sw_dev_change("led0", SW_TOGGLE);
        //sw_dev_close("led0");
        break;
      case KEY0:
        puts("key0 push down\r\n");
        sw_dev_change("relay", SW_TOGGLE);
        //sw_dev_close("relay");
        break;
      case KEY1:
        puts("key1 push down\r\n");
        sw_dev_blink("beep", 1, 100, 3);
        //sw_dev_close("beep");
        break;
    }
  }
#endif

  while(1);
#endif  // USE_RTOS
}

void sw_dev_callback(void *arg)
{
  int id;
  u8 *data = (u8 *)arg;
  u8 *times = &data[6];
  u8 isblink = data[1];
  u32 msec;
  const char *name;
  
  name = (const char *)(*((u32 *)&data[7]));
  data[0] = (u8)((enum sw_state)data[0] == SW_ON ? SW_OFF : SW_ON);
  sw_dev_change(name, (enum sw_state)data[0]);
  if(*times != 0xff) {
    *times = *times - 1;
  }
  if(isblink && *times) {  // if blink
    memcpy((u8 *)&msec, data + 2, 4);
    id = timer_event_add(data, sw_dev_callback, msec);
    //timer_event_del(id);  //中断中删除测试
  }
}

int sw_dev_blink(const char *name, u8 isblink, u32 msec, u8 times)
{
  int id, err;
  static u8 data[20];
  
  data[0] = (u8)SW_ON;
  data[1] = isblink;
  memcpy(data + 2, (u8 *)&msec, 4);
  if(times != 0xff) {
    data[6] = (u8)(times * 2 -1);
  } else {
    data[6] = 0xff;
  }
  data[7] = (u8)((u32)name & 0x000000ff);
  data[8] = (u8)(((u32)name & 0x0000ff00) >> 8);
  data[9] = (u8)(((u32)name & 0x00ff0000) >> 16);
  data[10] = (u8)(((u32)name & 0xff000000) >> 24);
  err = sw_dev_change(name, (enum sw_state)data[0]);
  if (err != SW_ERR_NONE) {
    printf("%s() return err with %d\r\n", __func__, err);
    return err;
  }
  id = timer_event_add(data, sw_dev_callback, msec);
  //timer_event_del(id);  //中断外删除测试
  return 0;
}

void task1(void *arg)
{
  u8 key_val;

  puts("task1 is created\r\n");
  //OSStatInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
  sw_dev_open("led0", GPIOB, GPIO_Pin_8, SW_RESET, SW_ON);
  sw_dev_open("relay", GPIOB, GPIO_Pin_10, SW_SET, SW_OFF);
  sw_dev_open("beep", GPIOE, GPIO_Pin_6, SW_SET, SW_OFF);
  while(1)
  {
    key_val = KeyScan();
    switch (key_val)
    {
      case KEY_WK:
        puts("key wk push down\r\n");
        sw_dev_change("led0", SW_TOGGLE);
        break;
      case KEY0:
        puts("key 0 push down\r\n");
        sw_dev_change("relay", SW_TOGGLE);
        break;
      case KEY1:
        puts("key 1 push down\r\n");
        sw_dev_blink("beep", 1, 100, 3);
        break;
    }
    OSTimeDly(2); //delay
  }
}
