#include "picture.h"
#include "lcd9341.h"
#include "usart.h"

#ifndef __USE_CHN
FATFS fsff;  //如果定义了__USE_CHN，则使用lcd9341.c定义的fsff，防止多次调用f_mount()
#else
extern FATFS fsff;  //如果定义了__USE_CHN，则fsff是来自lcd9341.c的外部变量
#endif
FIL filefp;
FILINFO fifp;
FRESULT resfp;

void LCD_BmpPrepare(void)
{
#ifndef __USE_CHN
	f_mount(&fsff,_T("0:"),1);
#endif
}

/*-----------------------------------------------------
 - Function Name: LCD_ShowBmp
 - Description:   LCD显示24bit真彩色bmp图片
 - Input:         x,y - 坐标
                  path - 图片路径
 - Output:        None
 - Return:        0 - succeed
                  1 - 文件类型不匹配
                  2 - 文件宽度大于320
                  3 - 图片显示范围超区
                  4 - 非24位色位图
 - Attention:     调用本函数前必须注册drive工作区
                  注意BMP扫描方式是从左到右，从下到上
-----------------------------------------------------*/
u8 LCD_ShowBmp(u16 x, u16 y, TCHAR* path)
{
	int i,j;
	u8 red,green,blue,bmpbuf[960]={0};  //每行最大支持320像素*24位色
	u16 l_width;
	UINT br;
	BITMAPFILEHEADER bmpfh;
	BITMAPINFOHEADER bmpih;
	BYTE  fileType[2];  //bmp文件必须为0X42,0X4D,即BM
	
	resfp = f_open(&filefp,path,FA_READ|FA_OPEN_EXISTING);
	if(resfp==FR_OK)
	{
		f_read(&filefp,fileType,2,&br);
		if(fileType[0]!=0X42 || fileType[1]!=0X4D)
		{
			f_close(&filefp);
			return 1;  //文件类型不匹配
		}
		f_read(&filefp,&bmpfh,12,&br);
		f_read(&filefp,&bmpih,40,&br);
		//计算行字节数，必须是4的整数倍，不足末尾补0
		if(bmpih.biBitCount==24)
			l_width = bmpih.biWidth * (bmpih.biBitCount/8) + bmpih.biWidth%4;
		if(l_width>960)
		{
			f_close(&filefp);
			return 2;
		}
		if(LCD_SetWindow(x, lcd_param.height-y-bmpih.biHeight, bmpih.biWidth, bmpih.biHeight))
		{
			f_close(&filefp);
			return 3;  //超出屏幕范围退出
		}
		else
		{
			if(bmpih.biBitCount!=24) 
			{
				f_close(&filefp);
				return 4;  //非24位色退出
			}		
			LCD_WR_REG(0x2C);  //Memroy Write;
			for(i=0; i<bmpih.biHeight; i++)
			{
				f_read(&filefp, bmpbuf, l_width, &br);  //读出一行RGB数据
				for(j=0; j<bmpih.biWidth; j++)  //填充列像素点
				{
					red   = bmpbuf[3*j+2];
					green = bmpbuf[3*j+1];
					blue  = bmpbuf[3*j+0];
					LCD_WR_DATA(RGB24TORGB16(red,green,blue));
				}
			}
			f_close(&filefp);
		}
	}
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
                  1 - fail
 - Attention:     注意截屏范围不能超出LCD坐标范围
                  截图前先设置LCD扫描方向
-----------------------------------------------------*/
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,TCHAR* path)
{
	int i,j;
	BITMAPFILEHEADER bmpfh;
	BITMAPINFOHEADER bmpih;
	BYTE  fileType[2];  //bmp文件必须为0X42,0X4D,即BM
	u8 red,green,blue,linebuf[960];
	u16 l_width,color,bw;
	
	//截图范围超区判定
	if(x>=lcd_param.width || y>=lcd_param.height) return 1;
	if(x+width-1>=lcd_param.width || y+height-1>=lcd_param.height) return 1;
	
	//位图文件头
	fileType[0]=0x42,fileType[1]=0X4D;
	l_width = width*3 + width%4;  //一个扫描行的字节数必须是4的整数倍
	bmpfh.bfSize = 54+l_width*height;
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
	bmpih.biSizeImage = l_width*height;
	bmpih.biXPelsPerMeter = 3780;
	bmpih.biYPelsPerMeter = 3780;
	bmpih.biClrUsed = 0;
	bmpih.biClrImportant = 0;
	
	resfp = f_open(&filefp, path, FA_WRITE|FA_CREATE_ALWAYS);
	if(resfp!=FR_OK) return 1;  //打开文件失败
	f_write(&filefp,fileType,2,(UINT*)&bw);
	f_write(&filefp,&bmpfh,12,(UINT*)&bw);  //写入位图文件头
	f_write(&filefp,&bmpih,40,(UINT*)&bw);  //写入位图信息头
	for(i=0;i<height;i++) //对一整行操作
	{
		for(j=0;j<width;j++)
		{
			color = LCD_ReadPoint(x+j,lcd_param.height-y-height+i);
			red = color>>11<<3;        //8bit R
			green = (color&0X07E0)>>3; //8bit G
			blue = (color&0X1F)<<3;    //8bit B
			linebuf[3*j+0] = blue;
			linebuf[3*j+1] = green;
			linebuf[3*j+2] = red;
		}
		for(j=3*width; j<l_width; j++)  //字节数不足4的整数倍末尾补0
			linebuf[j] = 0;
		f_write(&filefp,linebuf,l_width,(UINT*)&bw);
	}
	f_close(&filefp);
	return 0;
}
