#include "switch_device.h"
#include "stdlib.h"
#include "global.h"
#include "string.h"
#include "includes.h"

#define MALLOC(size)        malloc_safe(size)
#define FREE(ptr)           free_safe(ptr)

#ifdef OS_uCOS_II_H
typedef OS_EVENT *prot_t;
#else
typedef u8 prot_t;
#endif
static prot_t pt;

struct switch_dev {
  const char *name;
  struct switch_dev *next;
  GPIO_TypeDef *gpio_port;
  u16 gpio_pin;
  enum sw_state cur_state;
  enum sw_level on_level, off_level;
};

static struct switch_dev *sw_list_head = 0;

static prot_t os_protect_init(void)
{
#ifdef OS_uCOS_II_H
  u8 err;
  prot_t t;
  t = OSMutexCreate(SWITCH_MUTEX_PRIO, &err);
  if (err == OS_ERR_NONE) {
    return t;
  }
  else {
    return 0;
  }
#else
  return 0xff;
#endif
}

static int os_protect_pend(prot_t t)
{
#ifdef OS_uCOS_II_H 
  u8 err;
  OSMutexPend(t, 0, &err);
  if(err != OS_ERR_NONE) {
    return err;
  }
  else {
    return 0;
  }
#else
  return 0xff;
#endif
}

static int os_protect_post(prot_t t)
{
#ifdef OS_uCOS_II_H
  u8 err;
  err = OSMutexPost(t);
  if(err == OS_ERR_NONE)
    return 0;
  else
    return err;
#else
  return 0xff;
#endif
}

int sw_dev_open(const char *name, GPIO_TypeDef *port, u16 pin, enum sw_level on_level, enum sw_state init_stat)
{
  struct switch_dev *dev;
  GPIO_InitTypeDef gpio_init;

  if (name == 0) {
    return SW_ERR_PARAM;
  }
  if (init_stat == SW_TOGGLE) {
    return SW_ERR_PARAM;
  }
  if (pt == 0) {
    pt = os_protect_init();
    if (pt == 0) {
      return SW_ERR_PROT;
    }
  }
  os_protect_pend(pt);
  for (dev = sw_list_head; dev != 0; dev = dev->next) {
    if (!strcmp(dev->name, name)) {
      os_protect_post(pt);
      return SW_ERR_EXIST;
    }
  }
  dev = (struct switch_dev *)MALLOC(sizeof(struct switch_dev));
  if (dev == 0) {
    os_protect_post(pt);
    return SW_ERR_NOMEM;
  }
  dev->name = name;
  dev->gpio_port = port;
  dev->gpio_pin = pin;
  dev->cur_state = init_stat;
  dev->on_level = on_level;
  dev->off_level = on_level == SW_SET ? SW_RESET : SW_SET;
  dev->next = sw_list_head;
  sw_list_head = dev;
  os_protect_post(pt);
  
  gpio_init.GPIO_Pin = dev->gpio_pin;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(dev->gpio_port, &gpio_init);
  if (dev->cur_state == SW_ON) {
    if (dev->on_level == SW_SET) {
      GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
    } else if (dev->on_level == SW_RESET) {
      GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
    }
  } else if (dev->cur_state == SW_OFF) {
    if (dev->off_level == SW_SET) {
      GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
    } else if (dev->off_level == SW_RESET) {
      GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
    }
  }
  return SW_ERR_NONE;
}

int sw_dev_close(const char *name)
{
  struct switch_dev *dev, *prev;
  GPIO_InitTypeDef gpio_init;

  if (name == 0) {
    return SW_ERR_PARAM;
  }
  if (pt == 0) {
    return SW_ERR_PROT;
  }
  os_protect_pend(pt);
  for (dev = sw_list_head; dev != 0; dev = dev->next) {
    if (!strcmp(dev->name, name)) {
      gpio_init.GPIO_Pin = dev->gpio_pin;
      gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      GPIO_Init(dev->gpio_port, &gpio_init);
      if (dev == sw_list_head) {
        sw_list_head = dev->next;
        FREE(dev);
      } else {
        prev->next = dev->next;
        FREE(dev);
      }
      os_protect_post(pt);
      return SW_ERR_NONE;
    }
    prev = dev;
  }
  os_protect_post(pt);
  return SW_ERR_NODEV;
}

int sw_dev_change(const char *name, enum sw_state new_state)
{
  struct switch_dev *dev;

  if (pt == 0) {
    return SW_ERR_PROT;
  }
  os_protect_pend(pt);
  for (dev = sw_list_head; dev != 0; dev = dev->next) {
    if (!strcmp(dev->name, name)) {
      if (new_state == SW_ON) {
        if (dev->on_level == SW_SET) {
          GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
        } else if (dev->on_level == SW_RESET) {
          GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
        }
        dev->cur_state = SW_ON;
      } else if (new_state == SW_OFF) {
        if (dev->off_level == SW_SET) {
          GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
        } else if (dev->off_level == SW_RESET) {
          GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
        }
        dev->cur_state = SW_OFF;
      } else if (new_state == SW_TOGGLE) {
        if (dev->cur_state == SW_ON) {
          if (dev->off_level == SW_SET) {
            GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
          } else if (dev->off_level == SW_RESET) {
            GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
          }
          dev->cur_state = SW_OFF;
        } else if (dev->cur_state == SW_OFF) {
          if (dev->on_level == SW_SET) {
            GPIO_SetBits(dev->gpio_port, dev->gpio_pin);
          } else if (dev->on_level == SW_RESET) {
            GPIO_ResetBits(dev->gpio_port, dev->gpio_pin);
          }
          dev->cur_state = SW_ON;
        } 
      }
      os_protect_post(pt);
      return SW_ERR_NONE;
    }
  }
  os_protect_post(pt);
  return SW_ERR_NODEV;
}
