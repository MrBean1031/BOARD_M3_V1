#include "stm32f10x.h"
#include "key.h"
#include "led.h"
#include "SysTick.h"
#include "usart.h"
#include "ff.h"
#include "lcd9341_fsmc.h"

int main(void)
{	
	
	u16 color;
	
	SysTick_Init();
	LED_Config();
	Key_Config();
	USART_Config();
	ILI9341_Initial();
	
	LCD_ScreenDir(U2D_R2L);
	LCD_DrawRectangle(0,0,159,119,RED);
	LCD_DrawCircle(109,70,50,BLUE);
	LCD_DrawLine(20,239-20,319-20,20,GREEN);
	LCD_ShowChar(20,20,'A',12,1);
	LCD_ShowChar(26,20,'A',16,1);
	LCD_ShowChar(34,20,'A',24,1);
	LCD_ScreenDir(R2L_D2U);
	LCD_ShowChar(20,20,'A',12,0);
	LCD_ShowChar(26,20,'A',16,0);
	LCD_ShowChar(34,20,'A',24,0);
	LCD_ScreenDir(D2U_L2R);
	LCD_ShowChar(20,20,'A',12,1);
	LCD_ShowChar(26,20,'A',16,1);
	LCD_ShowChar(34,20,'A',24,1);
	LCD_ScreenDir(L2R_U2D);
	LCD_ShowChar(20,20,'A',12,0);
	LCD_ShowChar(26,20,'A',16,0);
	LCD_ShowChar(34,20,'A',24,0);
	LCD_ScreenDir(U2D_R2L);
	LCD_ShowStr(20,60,"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ",16,1);
	//LCD_ShowStr(20,108,"正点原子STM32_TFTLCD中英文显示",16,1);
	
	BackColor = RED;
	LCD_ShowNum(20,140,32767,6,16,0);
	LCD_ShowNum(20,156,-32767,6,16,0);
	LCD_ShowNum(20,172,1000000,6,16,0);
	
	LCD_ShowNum(148,140,32767,6,16,1);
	LCD_ShowNum(148,156,-32767,6,16,1);
	LCD_ShowNum(148,172,1000000,6,16,1);
	
	LCD_ShowNum(20,188,32767,6,12,0);
	LCD_ShowNum(20,200,-32767,6,12,0);
	LCD_ShowNum(20,212,1000000,6,12,0);

	color = LCD_ReadPoint(0,0);
	printf("Point color read from GRAM %X\r\n", color);
	
	while(1)
	{
    ;
	}
}


