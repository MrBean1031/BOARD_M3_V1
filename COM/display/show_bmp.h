#ifndef __PICTURE_H
#define __PICTURE_H

#include "stm32f10x.h"
#include "ff.h"

typedef struct{
	DWORD bfSize;       //文件大小
	WORD  bfReserved1;  //保留字
	WORD  bfReserved2;
	DWORD bfOffset;     //实际位图起始地址，即位图文件头，位图信息头和颜色表长度之和
}BITMAPFILEHEADER;

typedef struct{
	DWORD biSize;           //结构体的长度，为40
	long  biWidth;          //位图宽，以像素为单位
	long  biHeight;         //位图高，以像素为单位
	WORD  biPlanes;         //平面数，强制为1
	WORD  biBitCount;       //采用颜色位数，取值有1，2，8，16，24
	DWORD biCompression;    //压缩方式，可以是0，1，2，0表示不压缩
	DWORD biSizeImage;      //实际位图数据占用字节数
	long  biXPelsPerMeter;  //X方向分辨率，像素每米
	long  biYPelsPerMeter;  //Y方向分辨率，像素每米
	DWORD biClrUsed;        //使用的颜色数，0表示默认值（2^biCount）
	DWORD biClrImportant;   //重要颜色数，0表示所有颜色都重要
}BITMAPINFOHEADER;

//BMP24到BMP16图像数据压缩
#define RGB24TORGB16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3))

//函数声明
void LCD_BmpPrepare(void);
u8 LCD_ShowBmp(u16 x, u16 y, TCHAR* path);
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,TCHAR* path);

#endif
