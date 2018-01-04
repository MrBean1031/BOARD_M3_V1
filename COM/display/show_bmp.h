#ifndef __PICTURE_H
#define __SHOW_BMP_H

#include "stm32f10x.h"

typedef struct
{
	u32  bfSize;       //�ļ���С
	u16  bfReserved1;  //������
	u16  bfReserved2;
	u32  bfOffset;     //ʵ��λͼ��ʼ��ַ����λͼ�ļ�ͷ��λͼ��Ϣͷ����ɫ����֮��
} BitmapFileHdr;

typedef struct
{
	u32  biSize;           //�ṹ��ĳ��ȣ�Ϊ40
	long biWidth;          //λͼ��������Ϊ��λ
	long biHeight;         //λͼ�ߣ�������Ϊ��λ
	u16  biPlanes;         //ƽ������ǿ��Ϊ1
	u16  biBitCount;       //������ɫλ����ȡֵ��1��2��8��16��24
	u32  biCompression;    //ѹ����ʽ��������0��1��2��0��ʾ��ѹ��
	u32  biSizeImage;      //ʵ��λͼ����ռ���ֽ���
	long biXPelsPerMeter;  //X����ֱ��ʣ�����ÿ��
	long biYPelsPerMeter;  //Y����ֱ��ʣ�����ÿ��
	u32  biClrUsed;        //ʹ�õ���ɫ����0��ʾĬ��ֵ��2^biCount��
	u32  biClrImportant;   //��Ҫ��ɫ����0��ʾ������ɫ����Ҫ
} BitmapInfoHdr;

//BMP24��BMP16ͼ������ѹ��
#define RGB24TORGB16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3))

//��������
u8 LCD_ShowBmp(u16 x, u16 y, const char* path);
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,const char* path);

#endif
