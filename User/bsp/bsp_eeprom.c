#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_eeprom.h"


#define ADDR_24LC08		0xA0    //器件地址
#define I2C_PAGESIZE	4		/* 24C01/01A页缓冲是4个 */

/* Private define ------------------------------------------------------------*/
#define SCL_H         GPIO_SetBits(GPIOB , GPIO_Pin_6)
#define SCL_L         GPIO_ResetBits(GPIOB , GPIO_Pin_6)
   
#define SDA_H         GPIO_SetBits(GPIOB , GPIO_Pin_7)
#define SDA_L         GPIO_ResetBits(GPIOB , GPIO_Pin_7)

#define SDA_read      GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_7)
#define  uint   u16;

struct _EEPROM EEPROMDEVICE;

/*******************************************************************************
* Function Name  : I2C_Configuration
* Description    : EEPROM管脚配置
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void I2C_Configuration(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure; 

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);

   /* Configure I2C1 pins: PB6->SCL and PB7->SDA */
   GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	//开漏输出
   GPIO_Init(GPIOB, &GPIO_InitStructure);
		
}

void SDA_R_Set(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure; 

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);

   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;			  //输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	/* PB7 */	
}

void SDA_W_Set(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure; 

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);

   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	//开漏输出
   GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/*EEPROM初始化*/
void BSP_EEPROMInit(void)
{
  u8 err = 0;
  I2C_Configuration();
  EEPROMDEVICE.active = 0;
  SCL_H ;
  SDA_H; 
  EEPROMDEVICE.IICMutex = OSMutexCreate(9,&err);
}

/*EEPROM打开*/
void BSP_EEPROMOpen(void)
{
  EEPROMDEVICE.active = 1;
}

/*EEPROM关闭*/
void BSP_EEPROMClose(void)
{
  EEPROMDEVICE.active = 0;
}

void I2C_delay(void)
{
	//OSTimeDly(1);	
   	uint8_t i=60;     
   	while(i) 
   	{ 
    	i--; 
   	} 

}


u8 I2C_Start(void)	 //I2C开始位
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)
	  return 0;	                    //SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) 
	  return 0;	                    //SDA线为高电平则总线出错,退出
	SDA_L;							//SCL为高电平时，SDA的下降沿表示停止位
	I2C_delay();
	return 1;
}


void I2C_Stop(void)			   //I2C停止位
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;					   //SCL为高电平时，SDA的上升沿表示停止位
	I2C_delay();
}


static void I2C_Ack(void)		//I2C响应位
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}


static void I2C_NoAck(void)		//I2C非响应位
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}


u8 I2C_WaitAck(void) 	  //I2C等待应答位
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
      return 0;
	}
	SCL_L;
	return 1;
}


 /*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : 数据从高位到低位
* Input          : - SendByte: 发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_SendByte(u8 SendByte) 
{
    u8 i;

    for(i = 0;i < 8; i++) 
    {
      SCL_L;
      I2C_delay();
      if(SendByte & 0x80)
        SDA_H;  			  //在SCL为低电平时，允许SDA数据改变
      else 
        SDA_L;   
      SendByte <<= 1;
      I2C_delay();
      SCL_H;
      I2C_delay();
    }
    SCL_L;
}


/*******************************************************************************
* Function Name  : I2C_ReceiveByte
* Description    : 数据从高位到低位
* Input          : None
* Output         : None
* Return         : I2C总线返回的数据
*******************************************************************************/
u8 I2C_ReceiveByte(void)  
{ 
    u8 i,ReceiveByte = 0;

    SDA_H;				
    for(i = 0;i < 8; i++) 
    {
      ReceiveByte <<= 1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)				   //在SCL为高电平时，SDA上的数据保持不变，可以读回来
      {
        ReceiveByte |= 0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
}


/*******************************************************************************
* Function Name  : BSP_EEPROMWriteByte
* Description    : 写一字节数据
* Input          : - SendByte: 待写入数据
*           	   - WriteAddress: 待写入地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功写入,=0失败
*******************************************************************************/           
u8 BSP_EEPROMWriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress)
{		
   INT8U  err;
   OSMutexPend(EEPROMDEVICE.IICMutex,0,&err);

    if(!I2C_Start())
	{
	  OSMutexPost(EEPROMDEVICE.IICMutex);
	  return 0;
	 }
    I2C_SendByte(((WriteAddress & 0x0700) >>7) | DeviceAddress & 0xFFFE); //设置高起始地址+器件地址

    if(!I2C_WaitAck())
	{
	  I2C_Stop(); 
	  OSMutexPost(EEPROMDEVICE.IICMutex);
	  return 0;
	}

    I2C_SendByte((u8)(WriteAddress & 0x00FF));   //设置低起始地址
    I2C_WaitAck();	
    I2C_SendByte(SendByte);
    I2C_WaitAck();
	   
    I2C_Stop(); 
 	
	OSTimeDlyHMSM(0, 0, 0, 10);//注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)

	OSMutexPost(EEPROMDEVICE.IICMutex);
    return 1;
}									 

/*******************************************************************************
* Function Name  : BSP_EEPROMReadByte
* Description    : 读取一串数据
* Input          : - pBuffer: 存放读出数据
*           	   - length: 待读出长度
*                  - ReadAddress: 待读出地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功读入,=0失败
*******************************************************************************/          
u8 BSP_EEPROMReadByte(u8* pBuffer,   u16 length,   u16 ReadAddress,  u8 DeviceAddress)
{		
   INT8U  err;
   OSMutexPend(EEPROMDEVICE.IICMutex,0,&err);
    if(!I2C_Start())
	{
	  OSMutexPost(EEPROMDEVICE.IICMutex);
	  return 0;
	}

    I2C_SendByte(((ReadAddress & 0x0700) >> 7) | DeviceAddress & 0xFFFE); //设置高起始地址+器件地址

    if(!I2C_WaitAck())
	{
	  I2C_Stop(); 
	  OSMutexPost(EEPROMDEVICE.IICMutex);
	  return 0;
	}

    I2C_SendByte((u8)(ReadAddress & 0x00FF));   //设置低起始地址   
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(((ReadAddress & 0x0700) >>7) | DeviceAddress | 0x0001);
    I2C_WaitAck();

    while(length)
    {
      *pBuffer = I2C_ReceiveByte();
      if(length == 1)
	    I2C_NoAck();
      else 
	    I2C_Ack(); 
      pBuffer++;
      length--;
    }

    I2C_Stop();

	OSMutexPost(EEPROMDEVICE.IICMutex);
    return 1;
}


/**********************************************************************************8  
邱毅
time:2009,04,13
从EEPROM中读写数据
*************************************************************************************/

//extern uint ReadDataEEPROM(uint Adss);
//extern char SendWordEEPROM(uint Adss,uint SData);

//extern bit SendDataEEPROM(uint Adss,uint Data)
/***************************************************************************************8
延时，控制IIC速率
**************************************************************************************/
void IICDelay()
{
	I2C_delay();
}

void IICStart()								/*启动I2C总线的函数，当SCL为高电平时使SDA产生一个负跳变*/ 
{
//	IICWP=0;
    IICDelay();
    IICDelay();
//	AD1CON1bits.ADON = 0;
//	IEC0bits.AD1IE=0;
	SDA_W_Set();		    //IO设置为写
	SDA_H;
    SCL_H;
	IICDelay();
  	IICDelay();									//
	SDA_L;									//
  	IICDelay();
    IICDelay();
    SCL_L;
  	IICDelay();
    IICDelay();

}

void IICStop(void)		 				/*终止I2C总线，当SCL为高电平时使SDA产生一个正跳变*/
{
	SDA_W_Set();		    //IO设置为写
     SCL_H;
	 IICDelay();
	 SDA_L;	
	 IICDelay();
     SCL_H;
     IICDelay();
	 SDA_H;	
     IICDelay();
//   SCL=0;								//结束时全部拉高
//		AD1CON1bits.ADON = 1;
     IICDelay();
	  IICDelay();
	 IICDelay();

//		IICWP=1;

     
} 

char Check_Acknowledge(void)	        /*发送完一个字节后检验设备的应答信号*/
{
	u16 Ack;
   // SDA=1;
	SDA_R_Set();		    //IO设置为读
    SCL_L;
	IICDelay();
    SCL_H;
   	IICDelay();
	Ack=SDA_read;
	IICDelay();
    SCL_L;
    IICDelay();
	SDA_W_Set();		    //IO设置为写
	SDA_H;
    if(Ack==1)
	{
          return(0);				//无检测到应答
	}
	else
	{
        return(1);					//检测到应答
    } 
}


/***********************************************************************************************
往24C04发送一个字节
输入参数：SData	所写数据 8bit	  
输出参数：返回值   0:发送失败   1:成功
***********************************************************************************************/
u16 IICSend(u16 IICData) 			/*向I2C总线写一个字节*/
{
       
     u16 i;
	u16 b;
//	TRISGbits.TRISG3=0;
     for(i=0;i<8;i++)
	{
   		 SCL_L;
    	IICDelay();
		b=(IICData<<i)&0x80;
    	 if(b==0x80)
		{
   		  	SDA_H;
		}
         else
		{
          	SDA_L;    
		}
		IICDelay();
		SCL_H;
    	IICDelay();
		IICDelay();
	}
	SCL_L;
//	SDA_H;
//	IICDelay();
	i=Check_Acknowledge();
	 return(i);				//0:发送失败   1:成功
}

/***********************************************************************************************
往24C04发送一个字
输入参数：SData	所写数据 8bit	  
输出参数：返回值   0:发送失败   1:成功
***********************************************************************************************/
u16 IICSendWord(u16 IICData) 			/*向I2C总线写一个字节*/
{
       
     u16 i;
	u16 b;
//	TRISGbits.TRISG3=0;
     for(i=0;i<16;i++)
	{
		SCL_L;
    	IICDelay();
		b=(IICData<<i)&0x8000;
    	 if(b==0x8000)
		{
   		  	SDA_H;
		}
         else
		{
          	SDA_L;    
		}
		IICDelay();
		SCL_H;
    	IICDelay();
		IICDelay();
	}
	SCL_L;
  	SDA_H;
	IICDelay();
	i=Check_Acknowledge();
	 return(i);				//0:发送失败   1:成功
}

void Ack()				//读数据时有应答
{
   	SDA_L; 
   	IICDelay();
	SCL_H;
	IICDelay();
	IICDelay();
	SCL_L;
    IICDelay();			
}

void NoAck()			//读数据时无应答
{
   	SDA_H; 
   	IICDelay();
	SCL_H;
	IICDelay();
	IICDelay();
	SCL_L;
   	SDA_L; 
    IICDelay();
}



u16 IICRead() 			/*向I2C总线读一个字节*/
{
       
     u16 i;
	u16 Data=0;
	u16 SDAIO;
	Data=0;
	SDA_R_Set();		    //IO设置为读
     for(i=0;i<8;i++)				//接位读数据
	{
		SCL_L;
    	IICDelay();
		SCL_H;
    	IICDelay();
		SDAIO=SDA_read;
    	if(SDAIO==1)
		{
   		  	Data=Data+(0x80>>i);
		}
		IICDelay();
		SCL_L;
		IICDelay();
	}
	SDA_W_Set();		    //IO设置为读
	return(Data);
}


/***********************************************************************************************
往24C04写一个字节
输入参数：Adss,所写地址
		  SData	所写数据 8bit
输出参数：无 
***********************************************************************************************/
u16 SendDataEEPROM(u16 Adss,u16 SData)
{
   INT8U  err;
   OSMutexPend(EEPROMDEVICE.IICMutex,0,&err);
	IICStart();

	if(IICSend(0xa0))			// 24C04的写命令
	{
		if(IICSend(Adss))		//写地址
			{
				if(IICSend(SData))			//写数据
				{
					IICStop();
				  OSMutexPost(EEPROMDEVICE.IICMutex);
					return(1);				//成功
				}
				else
				{
				  OSMutexPost(EEPROMDEVICE.IICMutex);
					IICStop();
					return(0);

				}
			}
			else
			{
				IICStop();
			  OSMutexPost(EEPROMDEVICE.IICMutex);
				return(0);
			}	
		}
	else
	{
		IICStop();
	  OSMutexPost(EEPROMDEVICE.IICMutex);
		return(0);
	}
  OSMutexPost(EEPROMDEVICE.IICMutex);
}


/***********************************************************************************************
往24C04写一个字
输入参数：Adss,所写地址
		  SData	所写数据 16bit 高位在前
输出参数：无 
***********************************************************************************************/
u8 SendWordEEPROM(u16 Adss,u16 SData)
{	

 	u16 DataH;
	u16 DataL;
	DataH=SData>>8;
  	DataL=SData&0x00ff;
	while(!BSP_EEPROMWriteByte(DataH,Adss, 0xA0))
	OSTimeDlyHMSM(0, 0, 0, 1);

	while(!BSP_EEPROMWriteByte(DataL, Adss+1, 0xA0))
	OSTimeDlyHMSM(0, 0, 0, 1);
	return(1);
}


/***********************************************************************************************
从一24C04读一个字
输入参数：Adss,所读地址
输出参数：所读数据返回值 
***********************************************************************************************/
u16 ReadWordEEPROM(u16 Adss)
{
//	StopINT();
   INT8U  err;
	u16 ReadData;

	u16 ADDH;
	u16 ADDH2;
	u16 ADDL;
   OSMutexPend(EEPROMDEVICE.IICMutex,0,&err);
	if(Adss>0xff)
	{
		ADDH=0xa2;
		ADDH2=0xa3;
		ADDL=Adss&0x00ff;
	}
	else
	{
		ADDH=0xa0;
		ADDH2=0xa1;
		ADDL=Adss&0x00ff;
	}

	IICStart();

	if(IICSend(ADDH))					
	{
//		Ack();
		if(IICSend(ADDL))									//写地址
		{
//			Ack();
			IICStart();
			if(IICSend(ADDH2))
			{
//				Ack();
				ReadData=IICRead();							//读高位
				Ack();										//应答
				ReadData=ReadData<<8;									
				ReadData=IICRead()+ReadData;				//读底位
				NoAck();									//无应答
				IICStop();
			}
			else
			{
				ReadData=0xffff;
				IICStop();
			}
		}
		else
		{
			ReadData=0xffff;
			IICStop();
		}	
	}
	else
	{
		ReadData=0xffff;
		IICStop();
	}
	SDA_H;
	return(ReadData);

//	StartINT();
	  OSMutexPost(EEPROMDEVICE.IICMutex);
}


/***********************************************************************************************
用途:从一24C04读一个字节
输入参数：Adss,所读地址
输出参数：所读数据返回值 
***********************************************************************************************/
u16 ReadDataEEPROM(u16 Adss)
{
	u16 ReadData;

	u16 ADDH;
	u16 ADDH2;
	u16 ADDL;
   INT8U  err;
   OSMutexPend(EEPROMDEVICE.IICMutex,0,&err);
	if(Adss>0xff)
	{
		ADDH=0xa2;
		ADDH2=0xa3;
		ADDL=Adss&0x00ff;
	}
	else
	{
		ADDH=0xa0;
		ADDH2=0xa1;
		ADDL=Adss&0x00ff;
	}


	IICStart();

	if(IICSend(ADDH))
	{
		if(IICSend(ADDL))				//写地址
		{
			IICStart();
			if(IICSend(ADDH2))
			{
				ReadData=IICRead();
				NoAck();	
				IICStop();
			}
			else
			{
				ReadData=0xffff;
				IICStop();
			}
		}
		else
		{
			ReadData=0xffff;
			IICStop();
		}	
	}
	else
	{
		ReadData=0xffff;
		IICStop();
	}
	OSMutexPost(EEPROMDEVICE.IICMutex);
	return(ReadData);
}




