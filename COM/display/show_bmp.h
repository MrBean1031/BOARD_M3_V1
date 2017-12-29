#ifndef __PICTURE_H
#define __PICTURE_H

#include "stm32f10x.h"
#include "ff.h"

typedef struct{
	DWORD bfSize;       //�ļ���С
	WORD  bfReserved1;  //������
	WORD  bfReserved2;
	DWORD bfOffset;     //ʵ��λͼ��ʼ��ַ����λͼ�ļ�ͷ��λͼ��Ϣͷ����ɫ����֮��
}BITMAPFILEHEADER;

typedef struct{
	DWORD biSize;           //�ṹ��ĳ��ȣ�Ϊ40
	long  biWidth;          //λͼ��������Ϊ��λ
	long  biHeight;         //λͼ�ߣ�������Ϊ��λ
	WORD  biPlanes;         //ƽ������ǿ��Ϊ1
	WORD  biBitCount;       //������ɫλ����ȡֵ��1��2��8��16��24
	DWORD biCompression;    //ѹ����ʽ��������0��1��2��0��ʾ��ѹ��
	DWORD biSizeImage;      //ʵ��λͼ����ռ���ֽ���
	long  biXPelsPerMeter;  //X����ֱ��ʣ�����ÿ��
	long  biYPelsPerMeter;  //Y����ֱ��ʣ�����ÿ��
	DWORD biClrUsed;        //ʹ�õ���ɫ����0��ʾĬ��ֵ��2^biCount��
	DWORD biClrImportant;   //��Ҫ��ɫ����0��ʾ������ɫ����Ҫ
}BITMAPINFOHEADER;

//BMP24��BMP16ͼ������ѹ��
#define RGB24TORGB16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3))

//��������
void LCD_BmpPrepare(void);
u8 LCD_ShowBmp(u16 x, u16 y, TCHAR* path);
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,TCHAR* path);

#endif
