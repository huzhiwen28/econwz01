/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-EVAL Evaluation Board
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#ifndef  __BSP_H__
#define  __BSP_H__

#include  <cpu.h>

#include <stm32f10x_conf.h>
#include <stm32f10x.h>

#include "bsp_usart.h"
#include "bsp_int.h"
#include "bsp_button.h"
#include "bsp_led.h"

#include  <ucos_ii.h>

/*
*********************************************************************************************************
*                                           宏定义
*********************************************************************************************************
*/

/* 
  安富莱STM32F103ZE-EK开发板，按键和LED口线定义如下：
	USER键     : PG8  (低电平表示按下)
	TAMPER键  : PC13 (低电平表示按下)
	WKUP键     : PA0  (!!!高电平表示按下)
	摇杆UP键   : PG15 (低电平表示按下)
	摇杆DOWN键 : PD3  (低电平表示按下)
	摇杆LEFT键 : PG14 (低电平表示按下)
	摇杆RIGHT键: PG13 (低电平表示按下)
	摇杆SELECT键: PG7 (低电平表示按下)

	LED口线分配：
	LED1 : PF6  (输出0点亮)
	LED2 : PF7  (输出0点亮)
	LED3 : PF8  (输出0点亮)
	LED4 : PF9  (输出0点亮)
*/
#define UserKeyDown()   (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_8) == Bit_RESET)
#define TamperKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)
#define WkupKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET)
#define JoyUpKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)
#define JoyDownKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)
#define JoyLeftKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)
#define JoyRightKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)
#define JoyOkKeyDown() (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)

#define LED1_ON()         GPIO_ResetBits(GPIOF, GPIO_Pin_6);	/* 点亮LED1 */ 
#define LED1_OFF()        GPIO_SetBits(GPIOF, GPIO_Pin_6);		/* 熄灭LED1 */

#define LED2_ON()         GPIO_ResetBits(GPIOF, GPIO_Pin_6);	/* 点亮LED1 */ 
#define LED2_OFF()        GPIO_SetBits(GPIOF, GPIO_Pin_6);		/* 熄灭LED1 */

#define LED3_ON()         GPIO_ResetBits(GPIOF, GPIO_Pin_6);	/* 点亮LED1 */ 
#define LED3_OFF()        GPIO_SetBits(GPIOF, GPIO_Pin_6);		/* 熄灭LED1 */

#define LED4_ON()         GPIO_ResetBits(GPIOF, GPIO_Pin_6);	/* 点亮LED1 */ 
#define LED4_OFF()        GPIO_SetBits(GPIOF, GPIO_Pin_6);		/* 熄灭LED1 */

/*
*********************************************************************************************************
*                                           函数声明(用于外部模块调用)
*********************************************************************************************************
*/

void BSP_Init(void);
void BSP_IntDisAll(void);
uint32_t BSP_CPU_ClkFreq(void);
void Tmr_TickISR_Handler(void);

#endif
