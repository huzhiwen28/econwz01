#ifndef __232C_H
#define __232C_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"


/*232C串口初始化*/
void BSP_232CInit(void);

/*232C串口打开*/
void BSP_232COpen(void);

/*232C串口关闭*/
void BSP_232CClose(void);

/*232C串口读*/
u16 BSP_232CRead(u8* buff,u16 len);

/*232C串口读缓冲区中长度*/
u16 BSP_232CInBuffLen(void);

/*232C串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_232CWrite(u8* buff,u16 len);

/*232C串口驱动，在大循环中一直运行*/
void BSP_232CRun(void);

/*485清除读缓冲*/
void BSP_232CClearReadBuff(void);


#endif

