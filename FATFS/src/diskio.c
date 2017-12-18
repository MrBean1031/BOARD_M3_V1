/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "stm32_eval_sdio_sd.h"
#include "spi_flash.h"
#include "stdio.h"
/* Definitions of physical drive number for each drive */
#define SD_CARD       0
#define SPI_FLASH     1

//Definitions of SPI_FLASH
#define FLASH_SECTOR_SIZE 512  //Flash在FATFS里的扇区大小
#define FLASH_ERASE_SIZE  4096  //Flash扇区大小,FATFS里的簇大小
#define FLASH_CAPACITY    4194304UL  //Flash磁盘大小（字节），剩余空间留给快速访问需求如CodePage转换表

SD_Error sd_err = SD_ERROR;
SD_CardInfo sd_info;


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS resp = STA_NOINIT;
	switch(pdrv)
	{
		case SD_CARD:
      if(sd_err == SD_OK) {
        resp = 0;
      } else {
        resp = STA_NOINIT;
      }
      break;
		case SPI_FLASH:
			resp = 0;
			break;
	}
	return resp;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS resp = STA_NOINIT;
  u32 flash_id = 0;
	switch(pdrv)
	{
		case SD_CARD:
			sd_err = SD_Init();
      if(sd_err != SD_OK) {
        resp = STA_NOINIT;
        break;
      }
      resp = 0;
			break;
		case SPI_FLASH:
			SPI_Flash_Config();  //端口初始化
      SPI_GetDeviceID();
			flash_id = SPI_GetFlashID();  //Flash上电并获取FlashID
      printf("spi flash id %06X\r\n", flash_id);
			resp = 0;
			break;
	}
	return resp;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	SD_Error rerr;
  SDTransferState transtate;

	if(!buff)
		return RES_PARERR;  //参数错误
	switch(pdrv) {
  	case SD_CARD:      
      if(sd_err != SD_OK) {
        return RES_NOTRDY;
      } 
      
      while((transtate = SD_GetStatus()) == SD_TRANSFER_BUSY) {}
      if(transtate == SD_TRANSFER_ERROR) {
        return RES_ERROR;
      }
      if(count==1) {
        rerr = SD_ReadBlock(buff, sector, 512);
        if(rerr != SD_OK) {
          return RES_ERROR;
        }
        rerr = SD_WaitReadOperation();
        if(rerr != SD_OK) {
          return RES_ERROR;
        }
      } else {
        rerr = SD_ReadMultiBlocks(buff, sector, 512, count);
        if(rerr != SD_OK) {
          return RES_ERROR;
        }
        rerr = SD_WaitReadOperation();
        if(rerr != SD_OK) {
          return RES_ERROR;
        }
      }
      return RES_OK;
  		
  	case SPI_FLASH:
  		SPI_Flash_BufferRead(buff, sector*FLASH_SECTOR_SIZE, count*FLASH_SECTOR_SIZE);
  		return RES_OK;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	SD_Error werr;
  SDTransferState transtate;

	if(!buff)
		return RES_PARERR;  //参数错误
	switch(pdrv) {
		case SD_CARD:
      if(sd_err != SD_OK) {
        return RES_NOTRDY;
      }
    
			while((transtate = SD_GetStatus()) == SD_TRANSFER_BUSY) {}
      if(transtate == SD_TRANSFER_ERROR) {
        return RES_ERROR;
      }
      if(count == 1) {
        werr = SD_WriteBlock((uint8_t *)buff, sector, 512);
        if(werr != SD_OK) {
          return RES_ERROR;
        }
        werr = SD_WaitWriteOperation();
        if(werr != SD_OK) {
          return RES_ERROR;
        }
      } else {
        werr = SD_WriteMultiBlocks((uint8_t *)buff, sector, 512, count);
        if(werr != SD_OK) {
          return RES_ERROR;
        }
        werr = SD_WaitWriteOperation();
        if(werr != SD_OK) {
          return RES_ERROR;
        }
      }
      return RES_OK;
		
		case SPI_FLASH:
      printf("spi flash writed data size %d bytes\r\n", count*FLASH_SECTOR_SIZE);
			SPI_Flash_WriteWithErase((uint8_t *)buff, sector*FLASH_SECTOR_SIZE, count*FLASH_SECTOR_SIZE);
			return RES_OK;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	
	switch (pdrv) {
		case SD_CARD:
			switch(cmd) {
				case CTRL_SYNC:
					res = RES_OK;  //读写函数末尾带忙检测
					break;
				case GET_SECTOR_SIZE:
					*(WORD *)buff = 512;
					res = RES_OK;
					break;
				case GET_BLOCK_SIZE:
					*(WORD *)buff = 8;  //4096 Bytes per cluster
					res = RES_OK;
					break;
				case GET_SECTOR_COUNT:
          if(sd_info.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 || 
             sd_info.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) {
            sd_info.CardCapacity /= 1024;
            if(sd_info.CardCapacity == 0) {
              sd_info.CardCapacity = 1;
            }
          }
					*(DWORD *)buff = sd_info.CardCapacity * 2;  //获取扇区数
					res = RES_OK;
          break;
			}
			return res;

		case SPI_FLASH:
			switch(cmd) {
				case CTRL_SYNC:
					SPI_Flash_WaitForWriteEnd();  //等待写入结束
					res = RES_OK;
					break;
				case GET_SECTOR_SIZE:
					*(WORD *)buff = FLASH_SECTOR_SIZE;
					res = RES_OK;
					break;
				case GET_BLOCK_SIZE:
					*(WORD *)buff = (FLASH_ERASE_SIZE / FLASH_SECTOR_SIZE);
					res = RES_OK;
					break;
				case GET_SECTOR_COUNT:
					*(DWORD *)buff = (FLASH_CAPACITY / FLASH_SECTOR_SIZE);
					res = RES_OK;
          break;
			}
			return res;
	}

	return RES_PARERR;
}

DWORD get_fattime(void)
{
	DWORD date_time=0;
	date_time |= (2016UL-1980) << 25;  //年,[31:25],0~127 相对与1980年
	date_time |= 6UL << 21;            //月,[24:21],1~12
	date_time |= 28UL << 16;           //日,[20:16],1:31
	date_time |= 17UL << 11;           //时,[15:11],0~23
	date_time |= 45UL << 5;            //分,[10：5],0~59
	date_time |= 15UL << 0;            //秒,[ 4: 0],0~29 单位为2秒
	return date_time;
}
