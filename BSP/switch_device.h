#ifndef __SWITCH_DEVICE_H_
#define __SWITCH_DEVICE_H_

#include "stm32f10x.h"

enum sw_state {SW_OFF, SW_ON, SW_TOGGLE};
enum sw_level {SW_RESET, SW_SET};

#define SW_ERR_NONE   0
#define SW_ERR_NOMEM  -1
#define SW_ERR_PARAM  -2
#define SW_ERR_EXIST  -3
#define SW_ERR_NODEV  -4
#define SW_ERR_PROT   -5

#define SWITCH_MUTEX_PRIO  3

int sw_dev_open(const char *name, GPIO_TypeDef *port, u16 pin, enum sw_level on_level, enum sw_state init_state);
int sw_dev_close(const char *name);
int sw_dev_change(const char *name, enum sw_state new_state);

#endif
