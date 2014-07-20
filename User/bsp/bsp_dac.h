#ifndef __DAC_H
#define __DAC_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"
/*外部DAC使用SPI连接
*/

/*DAC数据指针*/
extern u16* BSP_DAC1;
extern u16* BSP_DAC2;
extern u16* BSP_DAC3;
extern u16* BSP_DAC4;

/*DAC设备*/
struct _DAC{
	  /*设备是否打开*/
	  u8 active;
	  u16 mode;
	  u16 DACDataTab[4];
};

/*DAC初始化*/
void BSP_DACInit(void);

/*DAC打开*/
void BSP_DACOpen(void);

/*DAC关闭*/
void BSP_DACClose(void);

/*DAC设置输出形式*/
void BSP_DACMode(u8 mode);

/*DAC驱动运行*/
void BSP_DACRun(void *p_arg);

/*DAC输出模式设置*/
void DacOutMode(u16 Ch ,u16 OutMode);

void  DacOutEN(u16 CHX,u16 OutX);

#endif

