#include "led.h"

void LED_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(LED0_PERIPH_CLK | LED1_PERIPH_CLK, ENABLE);  //开启外设时钟
	
	GPIO_InitStructure.GPIO_Pin = LED0_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //设置IO模式为通用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED0_GPIO_PORT, &GPIO_InitStructure);  //led0=PB8
	
  GPIO_InitStructure.GPIO_Pin = LED1_GPIO_PIN;
	GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);  //led1=PB9
	
	GPIO_SetBits(LED0_GPIO_PORT, LED0_GPIO_PIN);
	GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO_PIN);
}

void LED0(uint8_t flag)
{
	if(flag == ON) {
		GPIO_ResetBits(LED0_GPIO_PORT, LED0_GPIO_PIN);
  } else {
		GPIO_SetBits(LED0_GPIO_PORT, LED0_GPIO_PIN);
  }
}

void LED1(uint8_t flag)
{
	if(flag == ON) {
		GPIO_ResetBits(LED1_GPIO_PORT, LED1_GPIO_PIN);
  } else {
		GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO_PIN);
  }
}
