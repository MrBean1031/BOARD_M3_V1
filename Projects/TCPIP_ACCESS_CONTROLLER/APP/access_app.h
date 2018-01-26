#ifndef __ACCESS_APP_H
#define __ACCESS_APP_H

#include"stm32f10x.h"
#include"lwip/ip_addr.h"
#include"includes.h"

enum aOperate {ACCESS, REGISTER, REMOVE, QUERY};
enum aStage {AUTHENT, NORMAL};

struct access_system {
  struct ip_addr local_addr;
  enum {CONNECTED, DISCONNECTED} conn_status;
  struct netconn *pconn;
  struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
  } date;
  struct {
    uint8_t hour;
    uint8_t minutes;
    uint8_t sec;
  } time;
  int timer;
  enum aOperate cur_mode;
  enum aStage stage;  //模式的阶段，认证阶段和常规阶段
};


#define ACCESS_CMD_ACCESS      0x01
#define ACCESS_CMD_REG_AUTH    0x02
#define ACCESS_CMD_REGISTER    0x03
#define ACCESS_CMD_RM_AUTH     0x04
#define ACCESS_CMD_REMOVE      0x05
#define ACCESS_CMD_QUERY       0X06
#define ACCESS_CMD_GETTIME     0x07

#define ACCESS_BACK_OK         0x10
#define ACCESS_BACK_NOTFOUND   0x11
#define ACCESS_BACK_EXISTING   0x12
#define ACCESS_BACK_AUTH_OK    0x13
#define ACCESS_BACK_AUTH_ERR   0x14
#define ACCESS_BACK_INVALID    0x1f


#define ACCESS_RECORD_MAX_NAME_LEN      20
#define ACCESS_RECORD_MAX_EXT_ID_LEN    12

struct access_record {
  uint8_t card_id[4];
  uint8_t name[ACCESS_RECORD_MAX_NAME_LEN];
  uint8_t ext_id[ACCESS_RECORD_MAX_EXT_ID_LEN];  //use as member id
  enum aOperate behavior;
  uint16_t auth_level;  //权限等级
  struct {
    uint8_t hour;
    uint8_t minutes;
    uint8_t sec;
  } time;
};

/* 任务间通信消息 */
struct access_msg {
  enum aOperate type;
  u8 value;
  struct access_record *pRecord;
};

struct Mifare1_S50_Info {
  u8 tagtype[2];
  u8 uid[4];
  u8 capacity;
  u8 CurSector;
  u8 SectorKey[6];
  u8 ReadBuff[16];
  u8 WriteBuff[16];
};

/* Declarations */
//variables
extern OS_STK access_gettime_stk[64];
extern OS_STK access_disp_stk[128];
extern OS_STK access_handler_stk[128];
//functions
void access_gettime(void *arg);
void access_disp(void *arg);
void access_handler(void *arg);


#endif
