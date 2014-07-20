#include <ucos_ii.h>
#include "app_usbmodbusslave.h"
#include "bsp_usb.h"
#include "app_usbcom.h"
#include "app_kernelregs.h"
#include "app_backend.h"

//modbus slave处理数据
void USB_ModbusSlave_ProcessFrame(u8* chardata,int len)
{
	//帧正确与否标识
	char Flag = 0;

	//如果长度为0，不处理
	if(len >0 )
	{
		u8 SendData[1024];

		//处理应答
		//LRC校验，帧是否正确
		if( chardata[len-1] == LRC(chardata,len-1) )
		{
			//功能码正确，下位机发送的功能码
			if(chardata[0] == 0x01)
			{
			    int i = 0;
				//处理数据，写寄存器
				for(i = 0;i < chardata[2];i++)
				{
					short sd = chardata[3+i*2];
					sd = sd << 8;//高8位
					sd = sd + chardata[3+i*2 + 1];//低8位
					
					//修改标识位
					if(sd != KernelRegs[chardata[1]+i] 
					&& ((chardata[1]+i) < 80) )
					{
					  RegsFlag[chardata[1]+i] = 1;
					  KernelRegs[chardata[1]+i] = sd;
					}
					else
					{
					  KernelRegs[chardata[1]+i] = sd;
					}
				}
			}
			else
			{
			}
			Flag = 1;
		}
		else
		{
		}

		//应答PC机
		if(Flag == 1)
		{
			//modbus组帧，真确包应答方式
			SendData[0] = 0x01;
			SendData[1] = chardata[1];
			SendData[2] = chardata[2];
			SendData[3] = LRC(SendData,3);
			//写数据
			USBCOM_Write(SendData,4);
		}
		else
		{
			//modbus组帧，故障包应答方式
			SendData[0] = 0x81;
			SendData[1] = 0x01;
			SendData[3] = LRC(SendData,2);
			//写数据
			USBCOM_Write(SendData,3);
		}
		//应用处理
   		App_Backend_NewFrame();
	}
}




