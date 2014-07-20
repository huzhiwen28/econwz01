#ifndef __APP_USBGATHER_H
#define __APP_USBGATHER_H
#include "stm32f10x.h"

	//初始化采集任务
	//num ,采集的单位数量
	//regs,采集的单位编号数组
	//mstime,采集时间间隔，毫秒
	void USB_Gather_Init(u8 num,u8* regs,int mstime);

	//打开设备
	void USB_Gather_Open(void);

	//关闭设备
	void USB_Gather_Close(void);

	//任务执行
	void USB_Gather_Run(void *p_arg);


//Gather
struct _USB_Gather
{
	//采集的单位数量
	u8 GatherNum;

	//采集的单位数组，数组中为寄存器编号
	u8 GatherRegs[10];

	//采集时间间隔，毫秒
	int GatherTimeMs;

	//是否打开 0未打开 1打开
	char OpenFlag;
};


#endif

