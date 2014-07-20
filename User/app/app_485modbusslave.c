#include <ucos_ii.h>
#include "app_485modbusslave.h"
#include "bsp.h"
#include "app_485com.h"
#include "app_kernelregs.h"
#include "app_backend.h"
#include "app_usbcom.h"

//通知modbus slave收到数据
void C485_ModbusSlave_ProcessFrame(u8* chardata,int len)
{
	//帧正确与否标识
	char Flag = 0;

	//如果长度为0，不处理
	if(len >5 )
	{
		u8 SendData[240];

		//处理应答
		//CRC校验
		u16 crcresult = CRC16(chardata,(u16)len -2);
		if (crcresult == (chardata[len-2]<<8 | chardata[len-1] ))
		{
			//下位机发送的功能码匹配
			//写reg
			if(chardata[1] == 0x06)
			{
				//处理数据，写寄存器
				//RecvData[]:0 id 1 cmd 2-3 寄存器地址 4-5 寄存器数据
				{
					short sd = 0;
					sd = chardata[4];
					sd = sd << 8;//高8位
					sd = sd + chardata[5];//低8位

					//修改标识位
					if(sd != KernelRegs[chardata[3]] 
					&& ((chardata[3]) < 80) )
					{
					  //79为控制位，无需考虑掉电保存
					  if(((chardata[3]) != 79))
					  {
					    RegsFlag[chardata[3]] = 1;
					  }
					  KernelRegs[chardata[3]] = sd;
					}
				}
				//应答
	
			}
			//写regs
			else if(chardata[1] == 0x10)
			{
			    int i = 0;
	
				//处理数据，写寄存器
				//RecvData[]:0 id 1 cmd 2-3 寄存器地址 4-5 寄存器数量 6数据字节数 7-结尾 数据
				for(i = 0;i < chardata[5];i++)
				{
					short sd = 0;
					sd = chardata[7+i*2];
					sd = sd << 8;//高8位
					sd = sd + chardata[7+i*2 + 1];//低8位

					//修改标识位
					if(sd != KernelRegs[chardata[3]+i] 
					&& ((chardata[3]+i) < 80) )
					{
					  //79为控制位，无需考虑掉电保存
					  if(((chardata[3]+i) != 79))
					  {
					    RegsFlag[chardata[3]+i] = 1;
					  }
					  KernelRegs[chardata[3]+i] = sd;
					}
				}
				//应答
	
			}
			//读
			else if(chardata[1] == 0x03)
			{
			  //do nothing
			}
			else
			{
	
			}
			Flag = 1;
		}


		//应答PLC
		if(Flag == 1)
		{
			//写应答
		    if(chardata[1] == 0x06)
			{
			  u16 crc = 0;
			  //modbus组帧，真确包应答方式
			  SendData[0] = MODBUSID; //id
			  SendData[1] = 0x06; //命令
			  SendData[2] = chardata[2];//起始地址
			  SendData[3] = chardata[3];//起始地址
			  SendData[4] = chardata[4];//寄存器值
			  SendData[5] = chardata[5];//寄存器值

			  crc = CRC16(SendData,6);
			  SendData[6] = crc>>8; //LRC
			  SendData[7] = crc; //LRC

			  //写数据
			  C485COM_Write(SendData,8);

			}			//写应答
		    else if(chardata[1] == 0x10)
			{
			  u16 crc = 0;
			  //modbus组帧，真确包应答方式
			  SendData[0] = MODBUSID; //id
			  SendData[1] = 0x10; //命令
			  SendData[2] = chardata[2];//起始地址
			  SendData[3] = chardata[3];//起始地址
			  SendData[4] = chardata[4];//寄存器数量
			  SendData[5] = chardata[5];//寄存器数量

			  crc = CRC16(SendData,6);
			  SendData[6] = crc>>8; //LRC
			  SendData[7] = crc; //LRC

			  //写数据
			  C485COM_Write(SendData,8);

			}//读应答
			else if(chardata[1] == 0x03)
			{
     		  int i,j;
 			  u16 crc = 0;

			  //modbus组帧，正确包应答方式
			  SendData[0] = MODBUSID; //id
			  SendData[1] = 0x03; //命令
			  SendData[2] = chardata[5]*2;//应答字节数

	          //寄存器内容
	          for(i = 0,j = chardata[3]; i < chardata[5];i++)
	          {
		        //short的高8位
		        SendData[3+i*2] = (char)((KernelRegs[i+j]>>8)&0xff);

		        //short的低8位
		        SendData[3+i*2+1] = (char)((KernelRegs[i+j])&0xff);
	          }

	          //CRC位
 			  crc = CRC16(SendData,chardata[5]*2+3);
			  SendData[chardata[5]*2+3] = crc>>8; //LRC
			  SendData[chardata[5]*2+4] = crc; //LRC

			  //写数据
			  C485COM_Write(SendData,chardata[5]*2 + 5);
			}
		}
		else
		{
		  	//u8 msg[20] = "LRCERR\n";
		  	//BSP_USBWrite(msg,7);
			//modbus组帧，故障包应答方式
 			u16 crc = 0;
			SendData[0] = MODBUSID;//id
			SendData[1] = chardata[1] | 0x80; //功能码
			SendData[2] = 0x01;//错误

          	//CRC位
		  	crc = CRC16(SendData,3);
		  	SendData[3] = crc>>8; //LRC
		  	SendData[4] = crc; //LRC

			//写数据
			C485COM_Write(SendData,5);
		}
	}
}




