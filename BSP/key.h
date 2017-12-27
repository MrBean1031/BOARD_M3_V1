#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f10x.h"

#define KEY_WK    0x01
#define KEY0      0x02
#define KEY1      0x03
#define KEY_NONE  0xFF

void Key_Config(void);
uint8_t KeyScan(void);

#endif /*_KEY_H_*/
