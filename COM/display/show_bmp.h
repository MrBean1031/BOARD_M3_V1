#ifndef __PICTURE_H
#define __SHOW_BMP_H

#include "stm32f10x.h"

typedef struct
{
	u32  bfSize;       //文件大小
	u16  bfReserved1;  //保留字
	u16  bfReserved2;
	u32  bfOffset;     //实际位图起始地址，即位图文件头，位图信息头和颜色表长度之和
} BitmapFileHdr;

typedef struct
{
	u32  biSize;           //结构体的长度，为40
	long biWidth;          //位图宽，以像素为单位
	long biHeight;         //位图高，以像素为单位
	u16  biPlanes;         //平面数，强制为1
	u16  biBitCount;       //采用颜色位数，取值有1，2，8，16，24
	u32  biCompression;    //压缩方式，可以是0，1，2，0表示不压缩
	u32  biSizeImage;      //实际位图数据占用字节数
	long biXPelsPerMeter;  //X方向分辨率，像素每米
	long biYPelsPerMeter;  //Y方向分辨率，像素每米
	u32  biClrUsed;        //使用的颜色数，0表示默认值（2^biCount）
	u32  biClrImportant;   //重要颜色数，0表示所有颜色都重要
} BitmapInfoHdr;

//BMP24到BMP16图像数据压缩
#define RGB24TORGB16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | ((b)>>3))

//函数声明
u8 LCD_ShowBmp(u16 x, u16 y, const char* path);
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,const char* path);

#endif
