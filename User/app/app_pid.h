
/*  
邱毅
time:2008,09,26
胡志文移植 20120215
*/
#ifndef _APP_PID_H
#define _APP_PID_H

#include "stm32f10x.h"
#include "app_basicdef.h"



extern uint PPP,DDD;
extern uint Mod,Mod2;
extern int	Error,dErr;

extern	struct PidRunState RUNState;
//extern int KPO[300];
//extern int KDO[300];
//extern int KIO[300];
//extern uint KO[300];
//extern int  KS[300];
//extern uint   KM[300];
//extern int  KE[300];
//extern int  KPP[300];
//extern uint AAAA;

void SetPIDCycT(void);							//设置PID周期

void InitPIDOut(void);


uint PIDOut(int NextPoint );

uint DaiOut(void);
void  PID_StateRead(void);
void  PID_StateInit(void);
void  PID_StateSave(void);

#endif


