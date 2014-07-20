#include <ucos_ii.h>
#include "app_usbcom.h"
#include "bsp_usb.h"
#include "app_usbmodbusmaster.h"
#include "app_usbmodbusslave.h"
#include "app_usbgather.h"



#define ControlVersion  2
struct _USBCOM_Backend 	USBCOM_Backend;
u16 HaveheadtimeMS = 0;

//汇报控制器信息给上位机
void  App_ReportControlInfo(u8* chardata,int len)
{
	//帧正确与否标识
	char Flag = 0;

	//如果长度为0，不处理
	if(len >0 )
	{
		u8 SendData[30];

		//处理应答
		//LRC校验，帧是否正确
		if( chardata[len-1] == LRC(chardata,len-1) )
		{
			//功能码正确，下位机发送的功能码
			if(chardata[0] == 0x73)
			{
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
			//modbus组帧，正确包应答方式
			SendData[0] = 0x73;

			//读控制器信息，含ProductID,ComID,CPUUsage,ControlVersion

			//ProductID
			//SendData[1] = (unsigned char)(App_Encryption.Device_Serial2 >> 24);
			//SendData[2] = (unsigned char)(App_Encryption.Device_Serial2 >> 16);
			//SendData[3] = (unsigned char)(App_Encryption.Device_Serial2 >> 8);
			//SendData[4] = (unsigned char)(App_Encryption.Device_Serial2 >> 0);

			//ComID
			SendData[5] = (unsigned char)(MODBUSID >> 8);
			SendData[6] = (unsigned char)(MODBUSID >> 0);
			
			//CPUUsage%
			SendData[7] = 0;
			SendData[8] = OSCPUUsage;

			//ControlVersion
			SendData[9] = (unsigned char)(ControlVersion >> 8);
			SendData[10] = (unsigned char)(ControlVersion >> 0);
			
			SendData[11] = LRC(SendData,11);

			//写数据
			USBCOM_Write(SendData,12);
		}
		else
		{
			//modbus组帧，故障包应答方式
			SendData[0] = 0x73|0x80;
			SendData[1] = LRC(SendData,1);
			//写数据
			USBCOM_Write(SendData,2);
		}
	}
}

void printint(int num)
{
	  u8 datastring[10]="";
	  u8 zerodata = '0';
	  u8 chardata = 0;
	  u8 tendata = 0;
	  int mynum = num;

		tendata = (u8)(mynum/1000000);
		chardata = tendata+zerodata;
		datastring[0] = chardata;
		mynum = mynum%1000000;

		tendata = (u8)(mynum/100000);
		chardata = tendata+zerodata;
		datastring[1] = chardata;
		mynum = mynum%100000;

		tendata = (u8)(mynum/10000);
		chardata = tendata+zerodata;
		datastring[2] = chardata;
		mynum = mynum%10000;

		tendata = (u8)(mynum/1000);
		chardata = tendata+zerodata;
		datastring[3] = chardata;
		mynum = mynum%1000;

		tendata = (u8)(mynum/100);
		chardata = tendata+zerodata;
		datastring[4] = chardata;
		mynum = mynum%100;

		tendata = (u8)(mynum/10);
		chardata = tendata+zerodata;
		datastring[5] = chardata;
		mynum = mynum%10;

		chardata = mynum+zerodata;
		datastring[6] = chardata;
		
		datastring[7] = ' ';
		//BSP_232CWrite(datastring,8);
		BSP_USBWrite(datastring,8);

}
/*
ascii码转换为char，例如"43" -> 0x43
charbuff 存放结果
ascii 待转换的ascii
len 待转换的长度，为偶数
*/
void ascii2char(u8* charbuff,u8* asciibuff,int len)
{
	int j,jj;//j 对应ascii jj对应char数组

	//ascii -> char
	for (j = 0,jj = 0; j < len-1; j += 2,++jj)
	{
		//高位ascii
		//字母
		if (asciibuff[j] <= 70 && asciibuff[j] >= 65)
		{
			charbuff[jj] = ((asciibuff[j] - 55) << 4) & 0xf0;
		}//数字
		else if (asciibuff[j] <= 57 && asciibuff[j] >= 48)
		{
			charbuff[jj] = ((asciibuff[j] - 48) << 4)& 0xf0;
		}

		//低位ascii
		//字母
		if (asciibuff[j+1] <= 70 && asciibuff[j+1] >= 65)
		{
			charbuff[jj] += (asciibuff[j+1] - 55) & 0x0f;
		}//数字
		else if (asciibuff[j+1] <= 57 && asciibuff[j+1] >= 48)
		{
			charbuff[jj] += (asciibuff[j+1] - 48) & 0x0f;
		}
	}
}

/*
char转换为ascii码，例如0x43 -> "43"
ascii 转换的结果ascii
charbuff 待转换
len 待转换的长度
*/
void chartoascii(u8* asciibuff,u8* charbuff,int len)
{
	int j,jj;//j 对应char, jj对应ascii数组

	//数据
	for (j = 0,jj = 0; j< len; ++j,jj += 2)
	{
	   //高位 
	   if(((charbuff[j]>>4) >= 0) && ((charbuff[j]>>4) <= 9))
	   {
	     *(asciibuff+jj) = (charbuff[j]>>4) + 48;
	   }
	   else if(((charbuff[j]>>4) >= 0x0a) && ((charbuff[j]>>4) <= 0x0f))
	   {
	     *(asciibuff+jj) = (charbuff[j]>>4) + 55;
	   }

	   //低位
	   if(((charbuff[j] & 0x0f) >= 0) && ((charbuff[j]& 0x0f) <= 9))
	   {
	     *(asciibuff+jj+1) = (charbuff[j] & 0x0f) + 48;
	   }
	   else if(((charbuff[j]& 0x0f) >= 0x0a) && ((charbuff[j]& 0x0f) <= 0x0f))
	   {
	     *(asciibuff+jj+1) = (charbuff[j] & 0x0f) + 55;
	   }
	}
	return;
}

/*
LRC校验值计算
charbuff 待计算的char数组
len 待计算的char数组长度
返回LRC计算结果
*/

char LRC(u8 *charbuff,int len)
{
	char retval = 0;
	int i = 0;
	for(i =0;i < len;i++)
	{
		retval += charbuff[i];
	}
	return retval;
}

/*
com初始化
*/
void USBCOM_Init(void)
{
	//状态初始化
	USBCOM_Backend.framestate = 0;
	USBCOM_Backend.buffend = 0;
	USBCOM_Backend.frameheadindex = -1;
	USBCOM_Backend.Newmsg = OSSemCreate(0);
}

//打开设备
void USBCOM_Open(void)
{
    USBCOM_Backend.comopen = 1;
}

//关闭设备
void USBCOM_Close(void)
{
	if (USBCOM_Backend.comopen == 1)
	{
		USBCOM_Backend.comopen = 0;
	}
}

//任务执行
void USBCOM_Run(void *p_arg)
{


    USBCOM_Init();
	USBCOM_Open();
    while(1)
	{
		u8 err;

		//已经有头，则设时间等待，否则无限期等待
		if(USBCOM_Backend.framestate == 1)
		{
			//等待通知,100ms超时
   			OSSemPend(USBCOM_Backend.Newmsg,OS_TICKS_PER_SEC/10,&err);
		}
		else
		{
			//等待通知无超时
   			OSSemPend(USBCOM_Backend.Newmsg,0,&err);
		}
		
		//只要串口打开，则后台运行
		if(USBCOM_Backend.comopen)
		{
			//待扫描起始
			int scanbegin = 0;
	
			//待扫描最后字节后一个位置
			int scanend = 0;
	
			int cnt = 0;
			int index = 0;
	
			//如果帧头超时则要清理
			if(USBCOM_Backend.framestate == 1)
			{
			   if(err == OS_TIMEOUT)
			   {
			     USBCOM_Backend.framestate = 0;
		        //最后一个字节后位置修改
		        USBCOM_Backend.buffend = 0;
	
		        //帧头为-1
		        USBCOM_Backend.frameheadindex = -1;
			   }
			}
	
			if((cnt = BSP_USBInBuffLen()) > 0 )
			{
			    
				scanbegin = USBCOM_Backend.buffend;
	
				//数据过长，只取一共1024个数据(含原来的数据)
				if(cnt+USBCOM_Backend.buffend > 1024)
				{
					//实际读的数据长度，否则溢出
					BSP_USBRead(USBCOM_Backend.buff + scanbegin,(1024-USBCOM_Backend.buffend));
					scanend = 1024;
				}
				else
				{
					//没有溢出，全部读
					BSP_USBRead(USBCOM_Backend.buff + scanbegin,cnt);
					scanend = cnt + USBCOM_Backend.buffend;
				}
	
				//最后的字节后的位置改变
				USBCOM_Backend.buffend = scanend;
	
				//遍历收到的数据
				for(index = scanbegin;index < scanend;index++)
				{
					//没有头
					if(USBCOM_Backend.framestate == 0)
					{
						//头
						if(USBCOM_Backend.buff[index] == ':')
						{
							USBCOM_Backend.framestate = 1;
							USBCOM_Backend.frameheadindex = index;
						}
					}
	
					//有头
					else if(USBCOM_Backend.framestate == 1)
					{
						//检测到结束字符
						if(USBCOM_Backend.buff[index] == '\n')
						{
							//存放转换结果
							u8 chardata[512];
	
							//转换结果长度
							int charlen = 0;
	 
							//ascii转换为char
							ascii2char(chardata,USBCOM_Backend.buff + USBCOM_Backend.frameheadindex+1 ,index - USBCOM_Backend.frameheadindex-1);
							charlen = (index - USBCOM_Backend.frameheadindex-1)/2;
	
							//打印收到的字符
							USBCOM_Backend.buff[index] = 0;
							//printf("收到的字符：%s",buff + frameheadindex+1);
	
							//根据功能码发送给不同的任务处理
							switch(chardata[0])
							{
								//PC写控制器数据
							case 0x01:
								USB_ModbusSlave_ProcessFrame(chardata,charlen);
								break;
	
								//控制器写PC数据
							case 0x02:
							case 0x82:
								USB_ModbusMaster_ProcessFrame(chardata,charlen);
								break;

								//读CPUID
							case 0x71:
								//App_Encryption_ReportID(chardata,charlen);
								break;

								//写加密串
							case 0x72:
								//App_Encryption_WriteEncry(chardata,charlen);
								break;

								//读控制器信息，含ProductID,ComID,CPUUsage,ControlVersion
							case 0x73:
								App_ReportControlInfo(chardata,charlen);
								break;

	
							default:
								;
							}
							
							//处理完毕状态改变
							USBCOM_Backend.framestate = 0;
						}
						else
						{
							//如果再检测到头
							if(USBCOM_Backend.buff[index] == ':')
							{
								//抛弃原来的头
								USBCOM_Backend.frameheadindex = index;
							}
						}
					}
				}
	
				//遍历完毕并找到头状态下面，数组要移位，使数组中字节都是不完整的下一个帧的数据
				if(USBCOM_Backend.framestate == 1)
				{
				    int i = 0;
					//有效数据往前移动
					for(i = 0;i < scanend - USBCOM_Backend.frameheadindex;i++ )
					{
						*(USBCOM_Backend.buff+i) = *(USBCOM_Backend.buff + USBCOM_Backend.frameheadindex+i);
					}
	
					//最后一个字节后位置修改
					USBCOM_Backend.buffend = scanend - USBCOM_Backend.frameheadindex;
	
					//帧头为0
					USBCOM_Backend.frameheadindex = 0;
				}
				else//遍历完毕，而且处于没有找到头状态下面，认为所有数据都消耗完毕
				{
					//最后一个字节后位置修改
					USBCOM_Backend.buffend = 0;
	
					//帧头为-1
					USBCOM_Backend.frameheadindex = -1;
	
				}
				//printint(USBCOM_Backend.buffend);
			}
		}
	}
}


//写字符
int USBCOM_Write(u8* chardata,int len)
{
	if(USBCOM_Backend.comopen)
	{
		u8 asciibuff[1024];

		//帧头
		asciibuff[0]= ':';

		//帧数据转换为ascii
		chartoascii(asciibuff+1,chardata,len);

		//帧尾
		asciibuff[len*2+1]= '\n';

		//便于打印
		asciibuff[len*2+2]= 0;

		BSP_USBWrite(asciibuff, len*2+2);
	}
	return len;
}

//通知任务有新消息来到
void USBCOM_Newmsg()
{
	if(USBCOM_Backend.comopen)
	{
	    OSSemPost(USBCOM_Backend.Newmsg);		//发送通知
	}
}

