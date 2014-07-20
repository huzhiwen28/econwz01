/*
*********************************************************************************************************
*	                                  
*	模块名称 : LED指示灯驱动模块    
*	文件名称 : bsp_led.h
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H

/* 供外部调用的函数声明 */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
void bsp_LedDisplay(u16 LedO);

#define COMMLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_2);
#define COMMLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_2);

#define RunLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_3);
#define RunLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_3);

#define ERRLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_4);
#define ERRLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_4);

#define JOGLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_5);
#define JOGLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_5);

#define XTLedOn   GPIO_SetBits(GPIOC, GPIO_Pin_10);
#define XTLedOff   GPIO_ResetBits(GPIOC, GPIO_Pin_10);
/*
#define RunLedOn   GPIO_SetBits(GPIOE, GPIO_Pin_3);
#define RunLedOff   GPIO_ResetBits(GPIOE, GPIO_Pin_3)

*/

#endif


