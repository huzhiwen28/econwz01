#ifndef __485_H
#define __485_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"

/*485串口初始化*/
void BSP_485Init(void);

/*485串口打开*/
void BSP_485Open(void);

/*485串口关闭*/
void BSP_485Close(void);

/*485串口读*/
u16 BSP_485Read(u8* buff,u16 len);

/*485串口读缓冲区中长度*/
u16 BSP_485InBuffLen(void);

/*485串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_485Write(u8* buff,u16 len);

/*485串口驱动，在大循环中一直运行*/
void BSP_485Run(void);


/*485清除读缓冲*/
void BSP_485ClearReadBuff(void);

#endif

