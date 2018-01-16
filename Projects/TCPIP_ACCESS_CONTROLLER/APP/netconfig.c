#include "netconfig.h"	
#include "lwip/err.h"  
#include "lwip/init.h"
#include "lwip/tcpip.h"
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

 	
struct netif enc28j60;	  		//netif 网络接口结构，用于描述网络硬件接口的特性
uint8_t macaddress[6]={0x54,0x55,0x58,0x10,0x00,0x24};	//设置网络接口的mac地址

/* 外部函数 */
extern err_t ethernetif_init(struct netif *netif);  //在ethernetif.c中定义

/*
 * 函数名：LWIP_Init
 * 描述  ：初始化LWIP协议栈，主要是把enc28j60与LWIP连接起来。
 			     包括IP、MAC地址，接口函数
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;

  /* tcpip_init()调用lwip_init()
     初始化网络接口结构体链表、内存池、pbuf结构体
     并创建tcpip_thread()内核线程 */
  tcpip_init(NULL, NULL);
	  
#if LWIP_DHCP			   					//若使用DHCP协议
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0; 
#else										
  IP4_ADDR(&ipaddr, 192, 168, 0, 254);   //设置网络接口的ip地址
  IP4_ADDR(&netmask, 255, 255, 255, 0);  //子网掩码
  IP4_ADDR(&gw, 192, 168, 0, 1);			   //网关
#endif
   
  /* 初始化enc28j60与LWIP的接口，参数为网络接口结构体、ip地址、
     子网掩码、网关、网卡信息指针、初始化函数、输入函数 */
  netif_add(&enc28j60, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
 
  /* 把enc28j60设置为默认网卡 */
  netif_set_default(&enc28j60);


#if LWIP_DHCP	   			      //若使用了DHCP
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  dhcp_start(&enc28j60);   	//启动DHCP
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&enc28j60); //使能enc28j60接口
}

