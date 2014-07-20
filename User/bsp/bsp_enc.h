#ifndef __ENC_H
#define __ENC_H 

/* Includes ------------------------------------------------------------------*/	   
#include "stm32f10x.h"
/*
*/
extern int64_t* BSP_ENC_CNT;

/*ENCODER设备*/
struct _ENC{
	  /*设备是否打开*/
	  u8 active;
	  int64_t currentCount;
};

/*ENC初始化*/
void BSP_ENCInit(void);

/*ENC打开*/
void BSP_ENCOpen(void);

/*ENC关闭*/
void BSP_ENCClose(void);

/*ENC运行驱动*/
void BSP_ENCRun(void *p_arg);

#endif

