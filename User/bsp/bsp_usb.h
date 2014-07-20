#ifndef __USB_H
#define __USB_H 

/* Includes ------------------------------------------------------------------*/	   
#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_fifo.h"

/*usb设备*/
struct _usb{
      struct _fifo ReadFIFO;
	  volatile u8 active;
      OS_EVENT *WriteMutex;//写锁
      OS_EVENT *ReadMutex; //读锁
};

/*USB串口初始化*/
void BSP_USBInit(void);

/*USB串口打开*/
void BSP_USBOpen(void);

/*USB串口关闭*/
void BSP_USBClose(void);

/*USB串口读*/
u16 BSP_USBRead(u8* buff,u16 len);

/*USB串口读缓冲区中长度*/
u16 BSP_USBInBuffLen(void);

/*USB串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_USBWrite(u8* buff,u16 len);

/*USB驱动通知消费者新的字符收到*/
void BSP_USBNewRecv(void);


#endif


