#include "access_app.h"
#include "lwip/tcpip.h"
#include "led.h"
#include "switch_device.h"
#include "usart.h"
#include "key.h"
#include "lcd9341_fsmc.h"
#include "timer_event.h"
#include "rc522_function.h"
#include <string.h>
#include "global.h"
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

extern struct netif enc28j60;

//定义门禁系统变量并初始化状态
static struct access_system aSystem;

static struct access_record aRecord;  //定义门禁单条记录变量
static struct access_msg aMsg;  //定义handler和disp通信的消息
static OS_EVENT *mbox_access;   //定义用于通信的邮箱

OS_STK access_gettime_stk[64];
OS_STK access_disp_stk[128];
OS_STK access_handler_stk[128];


static void LCD_ShowStr16(u16 line, u16 col, const char *str, u16 width)
{
  u16 x,y;
  x = col * 8;
  y = line * 16;
  LCD_ShowStr(x, y, str, 16, 0, width);
}

static void LCD_ClearStr16(u16 line, u16 col, u16 words, u16 lines)
{
  u16 x, y, width, height;
  x = col * 8;
  y = line * 16;
  width = words * 8;
  height = lines * 16;
  LCD_FillColor(x, y, width, height, BackColor);
}


static char RFID_FindCard(struct Mifare1_S50_Info *cardinfo)
{
  char rc522_err;
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR cpu_sr;
#endif
  
  OS_ENTER_CRITICAL();
  //寻卡
  rc522_err = PcdRequest(PICC_REQIDL, cardinfo->tagtype);
  if ( rc522_err != MI_OK  ) {
    OS_EXIT_CRITICAL();
    return MI_ERR;
  }
  printf("TagType: %02X%02X\r\n", cardinfo->tagtype[0], cardinfo->tagtype[1]);
  
  //防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）
  rc522_err = PcdAnticoll(cardinfo->uid);
  if (rc522_err != MI_OK) {
    printf("PcdAntiColl() error with %02X\r\n", rc522_err);
    OS_EXIT_CRITICAL();
    return MI_ERR;
  }
  printf("Card ID: %02X%02X%02X%02X\r\n", cardinfo->uid [0], 
          cardinfo->uid[1], cardinfo->uid[2], cardinfo->uid[3]);
  
  //选择卡
  rc522_err = PcdSelect(cardinfo->uid);  
  if (rc522_err != MI_OK) {
    printf("PcdSelect() error with %02X\r\n", rc522_err);
    OS_EXIT_CRITICAL();
    return MI_ERR;
  }
  //使被选中的卡进入休眠
  PcdHalt();
  OS_EXIT_CRITICAL();
  return MI_OK;
}

static void set_time_callback(void *arg)
{
  struct access_system *asys = (struct access_system *)arg;
  uint8_t days_per_mon;

  asys->time.sec++;
  if (asys->time.sec >= 60) {
    asys->time.sec = 0;
    asys->time.minutes++;
  }
  if (asys->time.minutes >= 60) {
    asys->time.minutes = 0;
    asys->time.hour++;
  }
  if (asys->time.hour >= 24) {
    asys->time.hour = 0;
    asys->date.day++;
  }
  if (asys->date.month == 2) {
    if ((asys->date.year % 4 == 0 && asys->date.year % 100 != 0) 
      || asys->date.year % 400 == 0) {
      days_per_mon = 29; 
    } else {
      days_per_mon = 28;
    }  
  } else {
    switch (asys->date.month) {
      case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        days_per_mon = 31;
        break;
      case 4: case 6: case 9: case 11:
        days_per_mon = 30;
        break;
    }
  }
  if (asys->date.day > days_per_mon) {
    asys->date.day = 1;
    asys->date.month++;
  }
  if (asys->date.month > 12) {
    asys->date.month = 1;
    asys->date.year++;
  }
  asys->timer = timer_event_add(asys, set_time_callback, 1000);
  if (asys->timer == 0) {
    printf("%s(): register set time callback fail\r\n", __FUNCTION__);
  }
}

void access_gettime(void *arg)
{
  uint8_t aCmd;
  err_t err;
  struct netbuf *inbuf;
  uint8_t *p;
  u16_t len;
  
  arg = arg;
  puts("access_gettime() is created\r\n");
  while (1) {
    if (aSystem.conn_status == CONNECTED) {
      aCmd = ACCESS_CMD_GETTIME;
      err = netconn_write(aSystem.pconn, &aCmd, 1, 0);
      if (err != ERR_OK) {
        aSystem.conn_status = DISCONNECTED;
        aSystem.pconn = 0;
        goto __exit0;
      }
      //接收日期数据
      inbuf = netconn_recv(aSystem.pconn);
      if (inbuf == 0) {
        aSystem.conn_status = DISCONNECTED;
        aSystem.pconn = 0;
        goto __exit0;
      } else {
        netbuf_data(inbuf, (void**)&p, &len);
        memcpy(&aSystem.date, p, len);
        netbuf_delete(inbuf);
      }
      //接收时间数据
      inbuf = netconn_recv(aSystem.pconn);
      if (inbuf == 0) {
        aSystem.conn_status = DISCONNECTED;
        aSystem.pconn = 0;
        goto __exit0;
      } else {
        netbuf_data(inbuf, (void**)&p, &len);
        memcpy(&aSystem.time, p, len);
        netbuf_delete(inbuf);
      }
      if (aSystem.timer == 0) {
        aSystem.timer = timer_event_add(&aSystem, set_time_callback, 1000);
        if (aSystem.timer == 0) {
          printf("%s(): register set time callback fail\r\n", __FUNCTION__);
        }
      }
    }
__exit0:
    OSTimeDlyHMSM(0,0,30,0);  //延时20秒
  }
}


void access_disp(void *arg)
{
  void *pTemp;
  const struct access_msg *recvmsg;
  char dispbuf[60];
  u16 color;
  enum aOperate preMode = ACCESS;
  enum aStage preStage = AUTHENT;
  
  arg = arg;  //avoiding warning
  puts("access_disp() is created\r\n");
  while (1) {
    //显示本地IP地址
    pTemp = inet_ntoa(*(struct in_addr *)&(aSystem.local_addr)); 
    LCD_ShowStr16(0, 0, "Local Addr:", 11);
    LCD_ShowStr16(0, 11, (const char*)pTemp, 16);      
    
    //显示日期和时间
    sprintf(dispbuf, "%04d/%02d/%02d  %02d:%02d:%02d", aSystem.date.year, aSystem.date.month,
            aSystem.date.day, aSystem.time.hour, aSystem.time.minutes, aSystem.time.sec);
    LCD_ShowStr16(2, 0, dispbuf, 20);
  
    //显示连接状态
    if (aSystem.conn_status == CONNECTED) {
      color = FontColor;
      FontColor = GREEN;
      LCD_ShowStr16(1, 0, "CONNECTED", 12);
      FontColor = color;
      
      if (aSystem.cur_mode != preMode || aSystem.stage != preStage) {
        LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);  //清除提示信息显示区域
        preMode = aSystem.cur_mode;
        preStage = aSystem.stage;
      }
      //显示模式
      LCD_ShowStr16(3, 0, "MODE:", 5);
      switch (aSystem.cur_mode) {
        case ACCESS:
          LCD_ShowStr16(3, 5, "ACCESS", 10);
          if ((recvmsg = OSMboxAccept(mbox_access)) != NULL) {
            if (recvmsg->type == ACCESS) {
              if (recvmsg->value == ACCESS_BACK_OK) {
                //show card id
                sprintf(dispbuf, "ID  : %02X%02X%02X%02X", recvmsg->pRecord->card_id[0], 
                        recvmsg->pRecord->card_id[1], recvmsg->pRecord->card_id[2],
                        recvmsg->pRecord->card_id[3]);
                LCD_ShowStr16(9, 0, dispbuf, lcd_param.width / 8);
                //show name
                pTemp = recvmsg->pRecord->name;
                if (((char *)pTemp)[0] != '0') {
                  sprintf(dispbuf, "Name: %s", (char *)pTemp);
                  LCD_ShowStr16(10, 0, dispbuf, lcd_param.width / 8);
                } else {
                  LCD_ShowStr16(10, 0, "Name: N/A", lcd_param.width / 8);
                }
                //show num
                pTemp = recvmsg->pRecord->ext_id;
                if (((char *)pTemp)[0] != '0') {
                  sprintf(dispbuf, "Num : %s", (char *)pTemp);
                  LCD_ShowStr16(11, 0, dispbuf, lcd_param.width / 8);
                } else {
                  LCD_ShowStr16(11, 0, "Num : N/A", lcd_param.width / 8);
                }
                //show time
                sprintf(dispbuf, "Time: %02d:%02d:%02d", recvmsg->pRecord->time.hour, 
                        recvmsg->pRecord->time.minutes, recvmsg->pRecord->time.sec);
                LCD_ShowStr16(12, 0, dispbuf, lcd_param.width / 8);
                OSTimeDlyHMSM(0, 0, 2, 0);  // 停留2s
                LCD_ClearStr16(9, 0, lcd_param.width / 8, 4);  //清除信息显示区域
              }
              else if (recvmsg->value == ACCESS_BACK_NOTFOUND) {
                LCD_ShowStr16(10, 3, "ID Not Found In System!", 23);
                OSTimeDlyHMSM(0,0,2,0);  //停留2s
                LCD_ClearStr16(10, 3, 23, 1);
              }
            }
          } else { //recvmsg not NULL
            LCD_ShowStr16(10, 7, "Waiting For Card", 16);
          }
          break;
          
          
        case REGISTER:
          LCD_ShowStr16(3, 5, "REGISTER", 10);
          if (aSystem.stage == AUTHENT) {
            if ((recvmsg = OSMboxAccept(mbox_access)) != NULL) {
              if (recvmsg->type == REGISTER) {
                LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);  //清除提示信息显示区域
                if (recvmsg->value == ACCESS_BACK_AUTH_ERR) {
                  LCD_ShowStr16(10, 4, "No Administrator Card!", 22);
                  OSTimeDlyHMSM(0, 0, 2, 0);
                  LCD_ClearStr16(10, 4, 22, 1);
                }
                else if (recvmsg->value == ACCESS_BACK_AUTH_OK) {
                  LCD_ShowStr16(10, 8, "Authent Pass", 12);
                  OSTimeDlyHMSM(0, 0, 2, 0);
                  LCD_ClearStr16(10, 8, 12, 1);
                }
              }
            } else {
              LCD_ShowStr16(10, 2, "Please Swipe the Administrator Card", 26);
            }
          }
          else if (aSystem.stage == NORMAL) {
            if ((recvmsg = OSMboxAccept(mbox_access)) != NULL) {
              if (recvmsg->type == REGISTER) {
                if (recvmsg->value == ACCESS_BACK_OK) {
                  sprintf(dispbuf, "ID %02X%02X%02X%02X Register Successfully", recvmsg->pRecord->card_id[0], 
                          recvmsg->pRecord->card_id[1], recvmsg->pRecord->card_id[2],
                          recvmsg->pRecord->card_id[3]);
                  LCD_ShowStr16(10, 2, dispbuf, 26);
                  OSTimeDlyHMSM(0, 0, 2, 0);  //停留2s
                  LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);
                }
                else if (recvmsg->value == ACCESS_BACK_EXISTING) {
                  LCD_ShowStr16(10, 2, "Card Already Exist, Register Fail", 26);
                  OSTimeDlyHMSM(0, 0, 2, 0);
                  LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);
                }
              }
            } else { //recvmsg not NULL
              LCD_ShowStr16(10, 7, "Waiting For Card", 16);
            }
          }
          break;
          
          
        case REMOVE:
          LCD_ShowStr16(3, 5, "REMOVE", 10);
          if (aSystem.stage == AUTHENT) {
            if ((recvmsg = OSMboxAccept(mbox_access)) != NULL) {
              if (recvmsg->type == REMOVE) {
                LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);  //清除提示信息显示区域
                if (recvmsg->value == ACCESS_BACK_AUTH_ERR) {
                  LCD_ShowStr16(10, 4, "No Administrator Card!", 22);
                  OSTimeDlyHMSM(0,0,2,0);
                  LCD_ClearStr16(10, 4, 22, 1);
                }
                else if (recvmsg->value == ACCESS_BACK_AUTH_OK) {
                  LCD_ShowStr16(10, 8, "Authent Pass", 12);
                  OSTimeDlyHMSM(0, 0, 2, 0);
                  LCD_ClearStr16(10, 8, 12, 1);
                }
              }
            } else {
              LCD_ShowStr16(10, 2, "Please Swipe the Administrator Card", 26);
            }
          }
          else if (aSystem.stage == NORMAL) {
            if ((recvmsg = OSMboxAccept(mbox_access)) != NULL) {
              if (recvmsg->type == REMOVE) {
                if (recvmsg->value == ACCESS_BACK_OK) {
                  sprintf(dispbuf, "ID %02X%02X%02X%02X Remove Successfully", recvmsg->pRecord->card_id[0], 
                          recvmsg->pRecord->card_id[1], recvmsg->pRecord->card_id[2],
                          recvmsg->pRecord->card_id[3]);
                  LCD_ShowStr16(10, 2, dispbuf, 26);
                  OSTimeDlyHMSM(0, 0, 2, 0);  //停留2s
                  LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);
                }
                else if (recvmsg->value == ACCESS_BACK_NOTFOUND) {
                  LCD_ShowStr16(10, 2, "ID Dosen't Exist, Remove Fail!", 26);
                  OSTimeDlyHMSM(0, 0, 2, 0);
                  LCD_ClearStr16(10, 0, lcd_param.width / 8, 2);
                }
              }
            } else { //recvmsg not NULL
              LCD_ShowStr16(10, 7, "Waiting For Card", 16);
            }
          }
          break;
        
        
        case QUERY:
          LCD_ShowStr16(3, 5, "QUERY", 10);
          break;
        
        
        default:
          color = FontColor;
          FontColor = RED;
          LCD_ShowStr16(10, 2, "Mode Error, Please Check", 26);
          OSTimeDlyHMSM(0 ,0, 2, 0);  //停留1s
          FontColor = color;
      }  //switch(aSystem.cur_mode)
    } else { //if(aSystem.conn_status == CONNECTED)
      color = FontColor; 
      FontColor = RED;
      LCD_ShowStr16(1, 0, "DISCONNECTED", 12);
      FontColor = color;
    }
    OSTimeDlyHMSM(0, 0, 0, 250);  //延时
  } //while 1
}


static void sw_dev_callback(void *arg)
{
  int id;
  u8 *data = (u8 *)arg;
  u8 *times = &data[6];
  u8 isblink = data[1];
  u32 msec;
  const char *name;
  
  name = (const char *)(*((u32 *)&data[7]));
  data[0] = (u8)((enum sw_state)data[0] == SW_ON ? SW_OFF : SW_ON);
  sw_dev_change(name, (enum sw_state)data[0]);
  if (*times != 0xff) {
    *times = *times - 1;
  }
  if (isblink && *times) {  // if blink
    memcpy(&msec, data + 2, 4);
    id = timer_event_add(data, sw_dev_callback, msec);
    if (id == 0) {
      free_safe(data);
      printf("%s() timer event add fail\r\n", __FUNCTION__);
    }
    //timer_event_del(id);  //中断中删除测试
  } else {
    free_safe(data); 
  }
}

static int sw_dev_blink(const char *name, u8 isblink, u32 msec, u8 times)
{
  int id, err;
  u8 *data;
  
  data = (u8 *)malloc_safe(20);
  if (data == NULL) {
    printf("%s() malloc fail\r\n", __FUNCTION__);
    return -10;
  }
  data[0] = (u8)SW_ON;
  data[1] = isblink;
  memcpy(data + 2, (u8 *)&msec, 4);
  if (times != 0xff) {
    data[6] = (u8)(times * 2 -1);
  } else {
    data[6] = 0xff;
  }
  data[7] = (u8)((u32)name & 0x000000ff);
  data[8] = (u8)(((u32)name & 0x0000ff00) >> 8);
  data[9] = (u8)(((u32)name & 0x00ff0000) >> 16);
  data[10] = (u8)(((u32)name & 0xff000000) >> 24);
  err = sw_dev_change(name, (enum sw_state)data[0]);
  if (err != SW_ERR_NONE) {
    printf("%s() return err with %d\r\n", __FUNCTION__, err);
    free_safe(data);
    return err;
  }
  id = timer_event_add(data, sw_dev_callback, msec);
  if (id == 0) {
    printf("%s() timer event add fail\r\n", __FUNCTION__);
  }
  //timer_event_del(id);  //中断外删除测试
  return 0;
}


static void door_unlock_callback(void *arg)
{
  sw_dev_change("relay", SW_OFF); 
}

static int door_unlock(void)
{
  int retval = 0;
  sw_dev_change("relay", SW_ON);
  retval = timer_event_add(0, door_unlock_callback, 5000);
  return !retval;
}


void access_handler(void *arg)
{
  struct netconn *conn;
  struct ip_addr serv_addr;
  err_t err;
  struct netbuf *inbuf;
  char *pData;
  u16 len, i;
  struct Mifare1_S50_Info cInfo;  //射频卡信息结构体
  u8 aCmd, AuthErrCnt = 0;
  u8 keyval;
  
  arg = arg; //avoiding warnning
  puts("access_handler() is created\r\n");  
    
  memset(&aSystem, 0, sizeof(struct access_system));
  aSystem.conn_status = DISCONNECTED;
  aSystem.cur_mode = ACCESS;
  aSystem.stage = AUTHENT;
  IP4_ADDR(&serv_addr, 172, 16, 23, 22);  //指定服务器地址
  do {
    mbox_access = OSMboxCreate(NULL);  //创建任务间通信的邮箱
  } while(!mbox_access);
    
  while (1) {
#if LWIP_DHCP
    while (enc28j60.dhcp->state != DHCP_BOUND) {
      OSTimeDly(100);
    }
    aSystem.local_addr = enc28j60.dhcp->offered_ip_addr;
#else
    aSystem.local_addr = enc28j60.ip_addr;
#endif
    
    do {
      conn = netconn_new(NETCONN_TCP);
      if (conn == NULL) {
        puts("netconn_new() fail!\r\n");
        OSTimeDly(100);
      }
    } while(conn == NULL);
    err = netconn_connect(conn, &serv_addr, 8080);
    if (err == ERR_OK) {
      aSystem.conn_status = CONNECTED;
      aSystem.pconn = conn;
      OSTimeDlyResume(12);
      while (aSystem.conn_status == CONNECTED) {
        if (RFID_FindCard(&cInfo) == MI_OK) {
          sw_dev_blink("beep", 1, 100, 2);
          sw_dev_blink("led0", 1, 100, 2);
          
          switch (aSystem.cur_mode) {
            case ACCESS:
              aCmd = ACCESS_CMD_ACCESS;
              err = netconn_write(conn, &aCmd, 1, 1);
              if (err != ERR_OK) {
                printf("ACCESS MODE: netconn_write() fail with %d while sending command\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              //向服务器发送读出的卡ID
              for (i = 0; i < 4; i++)
                aRecord.card_id[i] = cInfo.uid[i];
              err = netconn_write(conn, (const void*)&aRecord, sizeof(struct access_record), 1);
              if (err != ERR_OK) {
                printf("ACCESS MODE: netconn_write() fail with %d while sending record\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              
              //接收服务器的反馈
              inbuf = netconn_recv(conn);
              if (inbuf == NULL) {
                printf("ACCESS MODE: netconn_recv() fail with %d while receiving backlog\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              netbuf_data(inbuf, (void**)&pData, &len);
              if (*pData == ACCESS_BACK_OK) {
                puts("ACCESS MODE: get backlog \"ACCESS_BACK_OK\"\r\n");
                netbuf_delete(inbuf);
                inbuf = netconn_recv(conn);
                if (inbuf == NULL) {
                  printf("ACCESS MODE: netconn_recv() fail with %d while receiving record\r\n", err);
                  aSystem.conn_status = DISCONNECTED;
                  break;
                }
                netbuf_data(inbuf, (void**)&pData, &len);
                if (len != 0) {
                  puts("ACCESS MODE: succeed to get record\r\n");
                  memcpy(&aRecord, pData, len);
                  aMsg.value = ACCESS_BACK_OK;
                  aMsg.pRecord = &aRecord;
                } else {
                  puts("ACCESS MODE: fail to get record\r\n");
                  aMsg.value = ACCESS_BACK_NOTFOUND;
                  aMsg.pRecord = NULL;
                }
                door_unlock();  //开门
                netbuf_delete(inbuf);
              }
              else if (*pData == ACCESS_BACK_NOTFOUND) {
                puts("ACCESS MODE: get backlog \"ACCESS_BACK_NOTFOUND\"\r\n");
                netbuf_delete(inbuf);
                aMsg.value = ACCESS_BACK_NOTFOUND;
                aMsg.pRecord = NULL;
              }
              aMsg.type = ACCESS;
              while(OSMboxPost(mbox_access, &aMsg) == OS_ERR_MBOX_FULL)
                OSTimeDly(10);
              break;
              
              
            case REGISTER:
              if (aSystem.stage == AUTHENT) {
                aCmd = ACCESS_CMD_REG_AUTH;
              } 
              else if (aSystem.stage == NORMAL) {
                aCmd = ACCESS_CMD_REGISTER;
              }
              err = netconn_write(conn, &aCmd, 1, 1);
              if (err != ERR_OK) {
                printf("REGISTER MODE: netconn_write() fail with %d while sending command\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              //向服务器发送读出的卡ID
              for (i = 0; i < 4; i++)
                aRecord.card_id[i] = cInfo.uid[i];
              err = netconn_write(conn, (const void*)&aRecord, sizeof(struct access_record), 1);
              if (err != ERR_OK) {
                printf("REGISTER MODE: netconn_write() fail with %d while sending record\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              
              //接收服务器的反馈
              inbuf = netconn_recv(conn);
              if (inbuf == NULL) {
                printf("REGISTER MODE: netconn_recv() fail with %d while receiving backlog\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              netbuf_data(inbuf, (void**)&pData, &len);
              if (aSystem.stage == AUTHENT) {
                if (*pData == ACCESS_BACK_AUTH_OK) {
                  puts("REGISTER MODE: get backlog \"ACCESS_BACK_AUTH_OK\"\r\n");     
                  aSystem.stage = NORMAL;
                  aMsg.value = ACCESS_BACK_AUTH_OK;
                }
                else if (*pData == ACCESS_BACK_AUTH_ERR) {
                  puts("REGISTER MODE: get backlog \"ACCESS_BACK_AUTH_ERR\"\r\n");
                  AuthErrCnt++;
                  if (AuthErrCnt >= 3) {  //认证失败超过三次，返回访问模式
                    aSystem.cur_mode = ACCESS;
                    AuthErrCnt = 0;
                  }
                  aMsg.value = ACCESS_BACK_AUTH_ERR;
                }
              }
              else if (aSystem.stage == NORMAL) {
                if (*pData == ACCESS_BACK_OK) {
                  puts("REGISTER MODE: get backlog \"ACCESS_BACK_OK\"\r\n");
                  aMsg.value = ACCESS_BACK_OK;
                }
                else if (*pData == ACCESS_BACK_EXISTING) {
                  puts("REGISTER MODE: get backlog \"ACCESS_BACK_EXISTING\"\r\n");
                  aMsg.value = ACCESS_BACK_EXISTING;
                }
              }
              netbuf_delete(inbuf);
              aMsg.type = REGISTER;
              aMsg.pRecord = &aRecord;
              while (OSMboxPost(mbox_access, &aMsg) == OS_ERR_MBOX_FULL) {
                OSTimeDly(10);
              }
              break;
            
              
            case REMOVE:
              if (aSystem.stage == AUTHENT) {
                aCmd = ACCESS_CMD_RM_AUTH;
              } 
              else if(aSystem.stage == NORMAL) {
                aCmd = ACCESS_CMD_REMOVE;
              }
              err = netconn_write(conn, &aCmd, 1, 1);
              if (err != ERR_OK) {
                printf("REMOVE MODE: netconn_write() fail with %d while sending command\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              //向服务器发送读出的卡ID
              for (i = 0; i < 4; i++)
                aRecord.card_id[i] = cInfo.uid[i];
              err = netconn_write(conn, (const void*)&aRecord, sizeof(struct access_record), 1);
              if (err != ERR_OK) {
                printf("REMOVE MODE: netconn_write() fail with %d while sending record\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              
              //接收服务器的反馈
              inbuf = netconn_recv(conn);
              if (inbuf == NULL) {
                printf("REMOVE MODE: netconn_recv() fail with %d while receiving backlog\r\n", err);
                aSystem.conn_status = DISCONNECTED;
                aSystem.pconn = NULL;
                break;
              }
              netbuf_data(inbuf, (void**)&pData, &len);
              if (aSystem.stage == AUTHENT) {
                if (*pData == ACCESS_BACK_AUTH_OK) {
                  puts("REMOVE MODE: get backlog \"ACCESS_BACK_AUTH_OK\"\r\n");     
                  aSystem.stage = NORMAL;
                  aMsg.value = ACCESS_BACK_AUTH_OK;
                }
                else if (*pData == ACCESS_BACK_AUTH_ERR) {
                  puts("REMOVE MODE: get backlog \"ACCESS_BACK_AUTH_ERR\"\r\n");
                  AuthErrCnt++;
                  if (AuthErrCnt >= 3) {  //认证失败超过三次，返回访问模式
                    aSystem.cur_mode = ACCESS;
                    AuthErrCnt = 0;
                  }
                  aMsg.value = ACCESS_BACK_AUTH_ERR;
                }
              }
              else if (aSystem.stage == NORMAL) {
                if (*pData == ACCESS_BACK_OK) {
                  puts("REMOVE MODE: get backlog \"ACCESS_BACK_OK\"\r\n");
                  aMsg.value = ACCESS_BACK_OK;
                }
                else if (*pData == ACCESS_BACK_NOTFOUND) {
                  puts("REMOVE MODE: get backlog \"ACCESS_BACK_NOTFOUND\"\r\n");
                  aMsg.value = ACCESS_BACK_NOTFOUND;
                }
              }
              netbuf_delete(inbuf);
              aMsg.type = REMOVE;
              aMsg.pRecord = &aRecord;
              while (OSMboxPost(mbox_access, &aMsg) == OS_ERR_MBOX_FULL) {
                OSTimeDly(10);
              }
              break;
            
              
            case QUERY:
              break;
            
            
            default:
              aSystem.cur_mode = ACCESS;
          }  //switch(aSystem.cur_mode)          
        }   // judge if card read pass
        
        
        //获取按键值
        if ((keyval = KeyScan()) != KEY_NONE) {
          AuthErrCnt = 0;  //错误记数清零
          aSystem.stage = AUTHENT;  //重置为认证阶段
          switch (aSystem.cur_mode) {  //有限状态机模式切换
            case ACCESS:
              if (keyval == KEY_WK) {
                aSystem.cur_mode = REMOVE;
              }
              else if (keyval == KEY0) {
                aSystem.cur_mode = REGISTER;
              }
              break;
            case REGISTER:
              if (keyval == KEY_WK) {
                aSystem.cur_mode = ACCESS;
              }
              else if (keyval == KEY0) {
                aSystem.cur_mode = REMOVE;
              }
              break;
            case REMOVE:
              if (keyval == KEY_WK) {
                aSystem.cur_mode = REGISTER;
              }
              else if (keyval == KEY0) {
                aSystem.cur_mode = ACCESS;
              }
              break;
            case QUERY:
              break;
            default:
              aSystem.cur_mode = ACCESS;
          }
        }
        OSTimeDly(20);
      }  //while(aSystem.conn_status)
      netconn_close(conn);
      netconn_delete(conn);
      OSTimeDly(200);
    } else {  //netconn_connect()
      puts("netconn_connect() timeout!\r\n");
      puts("Please check,repair and reset.\r\n");
      aSystem.conn_status = DISCONNECTED;
      OSTaskDel(OS_PRIO_SELF);  //删除任务自身
    }
  }  //while(1)
}
