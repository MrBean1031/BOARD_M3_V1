#ifndef __RC522_CONFIG_H
#define	__RC522_CONFIG_H

#include "stm32f10x.h"

#define RC522_SPI1             0x01
#define RC522_SPI2             0x02
#define RC522_SPI3             0x04

/* 定义使用的SPI接口 */
#define RC522_USE_SPIx         RC522_SPI2
/* 定义是否使用硬件复位 */
#define RC522_HW_RESET             0
/* 定义SPI是否复用 */
#define RC522_SPI_MULTIPLEX        1

#if RC522_USE_SPIx == RC522_SPI1
#define RC522_SPITypeDef                 SPI1
#define RC522_GPIO_PORT                  GPIOA
#define RC522_GPIO_PIN_CS                GPIO_Pin_8
#define RC522_GPIO_PIN_SCK               GPIO_Pin_5
#define RC522_GPIO_PIN_MISO              GPIO_Pin_6
#define RC522_GPIO_PIN_MOSI              GPIO_Pin_7
#define RC522_GPIO_PIN_RST               GPIO_Pin_3
#define RC522_GPIO_ClockCmd(newstate)    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, newstate)
#define RC522_SPI_ClockCmd(newstate)     RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, newstate)

#elif RC522_USE_SPIx == RC522_SPI2
#define RC522_SPITypeDef                 SPI2
#define RC522_GPIO_PORT                  GPIOB
#define RC522_GPIO_PIN_CS                GPIO_Pin_11
#define RC522_GPIO_PIN_SCK               GPIO_Pin_13
#define RC522_GPIO_PIN_MISO              GPIO_Pin_14
#define RC522_GPIO_PIN_MOSI              GPIO_Pin_15
#define RC522_GPIO_PIN_RST               GPIO_Pin_9
#define RC522_GPIO_ClockCmd(newstate)    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, newstate)
#define RC522_SPI_ClockCmd(newstate)     RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, newstate)

#elif RC522_USE_SPIx == RC522_SPI3
#define RC522_SPITypeDef                 SPI3
#define RC522_GPIO_PORT                  GPIOB
#define RC522_GPIO_PIN_CS                GPIO_Pin_1
#define RC522_GPIO_PIN_SCK               GPIO_Pin_3
#define RC522_GPIO_PIN_MISO              GPIO_Pin_4
#define RC522_GPIO_PIN_MOSI              GPIO_Pin_5
#define RC522_GPIO_PIN_RST               GPIO_Pin_0
#define RC522_GPIO_ClockCmd(newstate)    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, newstate)
#define RC522_SPI_ClockCmd(newstate)     RCC_APB1PerhphClockCmd(RCC_APB1Periph_SPI3, newstate)

#else
#define RC522_SPITypeDef                 
#define RC522_GPIO_PORT                  
#define RC522_GPIO_PIN_CS                
#define RC522_GPIO_PIN_SCK               
#define RC522_GPIO_PIN_MISO              
#define RC522_GPIO_PIN_MOSI              
#define RC522_GPIO_PIN_RST               
#define RC522_GPIO_ClockCmd(newstate)    
#define RC522_SPI_ClockCmd(newstate)     
#endif /* RC522_USE_SPIx*/


#if RC522_SPI_MULTIPLEX > 0
#define macRC522_CS_Enable()             \
	do {\
		saveprescale = RC522_SPI_GetSpeed(RC522_SPITypeDef);\
    RC522_SPI_SetSpeed(RC522_SPITypeDef, rc522prescale);\
		RC522_GPIO_PORT->ODR &= ~RC522_GPIO_PIN_CS;\
	}while(0)
#define macRC522_CS_Disable()            \
	do{\
		RC522_GPIO_PORT->ODR |=  RC522_GPIO_PIN_CS;\
    RC522_SPI_SetSpeed(RC522_SPITypeDef, saveprescale);\
	}while(0)  

#else  //RC522_SPI_MULTIPLEX
#define macRC522_CS_Enable()             (RC522_GPIO_PORT->ODR &= ~RC522_GPIO_PIN_CS)
#define macRC522_CS_Disable()            (RC522_GPIO_PORT->ODR |=  RC522_GPIO_PIN_CS)
#endif //RC522_SPI_MULTIPLEX

#define macRC522_Reset_Enable()          (RC522_GPIO_PORT->ODR &= ~RC522_GPIO_PIN_RST)
#define macRC522_Reset_Disable()         (RC522_GPIO_PORT->ODR |=  RC522_GPIO_PIN_RST)


#if RC522_SPI_MULTIPLEX
extern uint16_t saveprescale;
extern uint16_t rc522prescale;
#endif
u8       RC522_SPI_SetSpeed   (SPI_TypeDef *SPIx, uint16_t BaudPrescale);
uint16_t RC522_SPI_GetSpeed   (SPI_TypeDef *SPIx);
void     RC522_SPI_Config     (uint16_t prescale);
uint8_t  RC522_SPI_Transmit   (uint8_t TxData);


#endif /* __RC522_CONFIG_H */
