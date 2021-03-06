/*
 * SPI1_NSS->PA4 ,SPI1_SCK->PA5, SPI1_MISO->PA6, SPI1_MOSI->PA7
 */
#include "spi_flash.h"

u8 flashbuf[4096]={0};  //SPI_Flash_WriteWithErase()��д������
u32 cur_sec_addr;
u16 bitmap_page;

void SPI_Flash_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef SPI_InitStructure;
  
  RCC_SPI_FlashClkCmd(ENABLE);
  /*Configure SPI1_NSS as general purpose push-pull output*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = FLASH_GPIOPin_CS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(FLASH_GPIOPort, &GPIO_InitStructure);
  
  /*Configure SPI1_SCK and SPI1_MOSI as alternative function push-pull output*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = FLASH_GPIOPin_SCK | FLASH_GPIOPin_MOSI;
  GPIO_Init(FLASH_GPIOPort, &GPIO_InitStructure);
  
  /*Configure SPI1_MISO as floating input*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = FLASH_GPIOPin_MISO;
  GPIO_Init(FLASH_GPIOPort, &GPIO_InitStructure);
  
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;  //Idle State: HIGH
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;  //Even edge sample
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ȫ˫��ģʽ
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(FLASH_SPIx, &SPI_InitStructure);
  
  SPI_Cmd(FLASH_SPIx, ENABLE);
  FLASH_NSS_HIGH;
  cur_sec_addr = 0;
  bitmap_page = 0xffff;  //������д��λͼ
}

uint8_t SPI_Flash_Transmit(uint8_t TxData)
{
  while(SPI_I2S_GetFlagStatus(FLASH_SPIx, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(FLASH_SPIx, TxData);
  
  while(SPI_I2S_GetFlagStatus(FLASH_SPIx, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(FLASH_SPIx);
}

uint8_t SPI_GetDeviceID(void)
{
  uint32_t temp;
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(RPD_DEVICE_ID);
  SPI_Flash_Transmit(FLASH_DUMMY);
  SPI_Flash_Transmit(FLASH_DUMMY);
  SPI_Flash_Transmit(FLASH_DUMMY);
  temp = SPI_Flash_Transmit(FLASH_DUMMY);
  FLASH_NSS_HIGH;
  return temp;
}

uint32_t SPI_GetFlashID(void)
{
  uint32_t temp=0,temp0,temp1,temp2;
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(JEDEC_ID);
  temp0 = SPI_Flash_Transmit(FLASH_DUMMY);  //M7  -M0
  temp1 = SPI_Flash_Transmit(FLASH_DUMMY);  //ID15-ID8
  temp2 = SPI_Flash_Transmit(FLASH_DUMMY);  //ID7 -ID0
  FLASH_NSS_HIGH;
  temp = (temp0<<16) + (temp1<<8) + temp2;
  return temp;
}

void SPI_Flash_WaitForWriteEnd(void)
{
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(READ_STATUS_REG);
  while(SPI_Flash_Transmit(FLASH_DUMMY) & FLASH_STATUS_BUSY_MASK);
  FLASH_NSS_HIGH;
}

void SPI_Flash_WriteEnable(void)
{
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(WRITE_ENABLE);
  FLASH_NSS_HIGH;
}

void SPI_Flash_EraseSector(uint32_t SectorAddr)
{
  SPI_Flash_WaitForWriteEnd();
  SPI_Flash_WriteEnable();
  
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(SECTOR_ERASE);
  SPI_Flash_Transmit((SectorAddr&0XFF0000u)>>16);
  SPI_Flash_Transmit((SectorAddr&0X00FF00u)>>8);
  SPI_Flash_Transmit(SectorAddr&0X0000FFu);
  FLASH_NSS_HIGH;
}

void SPI_Flash_PageWrite(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite)
{
  SPI_Flash_WaitForWriteEnd();
  SPI_Flash_WriteEnable();
  
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(PAGE_PROGRAM);
  SPI_Flash_Transmit((WriteAddr&0XFF0000u)>>16);
  SPI_Flash_Transmit((WriteAddr&0X00FF00u)>>8);
  SPI_Flash_Transmit(WriteAddr&0X0000FFu);
  while(BytesToWrite--)
  {
    SPI_Flash_Transmit(*buffer);
    buffer++;
  }
  FLASH_NSS_HIGH;
}

void SPI_Flash_BufferWrite(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite)
{
  uint32_t Addr=0, count=0, NumOfPage=0, NumOfSingle;
  Addr = WriteAddr % FLASH_PAGE_SIZE;
  count = FLASH_PAGE_SIZE - Addr;
  NumOfPage = BytesToWrite / FLASH_PAGE_SIZE;
  NumOfSingle = BytesToWrite % FLASH_PAGE_SIZE;
  /*WriteAddr is aligned to flash_page*/
  if(Addr == 0)
  {
    if(NumOfPage == 0)
      SPI_Flash_PageWrite(buffer, WriteAddr, NumOfSingle);
    else
    {
      while(NumOfPage--)
      {
        SPI_Flash_PageWrite(buffer, WriteAddr, FLASH_PAGE_SIZE);
        buffer += FLASH_PAGE_SIZE;
        WriteAddr += FLASH_PAGE_SIZE;
      }
      if(NumOfSingle != 0)
        SPI_Flash_PageWrite(buffer, WriteAddr, NumOfSingle);
    }
  }  
  /*WriteAddr isn't aligned to flash_page*/
  else
  {
    SPI_Flash_PageWrite(buffer, WriteAddr, count);
    buffer += count;
    WriteAddr += count;
    while(NumOfPage--)
    {
      SPI_Flash_PageWrite(buffer, WriteAddr, FLASH_PAGE_SIZE);
      buffer += FLASH_PAGE_SIZE;
      WriteAddr += FLASH_PAGE_SIZE;
    }
    if(NumOfSingle-count > 0)
      SPI_Flash_PageWrite(buffer, WriteAddr, NumOfSingle-count);
  }
}

void SPI_Flash_BufferRead(uint8_t* buffer, uint32_t ReadAddr, uint32_t BytesToRead)
{
  SPI_Flash_WaitForWriteEnd();
  
  FLASH_NSS_LOW;
  SPI_Flash_Transmit(READ_DATA);
  SPI_Flash_Transmit((ReadAddr&0XFF0000u)>>16);
  SPI_Flash_Transmit((ReadAddr&0X00FF00u)>>8);
  SPI_Flash_Transmit(ReadAddr&0X0000FFu);
  while(BytesToRead--)
  {
    *buffer = SPI_Flash_Transmit(FLASH_DUMMY);
    buffer++;
  }
  FLASH_NSS_HIGH;
}

void SPI_Flash_AddressResolve(uint32_t FlashAddr, Flash_Address* ResolveAddr)
{
  FlashAddr &= 0XFFFFFFu;
  
  ResolveAddr->ByteAddr = FlashAddr & FLASH_BYTE_ADDR_MASK;
  ResolveAddr->PageAddr = FlashAddr & FLASH_PAGE_ADDR_MASK;
  ResolveAddr->SectorAddr = FlashAddr & FLASH_SECTOR_ADDR_MASK;
  ResolveAddr->BlockAddr = FlashAddr & FLASH_BLOCK_ADDR_MASK;
  
  ResolveAddr->ByteNum = ResolveAddr->ByteAddr;
  ResolveAddr->PageNum = (ResolveAddr->PageAddr)>>8;
  ResolveAddr->SectorNum = (ResolveAddr->SectorAddr)>>12;
  ResolveAddr->BlockNum = (ResolveAddr->BlockAddr)>>16;
  
  ResolveAddr->SectorAddr |= ResolveAddr->BlockAddr;
  ResolveAddr->PageAddr |= ResolveAddr->SectorAddr;
}

u8 SPI_Flash_WriteWithErase(uint8_t* buffer, uint32_t WriteAddr, uint32_t BytesToWrite)
{
  u32 secoffset=0, secremain=0, len;
  Flash_Address flashaddr;
  u32 i;
  u16 bitmap;

  while(BytesToWrite)
  {
    secoffset = WriteAddr % 4096;
    if(secoffset)
    {
      SPI_Flash_AddressResolve(WriteAddr, &flashaddr);
      secremain = 4096 - secoffset;
      len = BytesToWrite > secremain ? secremain : BytesToWrite;
      bitmap = ~(0xffff << (len / FLASH_PAGE_SIZE + !!(len % FLASH_PAGE_SIZE)));
      if(flashaddr.SectorAddr == cur_sec_addr)
      {
        if((bitmap_page & (bitmap << flashaddr.PageNum)) == 0)
        {
          bitmap_page |= bitmap << flashaddr.PageNum;
          SPI_Flash_BufferWrite(buffer, WriteAddr, len);
          buffer += len;
          WriteAddr += len;
          BytesToWrite -= len;
          continue;
        }
      }
      else
      {
        cur_sec_addr = flashaddr.SectorAddr;
        bitmap_page = 0;
      }
      bitmap_page |= bitmap << flashaddr.PageNum;
      SPI_Flash_BufferRead(flashbuf, flashaddr.SectorAddr, 4096);  //��ȡ
      SPI_Flash_EraseSector(flashaddr.SectorAddr);  //����
      for (i=0; i<len; i++)
      {
        flashbuf[secoffset + i] = *(buffer + i);  //�޸�
      }
      SPI_Flash_BufferWrite(flashbuf, flashaddr.SectorAddr, 4096);  //д��
      buffer += len;
      WriteAddr += len;
      BytesToWrite -= len;
    }
    else
    {
      len = BytesToWrite > 4096 ? 4096 : BytesToWrite;
      bitmap = ~(0xffff << (len / FLASH_PAGE_SIZE + !!(len % FLASH_PAGE_SIZE)));
      cur_sec_addr = WriteAddr;
      bitmap_page = bitmap;
      if (len < 4096)
      {
        SPI_Flash_BufferRead(flashbuf, WriteAddr, 4096);  //��ȡ
        SPI_Flash_EraseSector(WriteAddr);  //����
        for (i=0; i<len; i++)
        {
          flashbuf[i] = *(buffer + i);  //�޸�
        }
        SPI_Flash_BufferWrite(flashbuf, WriteAddr, 4096);  //д��
      }
      else
      {
        SPI_Flash_EraseSector(WriteAddr);
        SPI_Flash_BufferWrite(buffer, WriteAddr, 4096);
      }
      buffer += len;
      WriteAddr += len;
      BytesToWrite -= len;
    }
  }
  return 0;
}
