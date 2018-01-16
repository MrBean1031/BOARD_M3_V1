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
 - Description:   LCD显示24bit真彩色bmp图片
 - Input:         x,y - 坐标
                  path - 图片路径
 - Output:        None
 - Return:        0 - succeed
                  1 - 文件访问错误
                  2 - 文件类型不匹配
                  3 - 非24位色位图
                  4 - 内存错误
                  5 - 图片显示范围超区
 - Attention:     1. 调用本函数前必须注册fs工作区
                  2. 注意BMP扫描方式是从左到右，从下到上，需
                  要先设置屏幕方向
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
    return 1;  //打开文件失败
	}
  f_read(&filefp, &bmpfh, sizeof(BitmapFileHdr), &br);
  if(bmpfh.filetype[0] != 0x42 || bmpfh.filetype[1] != 0x4d)
  {
    f_close(&filefp);
    return 2;  //文件类型不匹配
  }
  f_read(&filefp, &bmpih, sizeof(BitmapInfoHdr), &br);
  //计算行字节数，必须是4的整数倍，不足末尾补0
  if(bmpih.biBitCount != 24)
  {
    f_close(&filefp);
    return 3;  //非24位色
  }
  l_width = bmpih.biWidth * (bmpih.biBitCount/8) + bmpih.biWidth%4;
  linebuf = (u8 *)MALLOC(l_width);
  if(linebuf == NULL)
  {
    f_close(&filefp);
    return 4;  //内存错误
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
      f_read(&filefp, linebuf + j, len, &br);  //读出一行RGB数据
      j += len;
    } while(j < l_width && br > 0);
    for(j=0; j < bmpih.biWidth; j++)  //填充列像素点
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
 - Description:   截取当前屏幕内容，并保存为.bmp文件
 - Input:         x,y - 坐标
                  width - 截屏宽度
                  height - 截屏高度
                  path - 文件路径
 - Output:        None
 - Return:        0 - succeed
                  1 - 图像范围超出显示区域
                  2 - 文件访问错误
                  3 - 内存错误
 - Attention:     1. 注意截屏范围不能超出LCD坐标范围
                  2. 截图前先设置LCD扫描方向
                  3. 调用本函数前必须注册fs工作区
-----------------------------------------------------*/
u8 LCD_ScreenShot(u16 x, u16 y, u16 width, u16 height, const char* path)
{
	int i, j;
	BitmapFileHdr bmpfh;
	BitmapInfoHdr bmpih;
	u8 red, green, blue, *linebuf;
	u16 l_width, color, len;
  UINT bw;
	
	//截图范围超区判定
	if(x >= lcd_param.width || y >= lcd_param.height)
    return 1;
	if(x+width-1 >= lcd_param.width || y+height-1 >= lcd_param.height)
    return 1;
	
	//位图文件头
  bmpfh.filetype[0] = 0x42;
  bmpfh.filetype[1] = 0x4d;
	l_width = width*3 + width%4;  //一个扫描行的字节数必须是4的整数倍
	bmpfh.bfSize = 54 + l_width*height;
	bmpfh.bfReserved1 = 0;
	bmpfh.bfReserved2 = 0;
	bmpfh.bfOffset = 54;
	//位图信息头
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
    return 2;  //打开文件失败
  linebuf = (u8 *)MALLOC(l_width);
  if(linebuf == NULL)
  {
    f_close(&filefp);
    return 3;  //内存错误
  }
	f_write(&filefp, &bmpfh, sizeof(BitmapFileHdr), &bw);  //写入位图文件头
	f_write(&filefp, &bmpih, sizeof(BitmapInfoHdr), &bw);  //写入位图信息头
	for(i=0; i < height; i++) //对一整行操作
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
		for(j = 3*width; j < l_width; j++)  //字节数不足4的整数倍末尾补0
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
