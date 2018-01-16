/*
 *File Description: ��ʼ��enc28j60��SPI���ṩ�ײ��շ�����
 *Pin Mapping:      ENC28J60-CS   -> PA3
                    ENC28J60-SCK  -> PA5-SPI1_SCK
										ENC28J60-MISO -> PA6-SPI1_MISO
										ENC28J60-MOSI -> PA7-SPI1_MOSI
										ENC28J60-RST  -> Optional
										ENC28J60-INT  -> None
 *Author:           Mr.Bean
 *Date:             2017/2/22
 *Attention:        ENC28J60ֻ����ʹ��SPI��ģʽ0,0��SCK����ʱ��Ҫ���ֵ͵�ƽ�����Ҳ�
                    ֧��ʱ�Ӽ���ѡ����ÿ��SCK�������ص���ʱ����������ݴ�SI������
										�룬ÿ��SCK���½��ص���ʱ�����ݴ�SO�����Ƴ���
 
 �̳е���ֲ��Ϣ��
 * �ļ���  ��SPI.c
 * ����    ����ʼ��enc28j60ʹ�õ���SPI1�����ײ��շ����ݺ���       
 * ʵ��ƽ̨��Ұ��STM32������
 * Ӳ�����ӣ� ______________________________________
 *           |PB13         ��ENC28J60-INT (û�õ�)  |
 *           |PA6-SPI1-MISO��ENC28J60-SO            |
 *           |PA7-SPI1-MOSI��ENC28J60-SI            |
 *           |PA5-SPI1-SCK ��ENC28J60-SCK           |
 *           |PA4-SPI1-NSS ��ENC28J60-CS            |
 *           |PE1          ��ENC28J60-RST (û�õ�)  |
 *           |______________________________________|
 * ��汾  ��ST3.5.0
 * ����    ��wildfire team 
 * ��̳    ��www.ourdev.cn/bbs/bbs_list.jsp?bbs_id=1008
 * �Ա�    ��http://firestm32.taobao.com
 
 */

#include "SPI.h"

/*
 * ��������SPI1_Init
 * ����  ����ʼ��SPI1�˿ڼ���ģʽ
 * ����  ����
 * ���  : ��
 * ����  ���ⲿ����
 */
void SPI1_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		//CS
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_SetBits(GPIOA, GPIO_Pin_3);  //����CS
	
	/* Configure SPI1 pin: MISO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure SPI1 pins: SCK,and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	/* SPI1 configuration */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);
}

/*
 * ��������SPI1_ReadWrite
 * ����  ����ײ��ʹ��SPI1�շ�һ��������
 * ����  ��Ҫд�������
 * ���  : ���յ�������
 * ����  ���ⲿ����
 */
uint8_t SPI1_ReadWrite(uint8_t dat)
{
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, dat);

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}
