#include "stm32f10x.h"
#include "usart.h"
#include "spi_flash.h"
#include "stdio.h"
#include "string.h"

#define Address 0

int main(void)
{
	uint32_t DeviceID, FlashID;
	Flash_Address NowFlashAddr;
	const uint8_t Flash_TxData[256] =
	{"Chinese Kung fu is a commonly used term for summarizing all "
	 "the martial arts styles in China. In fact, wushu in Chinese "
	 "is the exact traditional term used to describe Chinese martial "
	 "arts. Kung fu describes external and internal styles of "
   "martial arts."};
	uint8_t Flash_RxData[256]={0};
	
	SPI_Flash_Config();
	USART_Config();
	
	DeviceID = SPI_GetDeviceID();
	FlashID = SPI_GetFlashID();
	printf("\r\n"__DATE__"  "__TIME__"\r\n");
	printf("W25Q64 FLASH information:\r\nDevice ID %X, JEDEC ID %X\r\n",DeviceID, FlashID);
	
	SPI_Flash_AddressResolve(Address, &NowFlashAddr);
	printf("BlockNum: %d, SectorNum: %d, PageNum: %d\r\n\r\n", NowFlashAddr.BlockNum, NowFlashAddr.SectorNum, NowFlashAddr.PageNum);
	
	SPI_Flash_EraseSector(NowFlashAddr.SectorAddr);
	//SPI_Flash_BufferWrite((uint8_t *)Flash_TxData, Address, 256);
  SPI_Flash_WriteWithErase((uint8_t *)Flash_TxData, Address, 256);
	printf("The Data written in W25Q64:\r\n%s\r\n\r\n", Flash_TxData);
	
	SPI_Flash_BufferRead(Flash_RxData, Address, 256);
	printf("The Data read from W25Q64:\r\n%s\r\n\r\n", Flash_RxData);
	
	if(strcmp((const char *)Flash_TxData, (const char *)Flash_RxData) == 0)  //if Flash_TxData is equal to Flash_RxData
		printf("Succeed to transmit data.\r\n");
	else
		printf("Fail to transmit data.\r\n");
//	SPI_Flash_WriteEnable();
//	SPI_Flash_WaitForWriteEnd();
//	FLASH_NSS_LOW;
//	SPI_Flash_Transmit(CHIP_ERASE);
//	FLASH_NSS_HIGH;
//	SPI_Flash_WaitForWriteEnd();
//	printf("Chip Erase succeed.\r\n");
	while(1);
}
