#ifndef __APP_USBMODBUSMASTER_H
#define __APP_USBMODBUSMASTER_H
#include "stm32f10x.h"

//初始化
void USB_ModbusMaster_Init(void);

//通知modbus master处理数据帧
void USB_ModbusMaster_ProcessFrame(u8* chardata,int len);

//异步写
//addr 目标起始位置 cnt寄存器数量 array寄存器内容
u8 USB_ModbusMaster_Write(unsigned char addr,unsigned char cnt,short *array);

//ModbusMaster
struct _USB_ModbusMaster
{
 	//写数据结果 0失败 1成功
	char retval;

	OS_EVENT *msg; //消息处理通知
	OS_EVENT *Mutex; //独占锁
};

#endif
