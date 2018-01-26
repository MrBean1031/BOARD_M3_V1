/*
 *File Description: LwIP操作系统模拟层移植
 *Pin Mapping:      None
 *Author:           Mr.Bean
 *Date:             2017/03/29
 *Attention:        OS Specified: uCOS-II
                    使用uCOS-II的内存管理函数分配邮箱空间和线程堆栈
 */
#include "lwip/sys.h"
#include "lwip/err.h"
#include "cc.h"

static OS_MEM *MboxMem;
static u8_t MboxMemArea[TOTAL_MBOX_NUM][sizeof(struct sys_mbox)];
static struct sys_timeouts TimeoutsList;
static const u32_t NullMsg;
static u16_t thread_mount[MAX_LWIP_THREADS];
static OS_STK thread_stk[MAX_LWIP_THREADS][THREAD_STK_SIZE];

/*-----------------------------------------------------
 - Function Name: sys_init()
 - Description:
 - Input:         None
 - Output:        None
 - Return:        None
 - Attention:     Is called to initialize the sys_arch layer.
-----------------------------------------------------*/
void sys_init(void)
{
  u8_t err;

  MboxMem = OSMemCreate((void*)MboxMemArea,TOTAL_MBOX_NUM,
                        sizeof(struct sys_mbox),&err);
  TimeoutsList.next = NULL;
  for (err = 0; err < MAX_LWIP_THREADS; err++) {
    thread_mount[err] = 0;
  }
}

/*-----------------------------------------------------
 - Function Name: sys_sem_new()
 - Description:
 - Input:         count
 - Output:        None
 - Return:        sys_sem_t type semaphore
 - Attention:     Creates and returns a new semaphore. The "count" argument specifies
                  the initial state of the semaphore.
-----------------------------------------------------*/
sys_sem_t sys_sem_new(u8_t count)
{
  return OSSemCreate((u16_t)count);
}

/*-----------------------------------------------------
 - Function Name: sys_sem_free()
 - Description:
 - Input:         sem
 - Output:        None
 - Return:        None
 - Attention:     Deallocates a semaphore.
-----------------------------------------------------*/
void sys_sem_free(sys_sem_t sem)
{
  u8_t err;
  OSSemDel(sem,OS_DEL_ALWAYS,&err);  //删除一个信号量，不检查返回值
}

/*-----------------------------------------------------
 - Function Name: sys_sem_signal()
 - Description:
 - Input:         sem
 - Output:        None
 - Return:        None
 - Attention:     Signals a semaphore.
-----------------------------------------------------*/
void sys_sem_signal(sys_sem_t sem)
{
  OSSemPost(sem);  //发送一个信号量，不检查返回值
}

/*-----------------------------------------------------
 - Function Name: sys_arch_sem_wait()
 - Description:
 - Input:         sem, timeout
 - Output:        None
 - Return:        the waiting time
 - Attention:     等待信号量，若timeout为0,则一直阻塞直到信号量到来；若timeout不为0，
                  则阻塞最长时间为timeout(ms)，成功获取信号量则返回等待时间(ms)，超时
                  就返回SYS_ARCH_TIMEOUT。信号量在函数调用时可用，可以返回0。
-----------------------------------------------------*/
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
  u32_t ticks;
  u8_t err;
  OS_SEM_DATA semdata;

  OSSemQuery(sem,&semdata);
  if (timeout!=0) {
    ticks = timeout*OS_TICKS_PER_SEC/1000;
    if (ticks==0) {
      ticks=1;
    }
    timeout = OSTimeGet();
    OSSemPend(sem, ticks, &err);
    timeout = (OSTimeGet()>timeout? OSTimeGet()-timeout:
               0xFFFFFFFF-timeout+OSTimeGet())
              * 1000/OS_TICKS_PER_SEC;
    if (err == OS_ERR_NONE) {
      if(semdata.OSCnt == 0) {  //信号量为0，需要等待
        return timeout;
      }
    } else {
      return SYS_ARCH_TIMEOUT;
    }
  } else {
    OSSemPend(sem, 0, &err);
    if(err != OS_ERR_NONE)
      return SYS_ARCH_TIMEOUT;
  }
  return 0;
}


/*-----------------------------------------------------
 - Function Name: sys_mbox_new()
 - Description:
 - Input:         size - the size of mbox,but is ignored in this implementation
 - Output:        None
 - Return:        sys_mbox_t type mbox
 - Attention:     Creates an empty mailbox for maximum "size" elements. Elements stored
                  in mailboxes are pointers. You have to define macros "_MBOX_SIZE"
                  in your lwipopts.h, or ignore this parameter in your implementation
                  and use a default size.
-----------------------------------------------------*/
sys_mbox_t sys_mbox_new(int size)
{
  sys_mbox_t mbox;
  u8_t err;

  size = size;  //avoid compiler warnning
  mbox = (sys_mbox_t)OSMemGet(MboxMem, &err);
  if (err == OS_ERR_NONE) {
    mbox->queue = OSQCreate(mbox->MsgBox, MAX_MSG_IN_MBOX);
    if (mbox->queue == (OS_EVENT*)0) {
      OSMemPut(MboxMem,mbox);  //分配队列控制块失败则释放内存块
      mbox = SYS_MBOX_NULL;
    }
  } else {
    mbox = SYS_MBOX_NULL;
  }
  return mbox;
}

/*-----------------------------------------------------
 - Function Name: sys_mbox_free()
 - Description:
 - Input:         mbox
 - Output:        None
 - Return:        None
 - Attention:     Deallocates a mailbox. If there are messages still present in the
                  mailbox when the mailbox is deallocated, it is an indication of a
                  programming error in lwIP and the developer should be notified.
-----------------------------------------------------*/
void sys_mbox_free(sys_mbox_t mbox)
{
  u8_t err;

  OSQFlush(mbox->queue);
  OSQDel(mbox->queue, OS_DEL_ALWAYS, &err);
  OSMemPut(MboxMem, mbox);  // 释放邮箱申请的内存
}

/*-----------------------------------------------------
 - Function Name: sys_mbox_post()
 - Description:
 - Input:         mbox, msg
 - Output:        None
 - Return:        None
 - Attention:     发送一条消息到邮箱，若邮箱是满的，则阻塞线程，直到发送成功；LwIP可能
                  发送空的消息NULL，但uCOS-II不支持NULL消息的投递，需要对NULL进行转换
-----------------------------------------------------*/
void sys_mbox_post(sys_mbox_t mbox, void *msg)
{
  if(msg == NULL)
    msg = (void*)&NullMsg;
  /* 如果队列满，则阻塞当前线程并切换到其他线程 */
  while(OSQPost(mbox->queue,msg) == OS_ERR_Q_FULL)
    OSTimeDly(10);
}

/*-----------------------------------------------------
 - Function Name: sys_mbox_trypost()
 - Description:
 - Input:         mbox, msg
 - Output:        None
 - Return:        err information
 - Attention:     尝试发送一条消息到邮箱，如果发送成功就返回ERR_OK，失败则返回ERR_MEM；
                  LwIP可能发送空的消息NULL，但uCOS-II不支持NULL消息的投递，需要对NULL
                  进行转换
-----------------------------------------------------*/
err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg)
{
  if(msg == NULL) {
    msg = (void*)&NullMsg;
  }
  if (OSQPost(mbox->queue, msg) != OS_ERR_NONE) {
    return ERR_MEM;
  }
  return ERR_OK;
}

/*-----------------------------------------------------
 - Function Name: sys_arch_fetch()
 - Description:
 - Input:         mbox, timeout
 - Output:        msg
 - Return:        the waiting time
 - Attention:     从mbox获取一条消息，若timeout为0，则一直阻塞直到收到消息；若timeout
                  不为0，则最长等待时间为timeout(ms)，在等待时间内收到消息就返回等待时
                  间，超时则返回SYS_ARCH_TIMEOUT；若函数调用时邮箱非空，可以返回0。可
                  能收到一条空消息，需要进行转换为NULL
-----------------------------------------------------*/
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
  u32_t ticks;
  u8_t err;
  OS_Q_DATA q_data;

  OSQQuery(mbox->queue, &q_data);
  if (timeout != 0) {
    ticks = timeout*OS_TICKS_PER_SEC/1000;
    if(ticks == 0) {
      ticks = 1;
    }
    timeout = OSTimeGet();
    *msg = OSQPend(mbox->queue, ticks, &err);
    timeout = (OSTimeGet()>timeout? OSTimeGet()-timeout:
               0xFFFFFFFF-timeout+OSTimeGet())
              *1000/OS_TICKS_PER_SEC;
    if (err == OS_ERR_NONE) {
      if(*msg == (void*)&NullMsg) {  //收到一条空消息
        *msg = NULL;
      }
      if (q_data.OSNMsgs == 0) {  //邮箱为空，需要等待消息
        return timeout;
      }
    } else {
      *msg = NULL;  //超时，丢弃该消息
      return SYS_ARCH_TIMEOUT;
    }
  } else {
    *msg = OSQPend(mbox->queue, 0, &err);
    if (*msg == (void*)&NullMsg) {  //收到一条空消息
      *msg = NULL;
    }
    if (err != OS_ERR_NONE) {
      *msg = NULL;  //发生错误，丢弃该消息
      return SYS_ARCH_TIMEOUT;
    }
  }
  return 0;
}

/*-----------------------------------------------------
 - Function Name: sys_arch_mbox_tryfetch()
 - Description:
 - Input:         mbox
 - Output:        msg
 - Return:        the result of calling
 - Attention:     尝试从mbox等待一条消息，不阻塞线程，若邮箱不为空就返回0，否则返回
                  SYS_MBOX_EMPTY
-----------------------------------------------------*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg)
{
  u8_t err;

  *msg = OSQAccept(mbox->queue, &err);
  if (err == OS_ERR_NONE) {
    if(*msg == (void*)&NullMsg) {
      *msg = NULL;
    }
    return 0;
  }
  return SYS_MBOX_EMPTY;
}

/*-----------------------------------------------------
 - Function Name: sys_arch_timeouts()
 - Description:
 - Input:         None
 - Output:        None
 - Return:        指向struct sys_timeouts类型的指针变量
 - Attention:     返回系统定时时间链表头指针
-----------------------------------------------------*/
struct sys_timeouts *sys_arch_timeouts(void)
{
  return &TimeoutsList;
}

/*-----------------------------------------------------
 - Function Name: sys_thread_new()
 - Description:
 - Input:         name,      线程名称
                  thread,    指向线程函数的指针
                  arg,       线程创建时传入的参数
                  stacksize, 线程堆栈字节数
                  prio       线程优先级
 - Output:        None
 - Return:        the id of the new created thread
 - Attention:     创建一个新线程
-----------------------------------------------------*/
sys_thread_t sys_thread_new(char *name,
                            void (*thread)(void *arg),
                            void *arg,
                            int stacksize,
                            int prio )
{
  u16_t err, i, j;

  name = name;  //ignore argument name,avoid compiler warnning
  stacksize = stacksize;  //ignore argument stacksize,avoid compiler warnning
  j = 0xffff;
  for (i = 0; i < MAX_LWIP_THREADS; i++) {
    if (thread_mount[i] == (0x0100 | prio)) {
      LWIP_ASSERT("sys_thread_new: prio exist", 0);
      return (sys_thread_t)prio;
    } else if (thread_mount[i] == 0 && j == 0xffff) {
      j = i;
    }
  }
  if (j != 0xffff) {
    err = OSTaskCreate(thread, arg, &thread_stk[i][THREAD_STK_SIZE-1], prio);
    LWIP_ASSERT("sys_thread_new: task create", err == OS_ERR_NONE);
    if (err == OS_ERR_NONE) {
      thread_mount[i] = 0x0100 | prio;
      return (sys_thread_t)prio;
    }
  }
  LWIP_ASSERT("sys_thread_new: stack pool full", 0);
  return (sys_thread_t)prio;
}


