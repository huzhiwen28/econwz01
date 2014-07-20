#include <ucos_ii.h>
#include "app_usbmodbusmaster.h"
#include "bsp_usb.h"
#include "app_usbcom.h"

struct _USB_ModbusMaster USB_ModbusMaster;

//初始化
void USB_ModbusMaster_Init(void)
{
	u8 err;
	USB_ModbusMaster.msg = OSSemCreate(0);
	USB_ModbusMaster.Mutex = OSMutexCreate(2,&err);
}

//同步写
//addr 目标起始位置 cnt寄存器数量 array寄存器内容 返回结果
u8 USB_ModbusMaster_Write(unsigned char addr,unsigned char cnt,short *array)
{
	int i;

	u8 data[1000];
	u8 err;
	//独占设备，防止捣乱
 	OSMutexPend(USB_ModbusMaster.Mutex,0,&err);

	//组帧
	//功能码
	data[0] = 0x02;
	//起始地址
	data[1] = (char)addr;
	//寄存器数量
	data[2] = (char)cnt;

	//继承器内容
	for(i = 0; i < cnt;i++)
	{
		//short的高8位
		data[3+i*2] = (char)((array[i]>>8)&0xff);

		//short的低8位
		data[3+i*2+1] = (char)((array[i])&0xff);
	}

	//LRC位
	data[cnt*2+3]=LRC(data,cnt*2+3);

 	USB_ModbusMaster.retval = 0;
	
	//写数据
	USBCOM_Write(data,4+cnt*2);

	//等待通知,2000ms超时
   	OSSemPend(USB_ModbusMaster.msg,OS_TICKS_PER_SEC*2,&err);

	if(err == OS_TIMEOUT)
	{
		//释放设备
		OSMutexPost(USB_ModbusMaster.Mutex);
		return 0;
	}
	else
	{
		if(USB_ModbusMaster.retval == 1)
		{
			//释放设备
			OSMutexPost(USB_ModbusMaster.Mutex);
			return 1;
		}
		else
		{
			//释放设备
			OSMutexPost(USB_ModbusMaster.Mutex);
			return 0;
		}
	}
}


//通知modbus master收到数据
void USB_ModbusMaster_ProcessFrame(u8* chardata,int len)
{
	//长度大于0，否则等应答超时
	if(len >0 )
    {
	  	//处理应答
	  	//LRC校验
	  	if( chardata[len-1] == LRC(chardata,len-1) )
	  	{
			//功能码正确
			if(chardata[0] == 0x02)
			{
				USB_ModbusMaster.retval = 1;
			}
      	}
		//发通知
   		OSSemPost(USB_ModbusMaster.msg);
    }
}

