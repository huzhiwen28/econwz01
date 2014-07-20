#include <ucos_ii.h>
#include "app_usbgather.h"
#include "bsp_usb.h"
//#include <stdlib.h>

#include "app_usbcom.h"
#include "app_kernelregs.h"

struct _USB_Gather USB_Gather_Task;

void USB_Gather_Write()
{
	int i;
	//待发送的采集数据
	u8 data[50];
	u8 cnt;
	//malloc(3);

	//组帧
	//功能码
	data[0] = 0x03;

	//采集寄存器数量
	data[1] = (u8)USB_Gather_Task.GatherNum;
	cnt = (u8)USB_Gather_Task.GatherNum;

	//继承器内容
	for(i = 0; i < cnt;i++)
	{
		//short的高8位
		data[2+i*2] = (u8)((KernelRegs[USB_Gather_Task.GatherRegs[i]]>>8)&0xff);

		//short的低8位
		data[2+i*2+1] = (u8)((KernelRegs[USB_Gather_Task.GatherRegs[i]])&0xff);
	}

	//LRC位
	data[cnt*2+2]=LRC(data,cnt*2+2);

    //写数据
    USBCOM_Write(data,3+cnt*2);
}

	//初始化采集任务
	//num ,采集的单位数量
	//regs,采集的单位编号数组
	//mstime,采集时间间隔，毫秒
void USB_Gather_Init(u8 num,u8* regs,int gamstime)
{
  int i = 0;
  for(i = 0;i < num;i++)
  {
     USB_Gather_Task.GatherRegs[i] = regs[i];
  }
  USB_Gather_Task.GatherNum = num;
  USB_Gather_Task.GatherTimeMs = gamstime;
  USB_Gather_Task.OpenFlag = 0;
}

//打开设备
void USB_Gather_Open(void)
{
	if(USB_Gather_Task.OpenFlag == 0)
	{
		USB_Gather_Task.OpenFlag = 1;
	}
}

//关闭设备
void USB_Gather_Close(void)
{
	if(USB_Gather_Task.OpenFlag == 1)
	{
		//任务停止
		USB_Gather_Task.OpenFlag = 0;
	}
}

//任务执行
void USB_Gather_Run(void *p_arg)
{
	while(1)
	{
		//后台处理的
		if(USB_Gather_Task.OpenFlag == 1 )
		{
	    	USB_Gather_Write();
			OSTimeDlyHMSM(0, 0, 0, USB_Gather_Task.GatherTimeMs);
		}
		OSTimeDlyHMSM(0, 0, 0, 1);
	}
}
