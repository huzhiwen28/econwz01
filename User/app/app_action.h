

/*++++++++++++++++++++++++++++++++++++++
Action
主循环
+++++++++++++++++++++++++++++++++++++++*/

#ifndef _APP_ACTION_H
#define _APP_ACTION_H

#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"

extern uint AAA;

extern uint KFLG;

//开始运行
void StartAction(void);
//点动运行
void JogAction(void);

//点动停止
void JogStop(void);

//轴切运行
void ChangAction(uint S);			//轴切信号

void IoOutSwitch(uint S);		//IO设置输出开关

void OutSaveSet(uint S);		//输出记忆

void IOJog(uint S);

//加速
void IOSpeedUp(uint S);
//减速
void IOSpeedDown(uint S);

//A轴输出速度
void RollASpeed(void);

//B轴输出速度
void RollBSpeed(void);

//主牵引速度
void MainSpeed(void);

//模拟量输出
void OutPut(void);

//张力输出
void DISOutPut(void);


//运行切换
void RunSwitch( uint RunS);

//输入
void IOInput(void);

void  MainMotFR(uint RunS);

//开始输入
void StartIOInput(void);

void  DaiClear(void);	  //卷径数据清0
void DiaSave(void);

void DDInput(void);

				//输出滤波

#endif




