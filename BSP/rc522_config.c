#include "rc522_config.h"

#if RC522_SPI_MULTIPLEX
uint16_t saveprescale;
uint16_t rc522prescale;
#endif

/*-----------------------------------------------------
 - Function Name: RC522_SPI_SetSpeed
 - Description:   Set SPIx Clock Prescale
 - Input:         SPIx, BaudPrescale
 - Output:        None
 - Return:        0: succeed, 1: timeout
 - Attention:     SPI1 Clock Sourse: fPCLK2 72MHz
                  /2   36M      /4   18M
                  /8   9M       /16  4.5M
                  /32  2.25M    /64  1.125M
                  /128 562.5K   /256 281.25k
									@BaudPrescale
                  This parameter can be a value of @ref SPI_BaudRatePrescaler_.
-----------------------------------------------------*/
/* SPI SPE mask */
#ifndef CR1_SPE_Set
#define CR1_SPE_Set          ((uint16_t)0x0040)
#endif
#ifndef CR1_SPE_Reset
#define CR1_SPE_Reset        ((uint16_t)0xFFBF)
#endif
u8 RC522_SPI_SetSpeed(SPI_TypeDef* SPIx, uint16_t BaudPrescale)
{
	u8 timeout;
	/* check the parameters */
	assert_param(IS_SPI_ALL_PERIPH(SPIx));
	assert_param(IS_SPI_BAUDRATE_PRESCALER(BaudPrescale));
	
	timeout=200;
	while((SPIx->SR &0x0080) && --timeout);  //check busy
	if(timeout==0)
		return 1;
	SPIx->CR1 &= CR1_SPE_Reset;  //disable SPIx
	
	SPIx->CR1 &= ~(u16)0X0038;  //clear SPI_CR1 BR[5:3] bit field
	SPIx->CR1 |= BaudPrescale;
	
	SPIx->CR1 |= CR1_SPE_Set;  //enable SPIx
	return 0;
}

/*-----------------------------------------------------
 - Function Name: RC522_SPI_GetSpeed
 - Description:   Get SPIx Clock Prescale
 - Input:         SPIx, BaudPrescale
 - Output:        None
 - Return:        0xFFFF SPIx dose not work, else the presclae of SPIx
 - Attention:     SPI1 Clock Sourse: fPCLK2 72MHz
                  /2   36M      /4   18M
                  /8   9M       /16  4.5M
                  /32  2.25M    /64  1.125M
                  /128 562.5K   /256 281.25k
									@BaudPrescale
                  This parameter can be a value of @ref SPI_BaudRatePrescaler_.
-----------------------------------------------------*/
uint16_t RC522_SPI_GetSpeed(SPI_TypeDef *SPIx)
{
	uint16_t prescale;
	
	/* check the parameters */
	assert_param(IS_SPI_ALL_PERIPH(SPIx));
	/* check if SPIx does not work */
	if(!(SPIx->CR1 & CR1_SPE_Set))
		return 0xFFFF;
	prescale = SPIx->CR1 & 0x0038;  //SPI_CR1 BR[5:3] bit field
	return prescale;
}

void RC522_SPI_Config(uint16_t prescale)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	
	RC522_GPIO_ClockCmd(ENABLE);
	RC522_SPI_ClockCmd(ENABLE);
	/* configure CS as general-purpuse push-pull output */
	GPIO_InitStruct.GPIO_Pin = RC522_GPIO_PIN_CS;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_GPIO_PORT, &GPIO_InitStruct);
	/* configure SCK and MOSI as alternative function push-pull output */
	GPIO_InitStruct.GPIO_Pin = RC522_GPIO_PIN_SCK | RC522_GPIO_PIN_MOSI;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_GPIO_PORT, &GPIO_InitStruct);
	/* configure MISO as floating input */
	GPIO_InitStruct.GPIO_Pin = RC522_GPIO_PIN_MISO;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_GPIO_PORT, &GPIO_InitStruct);
	RC522_GPIO_PORT->ODR |= RC522_GPIO_PIN_CS;  //拉高CS
#if RC522_HW_RESET > 0
	/* configure RST as general-purpuse function push-pull output */
	GPIO_InitStruct.GPIO_Pin = RC522_GPIO_PIN_RST;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RC522_GPIO_PORT, &GPIO_InitStruct);
	macRC522_Reset_Disable();  //拉高RST
#endif //RC522_HW_RESET

#if RC522_SPI_MULTIPLEX > 0
	saveprescale = RC522_SPI_GetSpeed(RC522_SPITypeDef);
	if(saveprescale == 0xFFFF)  //未启动的SPI
		saveprescale = prescale;
	rc522prescale = prescale;  //RC522设置的SPI波特率
	SPI_InitStruct.SPI_BaudRatePrescaler = saveprescale;
#else
	SPI_InitStruct.SPI_BaudRatePrescaler = prescale;
#endif	
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;  //Idle State: Low
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;  //Odd edge sample
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //全双工模式
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(RC522_SPITypeDef, &SPI_InitStruct);
	
	SPI_Cmd(RC522_SPITypeDef, ENABLE);  //使能SPI端口
}

uint8_t RC522_SPI_Transmit(uint8_t TxData)
{
	while(SPI_I2S_GetFlagStatus(RC522_SPITypeDef, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(RC522_SPITypeDef, TxData);
	
	while(SPI_I2S_GetFlagStatus(RC522_SPITypeDef, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(RC522_SPITypeDef);
}

