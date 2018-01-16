#include "show_bmp.h"
#include "lcd9341_fsmc.h"
#include "usart.h"
#include "ff.h"
#include "global.h"

FIL filefp;
FILINFO fifp;
FRESULT resfp;

#define MALLOC(size) malloc_safe(size)
#define FREE(ptr)    free_safe(ptr)


/*-----------------------------------------------------
 - Function Name: LCD_ShowBmp
 - Description:   LCD��ʾ24bit���ɫbmpͼƬ
 - Input:         x,y - ����
                  path - ͼƬ·��
 - Output:        None
 - Return:        0 - succeed
                  1 - �ļ����ʴ���
                  2 - �ļ����Ͳ�ƥ��
                  3 - ��24λɫλͼ
                  4 - �ڴ����
                  5 - ͼƬ��ʾ��Χ����
 - Attention:     1. ���ñ�����ǰ����ע��fs������
                  2. ע��BMPɨ�跽ʽ�Ǵ����ң����µ��ϣ���
                  Ҫ��������Ļ����
-----------------------------------------------------*/
u8 LCD_ShowBmp(u16 x, u16 y, const char* path)
{
	int i, j;
	u8 red, green, blue, *linebuf;
	u16 l_width, len;
	UINT br;
	BitmapFileHdr bmpfh;
	BitmapInfoHdr bmpih;
	
	resfp = f_open(&filefp, path, FA_READ | FA_OPEN_EXISTING);
	if(resfp != FR_OK)
	{
    return 1;  //���ļ�ʧ��
	}
  f_read(&filefp, &bmpfh, sizeof(BitmapFileHdr), &br);
  if(bmpfh.filetype[0] != 0x42 || bmpfh.filetype[1] != 0x4d)
  {
    f_close(&filefp);
    return 2;  //�ļ����Ͳ�ƥ��
  }
  f_read(&filefp, &bmpih, sizeof(BitmapInfoHdr), &br);
  //�������ֽ�����������4��������������ĩβ��0
  if(bmpih.biBitCount != 24)
  {
    f_close(&filefp);
    return 3;  //��24λɫ
  }
  l_width = bmpih.biWidth * (bmpih.biBitCount/8) + bmpih.biWidth%4;
  linebuf = (u8 *)MALLOC(l_width);
  if(linebuf == NULL)
  {
    f_close(&filefp);
    return 4;  //�ڴ����
  }
  f_lseek(&filefp, bmpfh.bfOffset);
  i = LCD_SetWindow(x, lcd_param.height-y-bmpih.biHeight, bmpih.biWidth, bmpih.biHeight);
  if(i)
  {
    FREE(linebuf);
    f_close(&filefp);
    return 5;
  }
  LCD_WR_REG(0x2C);  //Memroy Write;
  for(i=0; i < bmpih.biHeight; i++)
  {
    j = 0;
    do {
      len = l_width - j > 512 ? 512 : l_width - j;
      f_read(&filefp, linebuf + j, len, &br);  //����һ��RGB����
      j += len;
    } while(j < l_width && br > 0);
    for(j=0; j < bmpih.biWidth; j++)  //��������ص�
    {
      red   = linebuf[3*j+2];
      green = linebuf[3*j+1];
      blue  = linebuf[3*j+0];
      LCD_WR_DATA(RGB24TORGB16(red,green,blue));
    }
  }
  FREE(linebuf);
  f_close(&filefp);
	return 0;
}

/*-----------------------------------------------------
 - Function Name: LCD_ScreenShot
 - Description:   ��ȡ��ǰ��Ļ���ݣ�������Ϊ.bmp�ļ�
 - Input:         x,y - ����
                  width - �������
                  height - �����߶�
                  path - �ļ�·��
 - Output:        None
 - Return:        0 - succeed
                  1 - ͼ��Χ������ʾ����
                  2 - �ļ����ʴ���
                  3 - �ڴ����
 - Attention:     1. ע�������Χ���ܳ���LCD���귶Χ
                  2. ��ͼǰ������LCDɨ�跽��
                  3. ���ñ�����ǰ����ע��fs������
-----------------------------------------------------*/
u8 LCD_ScreenShot(u16 x, u16 y, u16 width, u16 height, const char* path)
{
	int i, j;
	BitmapFileHdr bmpfh;
	BitmapInfoHdr bmpih;
	u8 red, green, blue, *linebuf;
	u16 l_width, color, len;
  UINT bw;
	
	//��ͼ��Χ�����ж�
	if(x >= lcd_param.width || y >= lcd_param.height)
    return 1;
	if(x+width-1 >= lcd_param.width || y+height-1 >= lcd_param.height)
    return 1;
	
	//λͼ�ļ�ͷ
  bmpfh.filetype[0] = 0x42;
  bmpfh.filetype[1] = 0x4d;
	l_width = width*3 + width%4;  //һ��ɨ���е��ֽ���������4��������
	bmpfh.bfSize = 54 + l_width*height;
	bmpfh.bfReserved1 = 0;
	bmpfh.bfReserved2 = 0;
	bmpfh.bfOffset = 54;
	//λͼ��Ϣͷ
	bmpih.biSize = 40;
	bmpih.biWidth = width;
	bmpih.biHeight = height;
	bmpih.biPlanes = 1;
	bmpih.biBitCount = 24;
	bmpih.biCompression = 0;
	bmpih.biSizeImage = l_width * height;
	bmpih.biXPelsPerMeter = 3780;
	bmpih.biYPelsPerMeter = 3780;
	bmpih.biClrUsed = 0;
	bmpih.biClrImportant = 0;
	
	resfp = f_open(&filefp, path, FA_WRITE | FA_CREATE_ALWAYS);
	if(resfp != FR_OK)
    return 2;  //���ļ�ʧ��
  linebuf = (u8 *)MALLOC(l_width);
  if(linebuf == NULL)
  {
    f_close(&filefp);
    return 3;  //�ڴ����
  }
	f_write(&filefp, &bmpfh, sizeof(BitmapFileHdr), &bw);  //д��λͼ�ļ�ͷ
	f_write(&filefp, &bmpih, sizeof(BitmapInfoHdr), &bw);  //д��λͼ��Ϣͷ
	for(i=0; i < height; i++) //��һ���в���
	{
		LCD_ReadBuffer(x, lcd_param.height-y-height+i, width, 1, linebuf);
		for(j=width; j>0; j--)
		{
      color = *(u16 *)&linebuf[(j-1)*2];
			red = (color>>11)<<3;      //8bit R
			green = (color&0X07E0)>>3; //8bit G
			blue = (color&0X1F)<<3;    //8bit B
			linebuf[3*(j-1)+2] = red;
			linebuf[3*(j-1)+1] = green;
			linebuf[3*(j-1)+0] = blue;
		}
		for(j = 3*width; j < l_width; j++)  //�ֽ�������4��������ĩβ��0
			linebuf[j] = 0;
    j = 0;
    do {
      len = l_width - j > 512 ? 512 : l_width - j;
      f_write(&filefp, linebuf + j, len, &bw);
      j += len;
    } while(j < l_width && bw > 0);
	}
  FREE(linebuf);
	f_close(&filefp);
	return 0;
}
