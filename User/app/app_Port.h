

#ifndef _APP_PORT11_H
#define _APP_PORT11_H

#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"
#include "bsp_TimesInt.h"
#include "app_kernelregs.h"
#include "bsp_eeprom.h"

void UsbState(void );
void APortInput(void);				//模拟量输入口

void APortOutput(void);				//模拟量输出口

void App_Paramload(void);
void  ADCTsetS();
void  ADCZeroS();

uint  InputFilter(uint *InAddress);				//输放滤波

uint  OutFilter(u16);

#endif



