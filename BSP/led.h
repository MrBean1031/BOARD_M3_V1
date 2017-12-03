#ifndef _LED_H_
#define _LED_H_

#include "stm32f10x.h"

#define ON 1
#define OFF 0

#define LED0_PERIPH_CLK  RCC_APB2Periph_GPIOB
#define LED0_GPIO_PORT   GPIOB
#define LED0_GPIO_PIN    GPIO_Pin_8

#define LED1_PERIPH_CLK  RCC_APB2Periph_GPIOB
#define LED1_GPIO_PORT   GPIOB
#define LED1_GPIO_PIN    GPIO_Pin_9

void LED_Config(void);
void LED0(uint8_t flag);
void LED1(uint8_t flag);

#endif /*_LED_H_*/
