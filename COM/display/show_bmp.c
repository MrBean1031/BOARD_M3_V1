#include "picture.h"
#include "lcd9341.h"
#include "usart.h"

#ifndef __USE_CHN
FATFS fsff;  //���������__USE_CHN����ʹ��lcd9341.c�����fsff����ֹ��ε���f_mount()
#else
extern FATFS fsff;  //���������__USE_CHN����fsff������lcd9341.c���ⲿ����
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
 - Description:   LCD��ʾ24bit���ɫbmpͼƬ
 - Input:         x,y - ����
                  path - ͼƬ·��
 - Output:        None
 - Return:        0 - succeed
                  1 - �ļ����Ͳ�ƥ��
                  2 - �ļ���ȴ���320
                  3 - ͼƬ��ʾ��Χ����
                  4 - ��24λɫλͼ
 - Attention:     ���ñ�����ǰ����ע��drive������
                  ע��BMPɨ�跽ʽ�Ǵ����ң����µ���
-----------------------------------------------------*/
u8 LCD_ShowBmp(u16 x, u16 y, TCHAR* path)
{
	int i,j;
	u8 red,green,blue,bmpbuf[960]={0};  //ÿ�����֧��320����*24λɫ
	u16 l_width;
	UINT br;
	BITMAPFILEHEADER bmpfh;
	BITMAPINFOHEADER bmpih;
	BYTE  fileType[2];  //bmp�ļ�����Ϊ0X42,0X4D,��BM
	
	resfp = f_open(&filefp,path,FA_READ|FA_OPEN_EXISTING);
	if(resfp==FR_OK)
	{
		f_read(&filefp,fileType,2,&br);
		if(fileType[0]!=0X42 || fileType[1]!=0X4D)
		{
			f_close(&filefp);
			return 1;  //�ļ����Ͳ�ƥ��
		}
		f_read(&filefp,&bmpfh,12,&br);
		f_read(&filefp,&bmpih,40,&br);
		//�������ֽ�����������4��������������ĩβ��0
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
			return 3;  //������Ļ��Χ�˳�
		}
		else
		{
			if(bmpih.biBitCount!=24) 
			{
				f_close(&filefp);
				return 4;  //��24λɫ�˳�
			}		
			LCD_WR_REG(0x2C);  //Memroy Write;
			for(i=0; i<bmpih.biHeight; i++)
			{
				f_read(&filefp, bmpbuf, l_width, &br);  //����һ��RGB����
				for(j=0; j<bmpih.biWidth; j++)  //��������ص�
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
 - Description:   ��ȡ��ǰ��Ļ���ݣ�������Ϊ.bmp�ļ�
 - Input:         x,y - ����
                  width - �������
                  height - �����߶�
                  path - �ļ�·��
 - Output:        None
 - Return:        0 - succeed
                  1 - fail
 - Attention:     ע�������Χ���ܳ���LCD���귶Χ
                  ��ͼǰ������LCDɨ�跽��
-----------------------------------------------------*/
u8 LCD_ScreenShot(u16 x,u16 y,u16 width,u16 height,TCHAR* path)
{
	int i,j;
	BITMAPFILEHEADER bmpfh;
	BITMAPINFOHEADER bmpih;
	BYTE  fileType[2];  //bmp�ļ�����Ϊ0X42,0X4D,��BM
	u8 red,green,blue,linebuf[960];
	u16 l_width,color,bw;
	
	//��ͼ��Χ�����ж�
	if(x>=lcd_param.width || y>=lcd_param.height) return 1;
	if(x+width-1>=lcd_param.width || y+height-1>=lcd_param.height) return 1;
	
	//λͼ�ļ�ͷ
	fileType[0]=0x42,fileType[1]=0X4D;
	l_width = width*3 + width%4;  //һ��ɨ���е��ֽ���������4��������
	bmpfh.bfSize = 54+l_width*height;
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
	bmpih.biSizeImage = l_width*height;
	bmpih.biXPelsPerMeter = 3780;
	bmpih.biYPelsPerMeter = 3780;
	bmpih.biClrUsed = 0;
	bmpih.biClrImportant = 0;
	
	resfp = f_open(&filefp, path, FA_WRITE|FA_CREATE_ALWAYS);
	if(resfp!=FR_OK) return 1;  //���ļ�ʧ��
	f_write(&filefp,fileType,2,(UINT*)&bw);
	f_write(&filefp,&bmpfh,12,(UINT*)&bw);  //д��λͼ�ļ�ͷ
	f_write(&filefp,&bmpih,40,(UINT*)&bw);  //д��λͼ��Ϣͷ
	for(i=0;i<height;i++) //��һ���в���
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
		for(j=3*width; j<l_width; j++)  //�ֽ�������4��������ĩβ��0
			linebuf[j] = 0;
		f_write(&filefp,linebuf,l_width,(UINT*)&bw);
	}
	f_close(&filefp);
	return 0;
}
