/*
 *File Description: ILI9341�ײ���������
 *Pin Mapping:      CS -> PD7 - FSMC_NE1
                    RS -> PD11- FSMC_A16
										WR -> PD5 - FSMC_NWE
										RD -> PD4 - FSMC_NOE
										RST-> RST����
										D[15:0] -> FSMC_D[15:0]
										BLON -> PB0		
 *Author:           Mr.Bean
 *Date:             2017/4/30
 *Attention:        ʹ�������ַ���Ҫ��SD����Flash�������ģ�ļ�
                    ������__USE_CHN
 */
 
#include "lcd9341_fsmc.h"
//#include "SysTick.h"
#include "delay_tim.h"
#include "usart.h"
#include "font.h"
#include "stdlib.h"
#include "global.h"

#define MALLOC(size)   malloc(size)
#define FREE(ptr)      free(ptr)

#ifdef __USE_CHN
#include "ff.h"
FATFS fsff;
FIL fileff;
FILINFO fiff;
FRESULT resff;
#endif

LCD_GlobalTypeDef lcd_param;
u16 FontColor=WHITE,BackColor=BLUE;  //������ɫ�����屳����ɫ


/*-----------------------------------------------------
 - Function Name: LCD_SetWindow
 - Description:   ������ʾ����
 - Input:         x, y, width, height
 - Output:        None
 - Return:        0 succeed, 1 fail
 - Attention:     ���� x 0--239
                       y 0--319
                  ���� x 0--319
                       y 0--239
                  ����MY MX MV��ͬ����ͼ��㲻ͬ
-----------------------------------------------------*/
u8 LCD_SetWindow(u16 x, u16 y, u16 width, u16 height)
{
	if(x>=lcd_param.width || y>=lcd_param.height) return 1;
	if(x+width-1>=lcd_param.width || y+height-1>=lcd_param.height) return 1;
	LCD_WR_REG(0X2A);  //Column Address Set
	LCD_WR_DATA(x>>8);  //SC
	LCD_WR_DATA(x&0x00FF);
	LCD_WR_DATA((x+width-1)>>8);  //EC
	LCD_WR_DATA((x+width-1)&0X00FF);
	LCD_WR_REG(0X2B);  //Page Address Set
	LCD_WR_DATA(y>>8);  //SP
	LCD_WR_DATA(y&0x00FF);
	LCD_WR_DATA((y+height-1)>>8);  //EP
	LCD_WR_DATA((y+height-1)&0X00FF);
	return 0;
}

void LCD_SetScreenDir(SCREEN_DIR dir)
{
	if(dir <= 7)
		lcd_param.screendir = dir;
	if(lcd_param.screendir <= 3)
	{
		lcd_param.width = 240;
		lcd_param.height = 320;
	}else
	{
		lcd_param.width = 320;
		lcd_param.height = 240;
	}
	switch(lcd_param.screendir)
	{
		case L2R_U2D: LCD_WR_REG(0X36); LCD_WR_DATA(0X08); break;
		case L2R_D2U: LCD_WR_REG(0X36); LCD_WR_DATA(0X88); break;
		case R2L_U2D: LCD_WR_REG(0X36); LCD_WR_DATA(0X48); break;
		case R2L_D2U: LCD_WR_REG(0X36); LCD_WR_DATA(0XC8); break;
		case U2D_L2R: LCD_WR_REG(0X36); LCD_WR_DATA(0X28); break;
		case U2D_R2L: LCD_WR_REG(0X36); LCD_WR_DATA(0X68); break;
		case D2U_L2R: LCD_WR_REG(0X36); LCD_WR_DATA(0XA8); break;
		case D2U_R2L: LCD_WR_REG(0X36); LCD_WR_DATA(0XE8); break;
	}
}

void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
	LCD_SetWindow(x, y, 1, 1);
	LCD_WR_REG(0X2C);  //Memory Write
	LCD_WR_DATA(color);
}

u16 LCD_GetPoint(u16 x, u16 y)
{
	u16 val,r,g,b;
	LCD_SetWindow(x, y, 1, 1);
	LCD_WR_REG(0X2E);  //Memory Read
	LCD_RD_DATA();  //dummy read
	val = LCD_RD_DATA();  //read GRAM
	r = val>>11;
	g = val>>2 & 0X003F;
	val = LCD_RD_DATA();  //read GRAM
	b = val>>11;
	val = r<<11 | g<<5 | b;
	return val;
}

void LCD_ReadBuffer(u16 x, u16 y, u16 width, u16 height, u8 *buff)
{
	u16 val,r,g,b;
  u32 i;
	val = LCD_SetWindow(x, y, width, height);
  if (val) {
    return;
  }
	LCD_WR_REG(0X2E);  //Memory Read
	LCD_RD_DATA();  //dummy read
  for (i=0; i<width*height; i++)
  {
	  val = LCD_RD_DATA();  //read GRAM
	  r = val>>11;
	  g = val>>2 & 0X003F;
	  val = LCD_RD_DATA();  //read GRAM
	  b = val>>11;
	  val = r<<11 | g<<5 | b;
    *(u16 *)(buff + 2*i) = val;
  }
}

void LCD_FillColor(u16 x, u16 y, u16 width, u16 height, u16 color)
{
	u32 i;
	LCD_SetWindow(x, y, width, height);
	LCD_WR_REG(0X2C);
	for(i=0; i<width*height; i++)
	{
		LCD_WR_DATA(color);
	}
}

void LCD_FillBuffer(u16 x, u16 y, u16 width, u16 height, u8 *buff)
{
	u32 i;
	LCD_SetWindow(x, y, width, height);
	LCD_WR_REG(0X2C);
	for(i=0; i<width*height; i++)
	{
		LCD_WR_DATA(*(u16 *)(buff + 2*i));
	}
}

void ILI9341_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	FSMC_NORSRAMInitTypeDef FSMC_NORInitStruct;
  FSMC_NORSRAMTimingInitTypeDef FSMC_NORTiming;
  
  //ʹ��GPIO��FSMC��ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
  /* ��ʼ��FSMC��NE1,NWE,NOE,A[23:0],D[15:0]Ϊ���츴����� */
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_11|GPIO_Pin_0|
                             GPIO_Pin_1|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_13|
                             GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOE, &GPIO_InitStruct);
  /* ����FSMC_NWAITΪ�������� */
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
  /* ����BL_ONΪ����ͨ����� */
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //����FSMC_NOR��ʱ��
  FSMC_NORTiming.FSMC_CLKDivision = 1;  //HCLK��2��Ƶ
  FSMC_NORTiming.FSMC_AccessMode = FSMC_AccessMode_B;  //����ģʽ��ֻ�е���չģʽ�²���Ч
  FSMC_NORTiming.FSMC_AddressSetupTime = 4;
  FSMC_NORTiming.FSMC_AddressHoldTime = 4;
  FSMC_NORTiming.FSMC_DataSetupTime = 20;
  FSMC_NORTiming.FSMC_DataLatency = 5;  //ͬ������ʽ��NOR�����ݱ���ʱ�䣬�첽ģʽ��ʹ��
  FSMC_NORTiming.FSMC_BusTurnAroundDuration = 15;  //���߻ָ�ʱ�䣬���ڵ�ַ���ݸ���
  
  //����FSMC_NOR����
  FSMC_NORInitStruct.FSMC_Bank = FSMC_Bank1_NORSRAM1;  //NORSRAM1ʹ��FSMC_NE1
  FSMC_NORInitStruct.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;  //���ݵ�ַ�߸���ʹ��
  FSMC_NORInitStruct.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;  //16λ����λ��
  FSMC_NORInitStruct.FSMC_MemoryType = FSMC_MemoryType_NOR;
  FSMC_NORInitStruct.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  //��ֹ�첽�ȴ�
  FSMC_NORInitStruct.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;  //��ֹͻ��ģʽ
  FSMC_NORInitStruct.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  //��ֹ�ȴ��ź�
  FSMC_NORInitStruct.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  //�ȴ��źŲ�����ʱ��
  FSMC_NORInitStruct.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;  //�ȴ��źŵļ���
  FSMC_NORInitStruct.FSMC_WrapMode = FSMC_WrapMode_Disable;  //�Ƕ�����ʣ�ͻ��ģʽ��ʹ��
  FSMC_NORInitStruct.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  //дͻ����ͻ��ģʽ��ʹ��
  FSMC_NORInitStruct.FSMC_WriteOperation = FSMC_WriteOperation_Enable;  //дʹ�ܣ���ֹ�򲻻����дʱ��
  FSMC_NORInitStruct.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;  //��չģʽ����ʹ�ò�ͬ�Ķ�дʱ��
  FSMC_NORInitStruct.FSMC_ReadWriteTimingStruct = &FSMC_NORTiming;  //���ö�дʱ��
  FSMC_NORInitStruct.FSMC_WriteTimingStruct = &FSMC_NORTiming;
  FSMC_NORSRAMInit(&FSMC_NORInitStruct);  //��ʼ��
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1,ENABLE);  //ʹ��FSMC_Bank1_NORSRAM1
  
  LCD_WR_REG(0X01);  //SOFTWARE RESET
  Delay_us(10000);  //��ʱ10ms
	LCD_WR_REG(0XD3);  //��ID4
	LCD_RD_DATA();  //xx
	LCD_RD_DATA();  //0X00
	lcd_param.LCD_ID = LCD_RD_DATA()&0xFF;  //0X93
	lcd_param.LCD_ID = (lcd_param.LCD_ID<<8) + (LCD_RD_DATA()&0xFF);  //0X41
	printf("LCD ID %04X\r\n", lcd_param.LCD_ID);
	if(lcd_param.LCD_ID == 0X9341)
	{
		lcd_param.screendir = L2R_U2D;
		lcd_param.width = 240;
		lcd_param.height = 320;
		
		//************* Start Initial Sequence **********//
		LCD_ILI9341_CMD(0xCF);
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_Parameter(0x83);
		LCD_ILI9341_Parameter(0X30);
		LCD_ILI9341_CMD(0xED);
		LCD_ILI9341_Parameter(0x64);
		LCD_ILI9341_Parameter(0x03);
		LCD_ILI9341_Parameter(0X12);
		LCD_ILI9341_Parameter(0X81);
		LCD_ILI9341_CMD(0xE8);
		LCD_ILI9341_Parameter(0x85);
		LCD_ILI9341_Parameter(0x01);
		LCD_ILI9341_Parameter(0x79);
		LCD_ILI9341_CMD(0xCB);
		LCD_ILI9341_Parameter(0x39);
		LCD_ILI9341_Parameter(0x2C);
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_Parameter(0x34);
		LCD_ILI9341_Parameter(0x02);
		LCD_ILI9341_CMD(0xF7);
		LCD_ILI9341_Parameter(0x20);
		LCD_ILI9341_CMD(0xEA);
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_CMD(0xC0); //Power control
		LCD_ILI9341_Parameter(0x1D); //VRH[5:0]
		LCD_ILI9341_CMD(0xC1); //Power control
		LCD_ILI9341_Parameter(0x11); //SAP[2:0];BT[3:0]
		LCD_ILI9341_CMD(0xC5); //VCM control
		LCD_ILI9341_Parameter(0x33);
		LCD_ILI9341_Parameter(0x34);
		LCD_ILI9341_CMD(0xC7); //VCM control2
		LCD_ILI9341_Parameter(0Xbe);
		LCD_ILI9341_CMD(0x36); // Memory Access Control
		LCD_ILI9341_Parameter(0x08);
		LCD_ILI9341_CMD(0xB1);
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_Parameter(0x1B);
		LCD_ILI9341_CMD(0xB6); // Display Function Control
		LCD_ILI9341_Parameter(0x0A);
		LCD_ILI9341_Parameter(0xA2);
		LCD_ILI9341_CMD(0xF2); // 3Gamma Function Disable
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_CMD(0x26); //Gamma curve selected
		LCD_ILI9341_Parameter(0x01);
		LCD_ILI9341_CMD(0xE0); //Set Gamma
		LCD_ILI9341_Parameter(0x0F);
		LCD_ILI9341_Parameter(0x23);
		LCD_ILI9341_Parameter(0x1F);
		LCD_ILI9341_Parameter(0x09);
		LCD_ILI9341_Parameter(0x0f);
		LCD_ILI9341_Parameter(0x08);
		LCD_ILI9341_Parameter(0x4B);
		LCD_ILI9341_Parameter(0Xf2);
		LCD_ILI9341_Parameter(0x38);
		LCD_ILI9341_Parameter(0x09);
		LCD_ILI9341_Parameter(0x13);
		LCD_ILI9341_Parameter(0x03);
		LCD_ILI9341_Parameter(0x12);
		LCD_ILI9341_Parameter(0x07);
		LCD_ILI9341_Parameter(0x04);
		LCD_ILI9341_CMD(0XE1); //Set Gamma
		LCD_ILI9341_Parameter(0x00);
		LCD_ILI9341_Parameter(0x1d);
		LCD_ILI9341_Parameter(0x20);
		LCD_ILI9341_Parameter(0x02);
		LCD_ILI9341_Parameter(0x11);
		LCD_ILI9341_Parameter(0x07);
		LCD_ILI9341_Parameter(0x34);
		LCD_ILI9341_Parameter(0x81);
		LCD_ILI9341_Parameter(0x46);
		LCD_ILI9341_Parameter(0x06);
		LCD_ILI9341_Parameter(0x0e);
		LCD_ILI9341_Parameter(0x0c);
		LCD_ILI9341_Parameter(0x32);
		LCD_ILI9341_Parameter(0x38);
		LCD_ILI9341_Parameter(0x0F);
		LCD_ILI9341_CMD(0X3A);  //COLMOD:16bit or 18bit
		LCD_ILI9341_Parameter(0X55);
		LCD_ILI9341_CMD(0X11);  //SLEEP OUT
		Delay_us(120000);  //delay 120ms
		LCD_ILI9341_CMD(0X29);  //DISPLAY ON
	
		LCD_BL_ON;
		LCD_FillColor(0,0,240,320, BLACK);
#ifdef __USE_CHN
		f_mount(&fsff, _T("0:"),1);  //�����ļ�ϵͳ
#endif
	}
}



/*-----------------------------------------------------
 - Function Name: LCD_DrawLine
 - Description:   ���ߺ�����ʹ����bresenham�㷨��ֱ����
                  �����������꣬��������ʱ��
 - Input:         x1, y1, x2, y2, color
 - Output:        None
 - Return:        None
 - Attention:     
-----------------------------------------------------*/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	u16 t; 
	int xerr = 0, yerr = 0, delta_x, delta_y, distance; 
	int incx, incy, uRow, uCol; 

	delta_x = x2 - x1;  //������������ 
	delta_y = y2 - y1; 
	uRow = x1; 
	uCol = y1; 
	if (delta_x > 0) {
    incx = 1;  //���õ������� 
  } else if (delta_x == 0) {
    incx = 0;  //��ֱ�� 
  } else {
    incx = -1;
    delta_x = -delta_x;
  } 
	if (delta_y > 0) {
    incy = 1; 
  } else if (delta_y == 0) {
    incy = 0;  //ˮƽ�� 
  } else {
    incy = -1;
    delta_y = -delta_y;
  } 
	if (delta_x > delta_y) {
    distance = delta_x;  //ѡȡ�������������� 
  } else {
    distance = delta_y;
  }
	for (t = 0; t <= distance + 1; t++) { //�������  
		LCD_DrawPoint(uRow, uCol, color);  //���� 
		xerr += delta_x ;
		yerr += delta_y ;
		if (xerr > distance) { 
			xerr -= distance; 
			uRow += incx; 
		} 
		if (yerr > distance) { 
			yerr -= distance; 
			uCol += incy; 
		}
	}
}

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
	LCD_DrawLine(x2,y2,x1,y2,color);
	LCD_DrawLine(x1,y2,x1,y1,color);
}

/*-----------------------------------------------------
 - Function Name: LCD_DrawCircle
 - Description:   ��Բ������ʹ����bresenham�㷨��ֱ����
                  �����������꣬��������ʱ��
 - Input:         
 - Output:        
 - Return:        
 - Attention:     
-----------------------------------------------------*/
void LCD_DrawCircle(u16 x0, u16 y0, u8 r, u16 color)
{
	int a, b;
	int di;
	a = 0, b = r;	  
	di = 3 - (r << 1);             //�ж��¸���λ�õı�־
	while(a <= b)
	{
		LCD_DrawPoint(x0 - b, y0 - a, color);             //3           
		LCD_DrawPoint(x0 + b, y0 - a, color);             //0           
		LCD_DrawPoint(x0 - a, y0 + b, color);             //1       
		LCD_DrawPoint(x0 - b, y0 - a, color);             //7           
		LCD_DrawPoint(x0 - a, y0 - b, color);             //2             
		LCD_DrawPoint(x0 + b, y0 + a, color);             //4               
		LCD_DrawPoint(x0 + a, y0 - b, color);             //5
		LCD_DrawPoint(x0 + a, y0 + b, color);             //6 
		LCD_DrawPoint(x0 - b, y0 + a, color);             
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di < 0) {
			di += 4 * a + 6;	  
		} else {
			di += 10 + 4 * (a - b);   
			b--;
		} 
		LCD_DrawPoint(x0 + a, y0 + b, color);
	}
}

/*-----------------------------------------------------
 - Function Name: LCD_ShowChar
 - Description:   Ӣ���ַ���ʾ����asciixx������ȡģ
 - Input:         x, y, ch
                  size - �����С��12,16,24
                  mode - 1 ���ӣ�0 �ǵ���
 - Output:        None
 - Return:        None
 - Attention:     ע����Ļ��������LCD_ScreenDir()
                  ������ɫͨ��ȫ�ֱ���FontColor����,����
                  ͨ��BackColor����
-----------------------------------------------------*/
void LCD_ShowChar(u16 x, u16 y, char ch, u8 size, u16 mode)
{
	int i,j;
	u8 *cache, *buff;
	u16 csize, x0 = x, y0 = y, color = FontColor;
	csize = (size/8 + !!(size%8)) * (size/2);  //��ģ��ռ�ֽ���
  buff = (u8 *)MALLOC(size * size / 2 * 2);
  if (buff == 0) {
    return;
  }
  if(mode) {
    LCD_ReadBuffer(x0, y0, size / 2, size, buff);
  } else {
    memset(buff, BackColor ,size * size / 2 * 2);
  }
  switch(size) 
  {
    case 12:
      cache = ascii12[ch - ' '];
      break;
    case 16:
      cache = ascii16[ch - ' '];
      break;
    case 24:
      cache = ascii24[ch - ' '];
      break;
  }
	for(i=0; i<csize; i++)
	{
    for(j=0; j<8; j++)
    {
      if(cache[i] & 0x80>>j) {
        memcpy(buff + (y-y0)*size + (x-x0)*2, &color, 2); 
      }
      y++;
      if(y - y0 >= size) {
       y = y0;
       x++;
       break;
      }
    }
	}
  LCD_FillBuffer(x0, y0, size / 2, size, buff);
  FREE(buff);
}

#ifdef __USE_CHN
void LCD_ShowChn(u16 x, u16 y, const char* ch, u8 size, u8 mode)
{
	u32 offset = 0, i, j;
	u16 csize, br, x0 = x, y0 = y;  //��ģ�ֽ���,f_read()�������ֽ�������ʼ����
  u16 color = FontColor;
	u8 gbh=0, gbl=0;  //����������ֽڣ�������ֽ�
  u8 *buff, *matrix;
  buff = (u8 *)MALLOC(size * size * 2);
  if (buff == 0) {
    return;
  }
	csize = (size/8 + !!(size%8)) * size;
  matrix = (u8 *)MALLOC(csize);
  if (matrix == 0) {
    FREE(buff);
    return;
  }
	gbh = ch[0], gbl = ch[1];
	if (gbl < 0x7F) {
		offset = ((gbh-0x81)*190 + gbl-0x40) * csize;
  }	else if(gbl >= 0x80) {
		offset = ((gbh-0x81)*190 + gbl-0x41) * csize;
  }
  if (mode) {
    LCD_ReadBuffer(x0, y0, size, size, buff);
  } else {
    memset(buff, BackColor, size * size * 2);
  }
	if (size == 16) {
		resff = f_open(&fileff, _T("0:/GBK16.DZK"), FA_READ | FA_OPEN_EXISTING);
  } else if (size == 12) {
		resff = f_open(&fileff, _T("0:/GBK12.DZK"), FA_READ | FA_OPEN_EXISTING);
  } else if (size == 24) {
		resff = f_open(&fileff, _T("0:/GBK24.DZK"), FA_READ | FA_OPEN_EXISTING);
  }
	if(resff == FR_OK)
	{
		f_lseek(&fileff, offset);  //�ƶ�����Ӧƫ����
		f_read(&fileff, matrix, csize, (UINT*)&br);
		if (br < csize) {
      f_close(&fileff);
      FREE(matrix);
      FREE(buff);
      return;  //���󷵻�
    }
		for(i=0; i<csize; i++)
		{
			/*for(j=0;j<8;j++)*/
			/*{*/
				/*if(matrix[i] & 0X80>>j)*/
					/*LCD_DrawPoint(x,y,FontColor);*/
				/*else if(!mode)*/
					/*LCD_DrawPoint(x,y,BackColor);*/
				/*x++;*/
				/*if(x-x0>=size)*/
				/*{*/
					/*x=x0;*/
					/*y++;*/
					/*break;*/
				/*}*/
			/*}*/
      for(j=0; j<8; j++)
      {
        if (matrix[i] & 0x80>>j) {
          memcpy(buff + (y-y0)*size*2 + (x-x0)*2, &color, 2);
        }
      }
		}
		f_close(&fileff);
	}
}
#endif

/*-----------------------------------------------------
 - Function Name: LCD_ShowStr
 - Description:   �ַ�����ʾ��֧����Ӣ�Ļ�ϵ��ַ�������
                  ֧���Զ����У����±߽�������
 - Input:         x,y - ����
                  str - �ַ������룬֧����Ӣ���
                  size - �����С
                  mode - 1 ���ӣ�0 �ǵ���
 - Output:        None
 - Return:        None
 - Attention:     ʹ�������ַ�Ҫ����__USE_CHN��
                  ע��LCD_ScreenDir()�趨����Ļ������
                  �ĸ�����ᵼ���ı�������ʾ
-----------------------------------------------------*/
void LCD_ShowStr(u16 x,u16 y,const char* str,u8 size,u8 mode)
{
	u16 x0=x;
	while(*str != '\0')
	{
		if(x>=lcd_param.width || y>=lcd_param.height) break;
		if(*str & 0x80)  //�����ı�
		{
#ifdef __USE_CHN
			LCD_ShowChn(x,y,str,size,mode);
			x = x+size;
			if(x+size>=lcd_param.width)
			{
				x=x0;
				y=y+size;
			}
			if(y+size>=lcd_param.height) break;  //������Χ��ֹͣ��ʾ
#endif
			str = str+2;
		}else  //Ӣ���ı�
		{
			LCD_ShowChar(x,y,*str,size,mode);
			x=x+size/2;
			if(x+size/2 >= lcd_param.width)
			{
				x=x0;
				y=y+size;
			}
			if(y+size >= lcd_param.height) break;  //������Χ��ֹͣ��ʾ
			str++;
		}
	}
}

/*������ת�����ַ�����valueΪ��������α�����s����ת�������radix = 10��ʾʮ���ƣ��������NULL*/
static char* inter2string(int value, char *s, int radix)
{
	char* tmp=s;
	int count=0,digit[12]={0};
	if(radix == 10)
	{
		if(value < 0)
		{
			value= -value;
			*tmp++ = '-';
		}
		do{
			digit[count]= value%10 + '0';
			value= value/10;
			count++;
		}while(value != 0);
		for(;count>0;count--)
		{
			*tmp = (char)digit[count-1];
			tmp++;
		}
		*tmp = '\0';  //�����ַ���������־
		return s;
	}
	else
	{
		*s= '\0';
		return NULL;
	}
} 

void LCD_ShowNum(u16 x,u16 y,int num,u8 length,u8 size,u8 mode)
{
	//32bit���α���ʮ���Ʊ�ʾ��󳤶�12���������ţ��ַ���������־��
	char num2str[12];
	u8 strlen=0,i;
	inter2string(num,num2str,10);
	while(num2str[strlen]!='\0') strlen++;
	if(strlen<length)
	{
		//���ո�ʹ�ַ����Ҷ���
		for(i=0;i<length-strlen;i++)
		{
			LCD_ShowChar(x,y,0X20,size,mode);  
			x+=size/2;
		}
		LCD_ShowStr(x,y,(const char*)num2str,size,mode);
	}
	else if(strlen==length){
		LCD_ShowStr(x,y,(const char*)num2str,size,mode);
	}
	else{
		for(i=0;length>2 && i<length-2;i++)
		{
			LCD_ShowChar(x,y,0X20,size,mode);
			x+=size/2;
		}
		LCD_ShowStr(x,y,(const char*)"OF",size,mode);
	}
}
