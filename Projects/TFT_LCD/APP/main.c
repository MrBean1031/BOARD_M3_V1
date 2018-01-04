#include "stm32f10x.h"
#include "key.h"
#include "led.h"
/*#include "SysTick.h"*/
#include "delay_tim.h"
#include "usart.h"
#include "ff.h"
#include "lcd9341_fsmc.h"
#include "show_bmp.h"

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
  
  NVIC_InitStruct.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

int main(void)
{	
	
	u16 color;
  u16 lines;
  u8 keyval;
	
	/*SysTick_Init();*/
  NVIC_Configuration();
  TIM7_TimeBase_Config();
	LED_Config();
	Key_Config();
	USART_Config();
	ILI9341_Initial();
  puts("hello\r\n");
	
//  LCD_SetScreenDir(L2R_D2U);
//  LCD_ShowBmp(0,0,"0:/ENC28J60.bmp");
//  LCD_ShowBmp(10,10,"0:/ENC28J60_2.bmp");
//  LCD_ShowBmp(20,20,"0:/dog_code.bmp");
  
  BackColor = BLUE;
	LCD_SetScreenDir(U2D_R2L);
	LCD_DrawRectangle(0,0,159,119,RED);
	LCD_DrawCircle(109,70,50,BLUE);
	LCD_DrawLine(20,239-20,319-20,20,GREEN);
	LCD_ShowChar(20,20,'A',12,1);
	LCD_ShowChar(26,20,'A',16,1);
	LCD_ShowChar(34,20,'A',24,1);
	LCD_SetScreenDir(R2L_D2U);
	LCD_ShowChar(20,20,'A',12,0);
	LCD_ShowChar(26,20,'A',16,0);
	LCD_ShowChar(34,20,'A',24,0);
	LCD_SetScreenDir(D2U_L2R);
	LCD_ShowChar(20,20,'A',12,1);
	LCD_ShowChar(26,20,'A',16,1);
	LCD_ShowChar(34,20,'A',24,1);
	LCD_SetScreenDir(L2R_U2D);
	LCD_ShowChar(20,20,'A',12,0);
	LCD_ShowChar(26,20,'A',16,0);
	LCD_ShowChar(34,20,'A',24,0);
	LCD_SetScreenDir(L2R_U2D);
	lines = LCD_ShowStr(20,60,"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ",16,0,26);
  printf("lines %d\r\n", lines);
  lines = LCD_ShowStr(20,60+lines*16,"开发板STM32_TFTLCD中英文显示",16,1,26);
  printf("lines %d\r\n", lines);
	
	BackColor= RED;
	LCD_ShowNum(20,140,32767,16,0,6);
	LCD_ShowNum(20,156,-32767,16,0,6);
	LCD_ShowNum(20,172,1000000,16,0,6);
	
	LCD_ShowNum(148,140,32767,16,1,6);
	LCD_ShowNum(148,156,-32767,16,1,6);
	LCD_ShowNum(148,172,1000000,16,1,6);
	
	LCD_ShowNum(20,188,32767,12,0,6);
	LCD_ShowNum(20,200,-32767,12,0,6);
	LCD_ShowNum(20,212,1000000,12,0,6);

	color = LCD_GetPoint(0,0);
	printf("Point color read from GRAM %04x\r\n", color);
	
	while(1)
	{
    keyval = KeyScan();
    switch(keyval)
    {
      case KEY_WK:
        puts("key wk is pressed down\r\n");
        LCD_SetScreenDir(L2R_D2U);
        LCD_ScreenShot(0,0,lcd_param.width,lcd_param.height,"0:/screenshot.bmp");
        LED0(ON);
        break;
      case KEY0:
        puts("key 0 is pressed down\r\n");
        LED0(OFF);
        break;
      case KEY1:
        puts("key 1 is pressed down\r\n");
        break;
    }
	}
}


