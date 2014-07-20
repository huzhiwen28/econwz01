#ifndef __APP_BACKEND_H
#define __APP_BACKEND_H
#include "stm32f10x.h"


struct _App_Backend
{
  	//PC机是否在线
  	volatile char PCOnline;

  	//心跳应答失败计数
  	volatile char RespFailCnt;

  	volatile char GatherFlag;
	OS_EVENT *FlushParamsg; //刷新参数消息处理通知
};

extern struct _App_Backend App_Backend;

//处理新的数据帧
void App_Backend_Init(void);
void App_Backend_NewFrame(void);

//心跳任务
void App_HeartbeatTask_Run(void *p_arg);

//参数刷新任务
void App_FlushParaTask_Run(void *p_arg);

//心跳任务


//参数发送任务


#endif
