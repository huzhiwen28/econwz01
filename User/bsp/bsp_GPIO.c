
/*GPIO初始化
邱毅
2012 03 27
*/

#include "stm32f10x.h"
#include <stdio.h>
#include <bsp_GPIO.h>


 /*************************************************
函数: void GPIO_Configuration(void）
功能: GPIOE配置		指示灯输出
A0:				  继电器1
A1:	              DOUT1
参数: 无
返回: 无
 */

/*******************************************************************************
	函数名：GPIO_Configuration
	输  入:
	输  出:
	功能说明：配置输入口
	
 	输  入口线分配
	IN1	  PG8
	IN2	  PG9
	IN3	  PG11
	IN4	  PG12
	IN5	  PG13
	IN6	  PG7

*/
/**************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;        //定义GPIO初始化结构体
 
   /* Enable GPIOE clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);  //允许GPIOE、AFIO时钟

  /* Configure PE.2,PE.3,PE.4,PE.5, as Output push-pull */ 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOE, &GPIO_InitStructure); 

     /* Configure PC.10,PC.11as Output push-pull */ 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PA.0,PA.1as Output push-pull */ 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);

      /* Configure PG.14,PG.15as Output push-pull */ 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14 | GPIO_Pin_15 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOG, &GPIO_InitStructure);


  GPIO_ResetBits(GPIOE, GPIO_Pin_2);		 //输出高
  GPIO_ResetBits(GPIOE, GPIO_Pin_3);		 //输出高
  GPIO_ResetBits(GPIOE, GPIO_Pin_4);		 //输出高
  GPIO_ResetBits(GPIOE, GPIO_Pin_5);		 //输出高

 GPIO_SetBits(GPIOA, GPIO_Pin_0);		 //输出高,断开
 GPIO_SetBits(GPIOA, GPIO_Pin_1);		 //输出高，断开


  /*
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);  //允许GPIOE、AFIO时钟
*/
  /* Configure PE.2,PE.3,PE.4,PE.5, as Output push-pull */ 
/*  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
  GPIO_Init(GPIOE, &GPIO_InitStructure); 



 GPIO_SetBits(GPIOF, GPIO_Pin_10);		 //输出高
 GPIO_SetBits(GPIOF, GPIO_Pin_11);		 //输出高


 */


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	//IO 输入口初试化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	/* PB5 */
				   
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);	/* PF6 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);	/* PF7 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);	/* PF8 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);	/* PF9 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	/* PC12 */

	//ID拨码开关
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	/* PC12 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);	/* PC12 */
}






