#include <ucos_ii.h>
#include "app_backend.h"
#include "app_kernelregs.h"
#include "app_usbgather.h"
#include "app_usbmodbusmaster.h"
#include "bsp_usb.h"

//后台模块
struct _App_Backend App_Backend;

//应用后台初始化
void App_Backend_Init(void)
{
  	App_Backend.PCOnline = 0;
  	App_Backend.RespFailCnt = 0;
  	App_Backend.GatherFlag = 0;
	App_Backend.FlushParamsg = OSSemCreate(0);
	
  	KernelRegs[79] = 0;
}

//参数刷新任务
void App_FlushParaTask_Run(void *p_arg)
{
	while(1)
	{
		u8 err;

		//等待通知
   		OSSemPend(App_Backend.FlushParamsg,0,&err);
	   if((App_Backend.PCOnline == 1))
	   {
	   		//发送数据,失败重试
			if(USB_ModbusMaster_Write(0,111,KernelRegs) == 0)
			{
				//等待-重试
				OSTimeDlyHMSM(0, 0, 0, 100);
				USB_ModbusMaster_Write(0,111,KernelRegs);	
			}
		}
	}
}


//心跳任务
void App_HeartbeatTask_Run(void *p_arg)
{
	while(1)
	{
	   if((App_Backend.PCOnline == 1))
	   {
	   		if(USB_ModbusMaster_Write(80,20,&KernelRegs[80]))
			{
				App_Backend.RespFailCnt = 0;
			}
			else
			{
				App_Backend.RespFailCnt ++;

				//5次都失败则认为PC掉线
				if(App_Backend.RespFailCnt >= 5)
				{
					u8 err;

				    App_Backend.PCOnline = 0;
	
					//停止采集
					OSMutexPend(RegsWriteMutex,0,&err);
			        KernelRegs[79] = KernelRegs[79] & ~(0x0001<<1);
					OSMutexPost(RegsWriteMutex);
			        App_Backend.GatherFlag = 0;
					USB_Gather_Close();
			  	 }
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 1000);

	}
}

void App_Backend_NewFrame(void)
{
    //第一次PC机请求
    if(App_Backend.PCOnline == 0)
	{
	    //发送后才算是上线
	    App_Backend.PCOnline = 1;
        App_Backend.RespFailCnt = 0;
		//发通知
   		OSSemPost(App_Backend.FlushParamsg);
	}

	//1采集
    if( App_Backend.GatherFlag != (char)((KernelRegs[79]>>1) & 0x0001))
	{
		App_Backend.GatherFlag = (char)((KernelRegs[79]>>1) & 0x0001);
		//采集处理代码
		if(App_Backend.GatherFlag == 0x01)
		{
		  //采样通道收集
		  unsigned char channelcnt = 0;
		  unsigned char channelarray[10];
		  int i = 0;

		  for(i = 0;i < 10;i++)
		  {
		    if(KernelRegs[i+100] != 0)
			{
			    channelarray[i] = (u8)KernelRegs[i+100];
				channelcnt++;
			}
			else
			{
			    break;
			}
		  }
		  if(channelcnt > 0)
		  {
		    if(KernelRegs[110] > 0)
			{
		      USB_Gather_Init(channelcnt,channelarray,KernelRegs[110]);
			  USB_Gather_Open();
			}
			else
			{
		      USB_Gather_Init(channelcnt,channelarray,1000);
			  USB_Gather_Open();
			}
		  }
		}
		else
		{
  			USB_Gather_Close();
		}
	}

	//2刷参数
    if( (char)((KernelRegs[79]>>2) & 0x0001) == 0x01)
	{
		u8 err;
		//发通知
   		OSSemPost(App_Backend.FlushParamsg);

		//消费完毕清理
		OSMutexPend(RegsWriteMutex,0,&err);
		KernelRegs[79] = KernelRegs[79] & ~(0x0001<<2);
		OSMutexPost(RegsWriteMutex);
	}

	//3停止和PC联系
    if( (char)((KernelRegs[79]>>3) & 0x0001) == 0x01)
	{
		u8 err;
        App_Backend.PCOnline = 0;
		//消费完毕清理
		OSMutexPend(RegsWriteMutex,0,&err);
		KernelRegs[79] = KernelRegs[79] & ~(0x0001<<3);
		OSMutexPost(RegsWriteMutex);
	}
}


