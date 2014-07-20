#ifndef __SONIC_H
#define __SONIC_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"


/*超声波测距初始化
u8 filter :平均法滤波，滤波的单位个数1-10，默认1
u16 timems :毫秒单位的超声波测试间隔时间，时间最好大于80ms，以免回波引起误测试
*/
void BSP_SonicInit(u8 filter,u16 timems);

/*超声波测距打开*/
void BSP_SonicOpen(void);

/*超声波测距关闭*/
void BSP_SonicClose(void);

/*超声波测距读距离0.1mm单位*/
u16 BSP_SonicRead(void);

/*超声波测距运行*/
void BSP_SonicRun(void *p_arg);

#endif

