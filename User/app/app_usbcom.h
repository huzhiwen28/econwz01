#ifndef __APP_USBCOM_H
#define __APP_USBCOM_H
#include "stm32f10x.h"

/*
LRC校验值计算
charbuff 待计算的char数组
len 待计算的char数组长度
返回LRC计算结果
*/

char LRC(u8 *charbuff,int len);


/*
ascii码转换为char，例如"43" -> 0x43
charbuff 存放结果
ascii 待转换的ascii
len 待转换的长度，为偶数
*/
void ascii2char(u8* charbuff,u8* asciibuff,int len);

/*
char转换为ascii码，例如0x43 -> "43"
ascii 转换的结果ascii
charbuff 待转换
len 待转换的长度
*/
void chartoascii(u8* asciibuff,u8* charbuff,int len);

	//初始化
	void USBCOM_Init(void);

	//打开设备
	void USBCOM_Open(void);

	//关闭设备
	void USBCOM_Close(void);

	//写字符
	int USBCOM_Write(u8* chardata,int len);

	//任务执行
	void USBCOM_Run(void *p_arg);

	//通知任务有新消息来到
	void USBCOM_Newmsg(void);

//串口的后台线程
struct _USBCOM_Backend
{

	//缓冲
	u8 buff[1024];

	//空闲位置，指向数据后面的空白
	unsigned int buffend;

	//头位置
	int frameheadindex;

	//帧解析状态机 0没有头 1有头
	unsigned char framestate;

	//设备是否打开
	char comopen;

	//待检查时间
	u32 mstime;
	//时间记录
	u32 oldmstime;

	OS_EVENT *Newmsg; //新消息处理通知
};


#endif
