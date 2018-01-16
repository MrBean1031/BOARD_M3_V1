/*USART1 TXD -> PA9, USART1 RXD -> PA10*/
#include "usart.h"
#include "lcd9341_fsmc.h"

void USART_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	/*Enable USART1,GPIOA clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	
	/*Config USART1_Tx PA9 as alternate function push-pull*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	/*Config USART1_Rx PA10 as input floating*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*Config USART1*/
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1,&USART_InitStructure);
	USART_Cmd(USART1,ENABLE);
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //Enable USART1 IT
}

int fputc(int c, FILE * stream)
{
#if 1
  while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
	USART_SendData(USART1,(unsigned int)c);
#else
  LCD_PutChar((u8)c, 12, 1);
#endif
	return c;
}

