#ifndef __APP_ENCRYPTION_H
#define __APP_ENCRYPTION_H
#include "stm32f10x.h"
#include <ucos_ii.h>


struct _App_Encryption
{
	OS_EVENT *Invalidmsg; //非法消息
	OS_SEM_DATA Result;
  	unsigned int Device_Serial0;
	unsigned int Device_Serial1;
	unsigned int Device_Serial2;
};

extern struct _App_Encryption App_Encryption;

//初始化
void App_Encryption_Init(void);

//校验
void App_Encryption_Verify(void);

//否是非法，0 合法 1非法
char App_Encryption_Invalid(void);

//汇报cpuid给上位机
void  App_Encryption_ReportID(u8* chardata,int len);

//写加密串
void  App_Encryption_WriteEncry(u8* chardata,int len);

#endif
