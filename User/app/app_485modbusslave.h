#ifndef __APP_485MODBUSSLAVE_H
#define __APP_485MODBUSSLAVE_H
#include "stm32f10x.h"

//通知modbus master处理数据帧
void C485_ModbusSlave_ProcessFrame(u8* chardata,int len);

#endif
