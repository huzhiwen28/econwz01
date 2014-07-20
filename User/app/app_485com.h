#ifndef __APP_485COM_H
#define __APP_485COM_H
#include "stm32f10x.h"

/*modbus帧读写*/

//CRC校验的函数
u16 CRC16(unsigned char *puchMsg, unsigned short usDataLen);

//初始化
void C485COM_Init(void);

//打开设备
void C485COM_Open(void);

//关闭设备
void C485COM_Close(void);

//写字符
int C485COM_Write(u8* chardata,int len);

//任务执行
void C485COM_Run(void *p_arg);


#endif
