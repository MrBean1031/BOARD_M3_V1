#include "stm32f10x.h"
#include "includes.h"
#include "SysTick_OS.h"
#include "delay_tim.h"
#include "led.h"
#include "usart.h"
#include "spi.h"             //ENC28J60
#include "key.h"
#include "switch_device.h"
#include "timer_event.h"
#include "lcd9341_fsmc.h"
#include "rc522_function.h"
#include "lwip/tcpip.h"
#include "netconfig.h"
#include "access_app.h"
#include "global.h"

/* ������Դ */
#define OS_TASK_STK_SIZE      128  //512 Bytes
OS_STK StartTaskStack[OS_TASK_STK_SIZE];

/* �ⲿ�������� */

/* Function Declaration */
void NVIC_Configuration(void);
void StartTask(void *pdata);
extern void ethernetif_input(struct netif *netif);


int main(void)
{
  NVIC_Configuration();  //����NVIC
  OSInit();  //��ʼ��uCOS-II
  OSTaskCreate(StartTask, (void*)0, &StartTaskStack[OS_TASK_STK_SIZE-1], 0);
  OSStart();  //����uCOS-II�ں�
}

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

void StartTask(void* arg)
{
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR cpu_sr;
#endif
  
  arg = arg;  //��ֹ����������
  USART_Config();
  puts("StartTask() is created\r\n");
  OS_ENTER_CRITICAL();
  TIM7_TimeBase_Config();
  SysTick_Init();  //��ʼ��uCOSʱ��
  OS_EXIT_CRITICAL();
  
  timer_event_init();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
  sw_dev_open("led0", GPIOB, GPIO_Pin_8, SW_RESET, SW_OFF);
  sw_dev_open("led1", GPIOB, GPIO_Pin_9, SW_RESET, SW_OFF);
  sw_dev_open("relay", GPIOB, GPIO_Pin_10, SW_SET, SW_ON);
  sw_dev_open("beep", GPIOA, GPIO_Pin_8, SW_SET, SW_OFF);
  sw_dev_open("tp_cs", GPIOB, GPIO_Pin_12, SW_RESET, SW_OFF);  //necessary, ��ֹ����SPI�ӿڸ���RC522����
  Key_Config();
//  printf("FILE %s, LINE %d\r\n", __FILE__, __LINE__);
  ILI9341_Initial();
//  printf("FILE %s, LINE %d\r\n", __FILE__, __LINE__);
  //��ʼ��RC522
  RC522_SPI_Config(SPI_BaudRatePrescaler_8);
  PcdReset();
  M500PcdConfigISOType('A');  //ISO14443A
//  printf("FILE %s, LINE %d\r\n", __FILE__, __LINE__);
  //��ʼ��ENC28J60 SPI�ӿ�
  ENC_SPI_Init();
  
  LwIP_Init();
  OSStatInit();  //��ʼ��ͳ������,��ֹ�����ٽ������
//  printf("FILE %s, LINE %d\r\n", __FILE__, __LINE__);
  OSTaskCreate(access_handler, (void*)0, &access_handler_stk[127], 10);
  OSTaskCreate(access_disp, (void*)0, &access_disp_stk[127], 11);
  OSTaskCreate(access_gettime, (void*)0, &access_gettime_stk[63], 12);
  while(1)
  {
    ethernetif_input(&enc28j60);
    OSTimeDly(3);
  }
}

