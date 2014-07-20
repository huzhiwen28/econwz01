/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_button.c
*	说    明 : 实现按键的检测，具有软件滤波机制，可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
*				(5) 组合键
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_button.h"

static BUTTON_T s_BtnUser;		/* USER 键 */
static BUTTON_T s_BtnTamper;	/* TAMPER 键 */
static BUTTON_T s_BtnWakeUp;	/* WAKEUP 键 */

static BUTTON_T s_BtnUserTamper;/* 组合键，USER和TAMPER键 */

static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */

static void bsp_InitButtonVar(void);
static void bsp_InitButtonHard(void);
static void bsp_DetectButton(BUTTON_T *_pBtn);

/*
	按键口线分配：
	USER1键     : PC6  (低电平表示按下)
	TAMPEER键  : PC13 (低电平表示按下)
	WKUP键     : PA0  (!!!高电平表示按下)

	定义函数判断按键是否按下，返回值1 表示按下，0表示未按下
*/
static uint8_t IsKeyDownUser(void) 		{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_8) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownTamper(void) 	{if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET) return 0; return 1;}
//static uint8_t IsKeyDownWakeUp(void) 	{if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET) return 1; return 0;}
static uint8_t IsKeyDownUserTamper(void) {if (IsKeyDownUser() && IsKeyDownTamper()) return 0; return 1;}	/* 组合键 */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButton
*	功能说明: 初始化按键
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitButton(void)
{
	bsp_InitButtonVar();		/* 初始化按键变量 */
	bsp_InitButtonHard();		/* 初始化按键硬件 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参：_KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_Key.Buf[s_Key.Write] = _KeyCode;

	if (++s_Key.Write  >= KEY_FIFO_SIZE)
	{
		s_Key.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_Key.Read == s_Key.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_Key.Buf[s_Key.Read];

		if (++s_Key.Read >= KEY_FIFO_SIZE)
		{
			s_Key.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonHard
*	功能说明: 初始化按键硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitButtonHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
	按键口线分配：
	USER1键     : PC6  (低电平表示按下)
	TAMPEER键  : PC13 (低电平表示按下)
	WKUP键     : PA0  (!!!高电平表示按下)
	*/
			
	/* 第1步：打开GPIOA GPIOC GPIOD GPIOF GPIOG的时钟
	   注意：这个地方可以一次性全打开
	*/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
//			| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG, ENABLE);
	
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
/*	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);*/	/* PA0 */
/*	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);*/	/* PC13 */
/*	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);*/	/* PC6 */
}
	
/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonVar
*	功能说明: 初始化按键变量
*	形    参：strName : 例程名称字符串
*			  strDate : 例程发布日期
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitButtonVar(void)
{
	/* 对按键FIFO读写指针清零 */
	s_Key.Read = 0;
	s_Key.Write = 0;

	/* 初始化USER按键变量，支持按下、弹起、长按 */
	s_BtnUser.IsKeyDownFunc = IsKeyDownUser;		/* 判断按键按下的函数 */
	s_BtnUser.FilterTime = BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnUser.LongTime = BUTTON_LONG_TIME;			/* 长按时间 */
	s_BtnUser.Count = s_BtnUser.FilterTime / 2;		/* 计数器设置为滤波时间的一半 */
	s_BtnUser.State = 0;							/* 按键缺省状态，0为未按下 */
	s_BtnUser.KeyCodeDown = KEY_DOWN_USER;			/* 按键按下的键值代码 */
	s_BtnUser.KeyCodeUp = KEY_UP_USER;				/* 按键弹起的键值代码 */
	s_BtnUser.KeyCodeLong = KEY_LONG_USER;			/* 按键被持续按下的键值代码 */
	s_BtnUser.RepeatSpeed = 0;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnUser.RepeatCount = 0;						/* 连发计数器 */		

	/* 初始化TAMPER按键变量，支持按下 */
	s_BtnTamper.IsKeyDownFunc = IsKeyDownTamper;	/* 判断按键按下的函数 */
	s_BtnTamper.FilterTime = BUTTON_FILTER_TIME;	/* 按键滤波时间 */
	s_BtnTamper.LongTime = 0;						/* 长按时间, 0表示不检测 */
	s_BtnTamper.Count = s_BtnTamper.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnTamper.State = 0;							/* 按键缺省状态，0为未按下 */
	s_BtnTamper.KeyCodeDown = KEY_DOWN_TAMPER;		/* 按键按下的键值代码 */
	s_BtnTamper.KeyCodeUp = 0;						/* 按键弹起的键值代码 */
	s_BtnTamper.KeyCodeLong = 0;					/* 按键被持续按下的键值代码 */
	s_BtnTamper.RepeatSpeed = 0;					/* 按键连发的速度，0表示不支持连发 */
	s_BtnTamper.RepeatCount = 0;					/* 连发计数器 */	

	/* 初始化WAKEUP按键变量，支持按下 */
//	s_BtnWakeUp.IsKeyDownFunc = IsKeyDownWakeUp;	/* 判断按键按下的函数 */
	s_BtnWakeUp.FilterTime = BUTTON_FILTER_TIME;	/* 按键滤波时间 */
	s_BtnWakeUp.LongTime = 0;						/* 长按时间 */
	s_BtnWakeUp.Count = s_BtnWakeUp.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnWakeUp.State = 0;							/* 按键缺省状态，0为未按下 */
	s_BtnWakeUp.KeyCodeUp = 0;						/* 按键弹起的键值代码，0表示不检测 */
	s_BtnWakeUp.KeyCodeDown = KEY_DOWN_WAKEUP;		/* 按键按下的键值代码 */
	s_BtnWakeUp.KeyCodeLong = 0;					/* 按键被持续按下的键值代码，0表示不检测 */
	s_BtnWakeUp.RepeatSpeed = 10;					/* 按键连发的速度，0表示不支持连发 */
	s_BtnWakeUp.RepeatCount = 0;					/* 连发计数器 */		

	/* 初始化组合按键变量，支持按下 */
	s_BtnUserTamper.IsKeyDownFunc = IsKeyDownUserTamper;	/* 判断按键按下的函数 */
	s_BtnUserTamper.FilterTime = BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnUserTamper.LongTime = 0;							/* 长按时间 */
	s_BtnUserTamper.Count = s_BtnUserTamper.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnUserTamper.State = 0;								/* 按键缺省状态，0为未按下 */
	s_BtnUserTamper.KeyCodeDown = KEY_DOWN_USER_TAMPER;		/* 按键按下的键值代码 */
	s_BtnUserTamper.KeyCodeUp = 0;							/* 按键弹起的键值代码，0表示不检测 */
	s_BtnUserTamper.KeyCodeLong = 0;						/* 按键被持续按下的键值代码，0表示不检测 */
	s_BtnUserTamper.RepeatSpeed = 0;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnUserTamper.RepeatCount = 0;						/* 连发计数器 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectButton
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectButton(BUTTON_T *_pBtn)
{
	/* 如果没有初始化按键函数，则报错
	if (_pBtn->IsKeyDownFunc == 0)
	{
		printf("Fault : DetectButton(), _pBtn->IsKeyDownFunc undefine");
	}
	*/

	if (_pBtn->IsKeyDownFunc())
	{
		if (_pBtn->Count < _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count < 2 * _pBtn->FilterTime)
		{
			_pBtn->Count++;
		}
		else
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;

				/* 发送按钮按下的消息 */
				if (_pBtn->KeyCodeDown > 0)
				{
					/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeDown);
				}
			}

			if (_pBtn->LongTime > 0)
			{
				if (_pBtn->LongCount < _pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++_pBtn->LongCount == _pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						bsp_PutKey(_pBtn->KeyCodeLong);						
					}
				}
				else
				{
					if (_pBtn->RepeatSpeed > 0)
					{
						if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
						{
							_pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							bsp_PutKey(_pBtn->KeyCodeDown);														
						}
					}
				}
			}
		}
	}
	else
	{
		if(_pBtn->Count > _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count != 0)
		{
			_pBtn->Count--;
		}
		else
		{
			if (_pBtn->State == 1)
			{
				_pBtn->State = 0;

				/* 发送按钮弹起的消息 */
				if (_pBtn->KeyCodeUp > 0)
				{
					/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeUp);			
				}
			}
		}

		_pBtn->LongCount = 0;
		_pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyPro(void)
{
	bsp_DetectButton(&s_BtnUser);		/* USER 键 */
	bsp_DetectButton(&s_BtnTamper);		/* TAMPER 键 */
	bsp_DetectButton(&s_BtnWakeUp);		/* WAKEUP 键 */
	bsp_DetectButton(&s_BtnUserTamper);	/* 组合键 */
}
