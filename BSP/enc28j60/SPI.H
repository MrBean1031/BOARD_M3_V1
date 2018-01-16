#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

#define  ENC28J60_CSL()    (GPIOA->ODR &= ~GPIO_Pin_3)
#define  ENC28J60_CSH()    (GPIOA->ODR |=  GPIO_Pin_3)

#define  ENC28J60_RSTL()   //(GPIOE->ODR &= ~(1<<1))
#define  ENC28J60_RSTH()   //(GPIOE->ODR |= 1<<1)

#define  ENC_SPI_ReadWrite(x)     SPI1_ReadWrite(x)
#define  ENC_SPI_Init(x)          SPI1_Init(x)

void SPI1_Init(void);
uint8_t SPI1_ReadWrite(uint8_t writedat);
   

#endif
