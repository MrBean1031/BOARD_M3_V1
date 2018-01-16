#ifndef __LWIP_INIT_H
#define __LWIP_INIT_H

#include "stm32f10x.h"

extern struct netif enc28j60;

void LwIP_Init( void );

#endif
