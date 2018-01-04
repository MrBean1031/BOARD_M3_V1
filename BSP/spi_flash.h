#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f10x.h"

#define RCC_SPI_FlashClkCmd(sta) \
  do { \
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, sta); \
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, sta); \
  } while(0)
#define FLASH_GPIOPort       GPIOA
#define FLASH_GPIOPin_CS     GPIO_Pin_4
#define FLASH_GPIOPin_SCK    GPIO_Pin_5
#define FLASH_GPIOPin_MISO   GPIO_Pin_6
#define FLASH_GPIOPin_MOSI   GPIO_Pin_7
#define FLASH_SPIx           SPI1
#define FLASH_NSS_HIGH       (FLASH_GPIOPort->ODR |=  FLASH_GPIOPin_CS)
#define FLASH_NSS_LOW        (FLASH_GPIOPort->ODR &= ~FLASH_GPIOPin_CS)

#define FLASH_DUMMY             0X00
#define FLASH_PAGE_SIZE         256
#define FLASH_STATUS_BUSY_MASK  0X01
#define FLASH_BYTE_ADDR_MASK    0X0000FFu
#define FLASH_PAGE_ADDR_MASK    0X000F00u
#define FLASH_SECTOR_ADDR_MASK  0X00F000u
#define FLASH_BLOCK_ADDR_MASK   0XFF0000u

/*-------------------------------------------------
  Command Word Declaration
-------------------------------------------------*/
#define WRITE_ENABLE        0X06
#define WRITE_DISABLE       0X04
#define READ_STATUS_REG     0X05
#define WRITE_STATUS_REG    0X01
#define READ_DATA           0X03
#define FAST_READ           0X0B
#define FAST_READ_DUAL_OUT  0X3B
#define PAGE_PROGRAM        0X02
#define BLOCK_ERASE         0XD8
#define SECTOR_ERASE        0X20
#define CHIP_ERASE          0XC7
#define POWER_DOWN          0XB9
#define RPD_DEVICE_ID       0XAB  //release power down
#define MANUFACTURER_ID     0X90
#define JEDEC_ID            0X9F

/*define a structure class Flash_Address to resolve the */
typedef struct
{
  uint32_t ByteAddr;
  uint32_t PageAddr;
  uint32_t SectorAddr;
  uint32_t BlockAddr;
  
  uint16_t ByteNum;
  uint16_t PageNum;
  uint16_t SectorNum;
  uint16_t BlockNum;
}Flash_Address;

/*Function Declaration*/
void SPI_Flash_Config(void);
uint8_t SPI_Flash_Transmit(uint8_t TxData);
uint8_t SPI_GetDeviceID(void);
uint32_t SPI_GetFlashID(void);
void SPI_Flash_WaitForWriteEnd(void);
void SPI_Flash_WriteEnable(void);
void SPI_Flash_EraseSector(uint32_t SectorAddr);
void SPI_Flash_PageWrite(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite);
void SPI_Flash_BufferWrite(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite);
void SPI_Flash_BufferRead(uint8_t* buffer, uint32_t ReadAddr, uint32_t BytesToRead);
void SPI_Flash_AddressResolve(uint32_t FlashAddr, Flash_Address* ResolveAddr);
u8 SPI_Flash_WriteWithErase(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite);

#endif /*__SPI_FLASH_H*/
