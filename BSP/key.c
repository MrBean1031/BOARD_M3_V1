//key0 -> PA1, key1 -> PA2, WK_UP->PA0

#include "key.h"
//#include "SysTick.h"
#include "delay_tim.h"

void Key_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//ø™∆ÙÕ‚…Ë ±÷”
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

uint8_t KeyScan(void)
{
	static char flag0=1,flag1=1,flag_wk=1;
	u8 key = KEY_NONE;
	/*key0ºÏ≤‚*/
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1) == 0)
	{
		Delay_us(10000);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==0)
		{
			if(flag0==1){
				flag0=0;
				key = KEY0;
			}
		}
	} else {
    flag0=1;
  }
	/*key1ºÏ≤‚*/
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2) == 0)
	{
		Delay_us(10000);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)==0)
		{
			if(flag1==1){
				flag1=0;
				key = KEY1;
			}
		}
	} else {
    flag1=1;
  }
	/*wakeup_keyºÏ≤‚*/
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
	{
		Delay_us(10000);
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
		{
			if(flag_wk==1){
				flag_wk=0;
				key = KEY_WK;
			}
		}
	} else {
    flag_wk=1;
  }
	return key;
}
