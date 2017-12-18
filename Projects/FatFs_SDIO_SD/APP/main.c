#include "stm32f10x.h"
#include "led.h"
#include "usart.h"
#include "SysTick.h"
#include "spi_flash.h"
#include "ff.h"
#include <string.h>

#define FORMAT_BUFF_SIZE  4096
uint8_t format_buff[FORMAT_BUFF_SIZE];

uint8_t buf_in[512], buf_out[512];

#if _LFN_UNICODE==1
void StrConvert_GBK2Uni(char* psrc, u16* pdst)
{
	int i=0,j=0;  //i指向psrc当前位置，j指向pdst当前位置
	u16 temp;
	while(*(psrc+i) != '\0')
	{
		if(*(psrc+i) & 0X80)  //GBK
		{
			temp = (*(psrc+i)<<8) + *(psrc+i+1);
			*(pdst+j) = ff_convert(temp, 1);  //oem to unicode
			i = i+2;
		}
		else  //ASCII
		{
			*(pdst+j) = ff_convert(*(psrc+i), 1);
			i++;
		}
		j++;
	}
	*(pdst+j)='\0';  //末尾添加字符串结束标志
}
#endif

#if _LFN_UNICODE==1
void StrConvert_Uni2GBK(u16 *psrc, char * pdst)
{
	int i=0,j=0;  //i指向psrc当前位置，j指向pdst当前位置
	u16 temp;
	temp = ff_convert(*(psrc+i),0);  //unicode to oem
	while(temp !='\0')
	{
		//注意temp的字节序问题
		if(temp&0X8000)
		{
			*(pdst+j)   = (temp&0XFF00)>>8;
			*(pdst+j+1) = temp&0X00FF;
			j=j+2;
		}
		else
		{
			*(pdst+j) = temp;
			j=j+1;
		}
		i=i+1;
		temp = ff_convert(*(psrc+i),0);  //unicode to oem
	}
	*(pdst+j)='\0';  //末尾添加字符串结束标志
}
#endif

void fat_readtest()
{
	FIL myfile;
	FRESULT result;
	FILINFO fileinfo;
	TCHAR* pstr;  //用于输出宽字符串的指针
	uint8_t readbuf[512];
	UINT br;
	
  puts("\r\n-- fat read test --\r\n");
	result = f_open(&myfile, _T("0:/TDA2030_Information.LOG"), FA_READ|FA_OPEN_EXISTING);  //打开文件
	if(result != FR_OK)
  {
		printf("Open file error, error code %d\r\n", result);
    return;
  }
	
	f_stat(_T("0:/TDA2030_Information.LOG"), &fileinfo);  //获取文件信息
	pstr = fileinfo.fname;
	printf("File name: ");
	while(*pstr != 0)
	{
		putchar(*pstr);
		pstr += 1;
	}
	puts("\r\n");
	printf("File size in byte: %lu\r\n", fileinfo.fsize);
//	printf("File modified date : %4X\r\n", fileinfo.fdate);
//	printf("File modified time : %4X\r\n", fileinfo.ftime);
	printf("File modified date: %04u/%02u/%02u\r\n", 
				 (fileinfo.fdate>>9)+1980, fileinfo.fdate>>5 & 0X0F, fileinfo.fdate & 0X1F);
	printf("File modified time: %02u:%02u:%02u\r\n", 
					fileinfo.ftime>>11, fileinfo.ftime>>5 & 0X1F, (fileinfo.ftime & 0X1F)*2);
	printf("File attribute: %X\r\n", fileinfo.fattrib);
	
	f_read(&myfile, readbuf, 512, &br);
	while(br == 512)
	{
		printf("%s", readbuf);
		f_read(&myfile, readbuf, 512, &br);
	}
	readbuf[br] = 0;  //添加字符串结束标志
	printf("%s\r\n",readbuf);
	f_close(&myfile);
}

void fat_writetest()
{
	FIL myfile;
	FRESULT result;
	uint8_t writebuf[512];
	UINT bw;
	TCHAR *pstr;
	uint16_t i,j;
	FILINFO fileinfo;
	
	printf("\r\n-- fat write test --\r\n");
	result = f_open(&myfile, _T("0:/WriteSomething.txt"), FA_WRITE|FA_CREATE_ALWAYS);
	if(result != FR_OK)
  {
		printf("Open file fail,error code %d\r\n", result);
    return;
  }

  printf("Open file succeed.\r\n");
  for(i=0; i<16; i++)  //写两个簇
  {
    for(j=i*512; j<(i+1)*512; j++)
      writebuf[j%512] = j%26 + 65;  //循环打印大写英文字母
    result = f_write(&myfile, writebuf, 512, &bw);
    //printf("byte written %d, result %d\r\n", bw, result);
    //f_sync(&myfile);  //刷新缓冲区
  }
  f_close(&myfile);
  printf("End of writing test.\r\n");
  
  f_stat(_T("0:/WriteSomething.txt"), &fileinfo);
  printf("Long file name: ");
  pstr = fileinfo.fname;
  while(*pstr != 0)
  {
    putchar(*pstr);
    pstr += 1;
  }
  puts("\r\n");
#if _USE_LFN != 0
    printf("Short file name: ");
    pstr = fileinfo.altname;
    while(*pstr != 0)
    {
      putchar(*pstr);
      pstr += 1;
    }
    puts("\r\n");
#endif
  
  printf("File size in byte: %lu\r\n", fileinfo.fsize);
  printf("File attribute: 0x%X\r\n", fileinfo.fattrib);
}

void fat_fileoperate()
{
	FIL myfile;
	const BYTE teststr[] = "\r\nI'm a new line after truncating process.";
	UINT brw;
	uint8_t readbuf[512];
#if _LFN_UNICODE==1
	u16 str_chn[20];
#endif
	
  puts("\r\n-- file operation test --\r\n");
	//f_lseek() and f_truncate() operation
	f_open(&myfile, _T("0:/WriteSomething.txt"), FA_WRITE|FA_READ|FA_OPEN_EXISTING);
	f_lseek(&myfile, 2000);
	f_truncate(&myfile);  //在2000字节处截断文件
	f_write(&myfile, teststr, sizeof(teststr), &brw);
	if(brw != sizeof(teststr))
		printf("Fail to write the string.\r\n");
	f_lseek(&myfile, myfile.fptr - sizeof(teststr));
	f_read(&myfile, readbuf, sizeof(teststr), &brw);
	if(brw == sizeof(teststr)){
		printf("Read after write:%s\r\n", (char *)readbuf);
	}	
	f_close(&myfile);
	
	//文件删除，属性修改，重命名
	f_open(&myfile, _T("0:/file1.txt"), FA_CREATE_ALWAYS);
	f_close(&myfile); //删除文件前先关闭文件
	f_unlink(_T("0:/file1.txt"));
	f_chmod(_T("0:/file2.txt"), AM_RDO, AM_RDO|AM_ARC);  //置位只读属性，清除存档属性
	f_open(&myfile, _T("0:/file3.txt"), FA_CREATE_ALWAYS);
	f_close(&myfile);
#if _LFN_UNICODE==1
		StrConvert_GBK2Uni("0:/English中文.txt", str_chn);
		f_rename(_T("0:/file3.txt"), str_chn);
#else
  f_rename(_T("0:/file3.txt"), "0:/GBK编码文件名.txt");
#endif
}

void fat_diroperate()
{
	DIR mydir;
	FILINFO fileinfo;
	char str[40]={0};
	
	//文件夹操作
	printf("\r\n-- fat folder operation test --\r\n");
	f_mkdir(_T("0:/New_Folder"));
	f_opendir(&mydir, _T("0:/"));  //open root directory
	while(f_readdir(&mydir, &fileinfo) == FR_OK)
	{
		if(!fileinfo.fname[0]) break;  //所有文件检索完成
#if _LFN_UNICODE==1
		StrConvert_Uni2GBK(fileinfo.fname,str);
#else
		strcpy(str, fileinfo.fname);
#endif
		printf("%s\r\n", str);
	}
  f_closedir(&mydir);
}


#define FLASH_UNIGBK_BASE_ADDR  4194304UL  //UNI-GBK双向转换表基地址(字节为单位)
#define UNIGBK_SIZE  174336              //unigbk.bin文件大小 
uint8_t fbuff[4096];
void fat_update_uni2gbk(TCHAR* path)
{
	FIL updatefile;
	FILINFO fileinfo;
	FRESULT res;
	UINT br,offset=0;
	
	res = f_open(&updatefile, path, FA_READ|FA_OPEN_EXISTING);
	if(res != FR_OK)
	{
    puts("\r\nfat_update_uni2gbk(): fail to open file!\r\n");
    return;
	}
  printf("\r\nUpdating codepage\r\n");
  f_stat(path, &fileinfo);
  do {
    res = f_read(&updatefile, fbuff, 4096, &br);  //read from SD card
    if(res != FR_OK) 
    {
      puts("fat_update_uni2gbk(): error while reading\r\n");
      f_close(&updatefile);
      return;
    }
    SPI_Flash_WriteWithErase(fbuff, FLASH_UNIGBK_BASE_ADDR+offset, br);  
    offset += br;
    printf("%d%% -> ", (int)((float)updatefile.fptr/fileinfo.fsize*100));
  } while(f_eof(&updatefile) == 0);
  printf("\r\nComplete to update codepage from file %s.\r\n", path);
  f_close(&updatefile);
}

void buffer_fill(uint8_t *buf, uint32_t bufsize, uint8_t offset)
{
  uint32_t i;
  
  for(i=0; i<bufsize; i++)
  {
    *(buf + i) = offset + i;
  }
}

int buffer_compare(uint8_t *buf1, uint8_t *buf2, uint32_t bufsize)
{
  while(bufsize--)
  {
    if(*buf1++ != *buf2++)
    {
      return 0;
    }
  }
  return 1;
}

void fat_flashtest()
{
	FIL myfile;
	DIR mydir;
	FILINFO fileinfo;
	FRESULT result;
  UINT br,bw;

	char str[40];  //文件夹索引时的临时文件名
	
	printf("\r\n-- flash operation test --\r\n");	
	result = f_open(&myfile, _T("1:/yatou.txt"), FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
	if(result != FR_OK)
	{
 		printf("Open file fail, error code: %d\r\n", result);
    return;
  }
  printf("Open file in flash succeed.\r\n");
  buffer_fill(buf_in, 512, 0);
  puts("a");
  result = f_write(&myfile, buf_in, 512, &bw);
  puts("b");
  f_lseek(&myfile, 0);  //将文件读写指针归零
  puts("c");
  result = f_read(&myfile, buf_out, 512, &br);
  puts("d\r\n");
  if(bw && br && bw == br && buffer_compare(buf_in, buf_out, 512)) {
    puts("fat_flashtest(): read/write test past\r\n");
  } else {
    puts("fat_flashtest(): read/write test fail\r\n");
  }
  f_close(&myfile);
  
  //flash directory operation
  printf("\r\n--read dir from spi flash--\r\n");
  f_opendir(&mydir, _T("1:/"));  //open root directory
  while(f_readdir(&mydir, &fileinfo) == FR_OK)
  {
    if(!fileinfo.fname[0]) break;  //所有文件检索完成
#if _LFN_UNICODE==1
    StrConvert_Uni2GBK(fileinfo.fname, str);
#else
    strcpy(str, fileinfo.fname);
#endif
    printf("%s\r\n",str);
  }
  f_close(&myfile);
}

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{
	FATFS sd_card, flash;
	FRESULT result = FR_NOT_READY;
	DWORD fre_cluster, total_cluster;
	FATFS* fs;
	
  NVIC_Configuration();
	LED_Config();
	USART_Config();
	SysTick_Init();
  
	result = f_mount(&sd_card, _T("0:"), 1);  //挂载SD卡
	if(result != FR_OK) {
		printf("Mount SD_CARD failed, error code %02x\r\n", result); 
    if(result == FR_NO_FILESYSTEM) {
      result = f_mkfs(_T("0:"), FM_FAT | FM_FAT32, 4096, format_buff, FORMAT_BUFF_SIZE);
      if(result != FR_OK) {
        printf("format sd card error, error code %02x\r\n", result);
        goto __jump1;
      } else {
        result = f_mount(&sd_card, _T("0:"), 1);
        if(result != FR_OK) {
          printf("remount sd card fail %d\r\n", result);
          goto __jump1;
        }
      }
    } else {
      goto __jump1;
    }
  }
  printf("Mount SD_CARD succeed.\r\n");
  //获取空闲簇数
  f_getfree(_T("0:"), &fre_cluster, &fs);
  total_cluster = fs->n_fatent;
  printf("free cluster: %lu, total cluster: %lu, used: %ld%%\r\n",
         fre_cluster, total_cluster, (total_cluster - fre_cluster) * 100 / total_cluster);
  printf("Capacity: %lu KB\r\n", total_cluster * fs->csize / 2);
  //fat_readtest();
  //fat_writetest();
  //fat_fileoperate();
  //fat_diroperate();
  
__jump1:
	result = f_mount(&flash, _T("1:"), 1);  //挂载FLASH
  if (result != FR_OK) {
    printf("Mount flash disk fail, %02x\r\n", result);
    if (result == FR_NO_FILESYSTEM) {
      printf("no filesystem, formating flash disk ...\r\n");
      result = f_mkfs(_T("1:"), FM_FAT, 4096, format_buff, FORMAT_BUFF_SIZE);
      if (result != FR_OK) {
        printf("format flash disk fail\r\n");
        goto __jump2;
      }
      result = f_mount(&flash, _T("1:"), 1);
      if (result != FR_OK) {
        printf("remount flash fail %02x\r\n", result);
        goto __jump2;
      }
    } else {
      goto __jump2;
    }
  }
  
  printf("Mount flash disk successfully\r\n");
	fat_flashtest();
	//fat_update_uni2gbk(_T("0:/unigbk.bin"));
	
__jump2:
	while(1)
	{
    LED0(OFF);
		LED1(ON);
		Delay_us(200000);
    LED0(ON);
		LED1(OFF);
		Delay_us(200000);
	}
}
