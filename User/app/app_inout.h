#ifndef _APP_INOUT_H
#define _APP_INOUT_H
#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"


void Out1DAC(uint DAC1);
void Out2DAC(uint DAC2);
void Out3DAC(uint DAC3);
void Out4DAC(uint DAC4);
void InitPWM1(uint Speed );
void InitPWM2(uint Speed );

#endif

