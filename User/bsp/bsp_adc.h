#ifndef __ADC_H
#define __ADC_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"
/*adc1的通道10 11 12 13采样模拟量
DMA方式，采样周期为20us
*/

/*ADC数据指针*/
extern u16* BSP_ADC1;
extern u16* BSP_ADC2;
extern u16* BSP_ADC3;
extern u16* BSP_ADC4;

extern struct _ADC ADCDEVICE;

/*EEPROM设备*/
struct _ADC{
	  /*设备是否打开*/
	  u8 active;
	  u16 ADCDataTab[4*30];
	  u8 step;
};
//struct _ADC ADCDEVICE;
/*ADC初始化*/
void BSP_ADCInit(void);

/*ADC打开*/
void BSP_ADCOpen(void);

/*ADC关闭*/
void BSP_ADCClose(void);

#endif

