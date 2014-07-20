#ifndef __FIFO_H
#define __FIFO_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"

/*fifo作为基础构件提供给驱动使用,fifo大小固定为500个字节长度*/

/* FIFO结构体*/
struct _fifo
{
   //fifo的方式规避避免中断产生数据不全的现象
   volatile u8 FIFO[500];
   
   //ip 指向下一个空闲位
   volatile u16 ip;
   
   //op 指向第一个字符
   volatile u16 op;
   
   //fifo长度
   volatile u16 fifol;

};

/*初始化FIFO*/
u8 BSP_InitFIFO(struct _fifo* handle);

/*返回0表示一切正常，其他值表示未放入队列的数量值*/
u16 BSP_PushinFIFO(struct _fifo* handle,u8* buff,u16 len);

/*将fifo中的值取出，返回取出的长度*/
u16 BSP_PopoutFIFO(struct _fifo* handle,u8* buff,u16 len);

/*返回长度*/
u16 BSP_FIFOLen(struct _fifo* handle);

u8 BSP_ClearFIFO(struct _fifo* handle);
#endif


