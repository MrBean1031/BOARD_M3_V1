#ifndef __LCD9341_H
#define __LCD9341_H

#include "stm32f10x.h"

//端口宏定义
#define LCD_BL_ON  (GPIOB->ODR |=  GPIO_Pin_0)
#define LCD_BL_OFF (GPIOB->ODR &= ~GPIO_Pin_0)

//函数名宏封装
#define LCD_ILI9341_CMD(x)       LCD_WR_REG(x)
#define LCD_ILI9341_Parameter(x) LCD_WR_DATA(x)

//FSMC_A16接LCD的DCX
//当数据线位宽为16B时，FSMC[24:0]对应HADDR[25:1]
//选择Bank1_NORSRAM1连接LCD，地址范围为0x6000_0000 - 0x63FF_FFFF
#define Bank1_LCD_D    ((uint32_t)0x60020000)  //FSMC_A16 - HADDR_A17置1
#define Bank1_LCD_C    ((uint32_t)0x60000000)

#define LCD_WR_REG(cmd)    (*(__IO uint16_t*)Bank1_LCD_C = (uint16_t)(cmd))
#define LCD_WR_DATA(data)  (*(__IO uint16_t*)Bank1_LCD_D = (uint16_t)(data))
#define LCD_RD_DATA()      (*(__IO uint16_t*)Bank1_LCD_D)

//中文字模支持
//#define __USE_CHN


//无论MV取值如何，MY,MX决定了画图起点和终点
//Memory Access Control(36h)
//bit[7:0] MY MX MV ML BGR MH 0 0 
typedef enum{
	L2R_U2D,  //0:MY 0,MX 0, MV 0
	L2R_D2U,  //1:MY 1,MX 0, MV 0
	R2L_U2D,  //2:MY 0,MX 1, MV 0
	R2L_D2U,  //3:MY 1,MX 1, MV 0
	U2D_L2R,  //4:MY 0,MX 0, MV 1
	U2D_R2L,  //5:MY 0,MX 1, MV 1
	D2U_L2R,  //6:MY 1,MX 0, MV 1
	D2U_R2L   //7:MY 1,MX 1, MV 1
}SCREEN_DIR;

//LCD全局参数结构体声明
typedef struct{
	u16 LCD_ID;
	u16 width;
	u16 height;
	SCREEN_DIR screendir;
}LCD_GlobalTypeDef;


//基本绘图颜色
#define WHITE      0xFFFF
#define BLACK      0x0000	  
#define BLUE       0x001F  
#define BRED       0XF81F
#define GRED       0XFFE0
#define GBLUE			 0X07FF
#define RED        0xF800
#define MAGENTA    0xF81F
#define GREEN      0x07E0
#define CYAN       0x7FFF
#define YELLOW     0xFFE0
#define BROWN 		 0XBC40 //棕色
#define BRRED 	   0XFC07 //棕红色
#define GRAY       0X8430 //灰色

//外部变量声明
extern LCD_GlobalTypeDef lcd_param;
extern u16 FontColor,BackColor;

//函数声明
u8 LCD_SetWindow(u16 x, u16 y, u16 width, u16 height);
void LCD_SetScreenDir(SCREEN_DIR dir);
void LCD_DrawPoint(u16 x, u16 y, u16 color);
u16 LCD_ReadPoint(u16 x, u16 y);
void ILI9341_Initial(void);
void LCD_FillColor(u16 x, u16 y, u16 width, u16 height, u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawCircle(u16 x0, u16 y0, u8 r, u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
#ifdef __USE_CHN
void LCD_ShowChn(u16 x, u16 y, const char* ch, u8 size, u8 mode);
#endif
void LCD_ShowChar(u16 x, u16 y, char ch, u8 size, u16 mode);
void LCD_ShowStr(u16 x, u16 y, const char* str, u8 size, u8 mode);
static char* inter2string(int value, char *s, int radix);
void LCD_ShowNum(u16 x, u16 y, int num, u8 length, u8 size, u8 mode);

#endif //__LCD9341_H
