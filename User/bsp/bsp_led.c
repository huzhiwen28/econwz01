/*
*********************************************************************************************************
*	                                  
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	说    明 : 驱动LED指示灯
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_led.h"
#define	 NEWBROD 1
/*
#define COMMLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_2);
#define COMMLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_2);

#define RunLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_3);
#define RunLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_3);

#define ERRLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_4);
#define ERRLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_4);

#define JOGLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_5);
#define JOGLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_5);
*/
/*
#define RunLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_3);
#define RunLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_3)

*/

#if NEWBROD
 


#define ShowClk_H	 GPIO_SetBits(GPIOG, GPIO_Pin_15);
#define ShowClk_L	 GPIO_ResetBits(GPIOG, GPIO_Pin_15);

#define ShowData_H	 GPIO_SetBits(GPIOG, GPIO_Pin_14);
#define ShowData_L	 GPIO_ResetBits(GPIOG, GPIO_Pin_14);


 #else

 #define ShowClk_H	 GPIO_SetBits(GPIOC, GPIO_Pin_11);
#define ShowClk_L	 GPIO_ResetBits(GPIOC, GPIO_Pin_11);

#define ShowData_H	 GPIO_SetBits(GPIOC, GPIO_Pin_10);
#define ShowData_L	 GPIO_ResetBits(GPIOC, GPIO_Pin_10);

#endif

void LedShow(u16 Show)
{
 	u16 aa;
	u16 bb;
	ShowClk_H
	bb=Show;

//	BSP_DelayUS(1);
 	for(aa=0;aa<8;aa++)		//8位数据
	{
		ShowClk_L
	
		;
		if(((bb&0x0080)>>7)>0)
		{
		  	ShowData_H
		}
		else
		{
			ShowData_L
		}
//		ShowData=1;
		bb=bb<<1;
		ShowClk_H
		;
	}
}
 void bsp_InitLed()		   //初始化
{							//全灭
	LedShow(0xff);
}

void bsp_LedDisplay(u16 LedO)
{
	static  u16 LedState;
	if(	LedState!=LedO)
	{
		LedState=LedO;
		switch(LedO)
		{
		   case 0:  LedShow(0xff);break ;	  //全灭
		   case 1:  LedShow(0xfe);break ;	  //
		   case 2:  LedShow(0xfc);break ;	  //
		   case 3:  LedShow(0xf8);break ;	  //
		   case 4:  LedShow(0xf0);break ;	  //
		   case 5:  LedShow(0xe0);break ;	  //
		   case 6:  LedShow(0xc0);break ;	  //
		   case 7:  LedShow(0x80);break ;	  //
		   case 8:  LedShow(0x00);break ;	  //全开
		}
	}
}





