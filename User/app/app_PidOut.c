
#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"
#include "bsp_TimesInt.h"
#include "app_kernelregs.h"
#include "bsp_eeprom.h"
#include "app_PidOut.h"

int  IYuShu;		//PID 的I的余数,有符号


void PPB()
{
	KPout=(((PPP *  Pid.KP*Pid.KB )/10000)*Param.Basic.KPid)/1000;     // 比例项  最大  正
}

void PPM()
{
	KPout=(((PPP *  Pid.KP)/100)*Param.Basic.KPid)/1000;							// 比例项  中	正
}
void PPS()
{
	KPout=(((PPP *  Pid.KP*Pid.KS)/10000)*Param.Basic.KPid)/1000;
}
void PZO()
{
	KPout=0;											// 比例项  0
}
void PNS()
{
	KPout=0-((((PPP *  Pid.KP*Pid.KS)/10000)*Param.Basic.KPid)/1000);				// 比例项  最小	负
}
void PNM()
{
	KPout=0-((((PPP *  Pid.KP)/100)*Param.Basic.KPid)/1000);
}
void PNB()
{
	KPout=0-((((PPP *  Pid.KP*Pid.KB )/10000)*Param.Basic.KPid)/1000);				// 比例项  最大	负
}

void IPB()
{
	int  Data;
//	int   ABSI;		//I余数的绝对值
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*Pid.KB)<=0xa30000)
	 {
	 	 Data=(Pid.KB*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;
	//	 Data=Data+IYuShu;
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(Pid.KB*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 L64Data=L64Data+IYuShu;
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	Pid.SumError=Pid.SumError+Data;				// 积分项  最大  正
}
void IPM()
{
	int  Data;
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*100)<=0xa30000)
	 {
	 	 Data=(100*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(100*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }	
 	Pid.SumError=Pid.SumError+Data;				// 积分项  中  正
}
void IPS()
{
	int  Data;
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*Pid.KS)<=0xa30000)
	 {
	 	 Data=(Pid.KS*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(Pid.KS*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	Pid.SumError=Pid.SumError+Data; 				// 积分项  最小  正
}
void IZO()
{
//	Pid.SumError=Pid.SumError;	
}
void INS()
{
	int  Data;
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*Pid.KS)<=0xa30000)
	 {
	 	 Data=(Pid.KS*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(Pid.KS*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	Pid.SumError=Pid.SumError-Data; 				// 积分项  最小  正	
}
void INM()
{
	int  Data;
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*100)<=0xa30000)
	 {
	 	 Data=(100*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(100*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 Pid.SumError=Pid.SumError-Data;						// 积分项  中    负
}
void INB()
{
	int  Data;
//	int   ABSI;		//I余数的绝对值
	long  L64Data;
	 Data=PPP/10;				//把0～10000变成0～1000
	 if((Pid.CycT*Data*Pid.KB)<=0xa30000)
	 {
	 	 Data=(Pid.KB*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;
	//	 Data=Data+IYuShu;
		 IYuShu=Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	 else
	 {
  	 	 L64Data=(Pid.KB*(Pid.CycT/5)*Data*Param.Basic.KPid)+IYuShu;		//当前值加上次余数值
		 L64Data=L64Data+IYuShu;
		 IYuShu=L64Data%(Pid.KI*10000);			//新的余数，留做下次用
		 Data=L64Data/(Pid.KI*10000);	  			//10000=100(Pid.KB的百分比）*100（Param.Basic.KPid百分比）
	 }
	Pid.SumError=Pid.SumError-Data-1;				// 积分项  最大  负
}

void DPB()
{
	KDout=(((Pid.KP*Pid.KD*Pid.KB* DDD)/(Pid.CycT*10000))*Param.Basic.KPid)/100;					//微分项	最大 	正
}
void DPM()
{
	KDout=(((Pid.KP*Pid.KD* DDD)/(Pid.CycT*100))*Param.Basic.KPid)/100;
}
void DPS()
{
	KDout=(((Pid.KP*Pid.KD*Pid.KS* DDD)/(Pid.CycT*10000))*Param.Basic.KPid)/100;						// 微分项  最小	 正
}
void DZO()
{
	KDout=0;
}
void DNS()
{
	KDout=0-(((Pid.KP*Pid.KD*Pid.KS* DDD)/(Pid.CycT*10000))*Param.Basic.KPid)/100;					// 微分项  最小	负
}
void DNM()
{
	KDout=0-(((Pid.KP*Pid.KD* DDD)/(Pid.CycT*100))*Param.Basic.KPid)/100;	
}
void DNB()
{
	KDout=0-(((Pid.KP*Pid.KD*Pid.KB* DDD)/(Pid.CycT*10000))*Param.Basic.KPid)/100;						// 微分项  最大	负
}

void PidOut_Cap()
{
u32 LData32;
	if(Param.Basic.Mode==2)		//模糊PID
	{
		LData32=Pid.SetD*Pid.Over;
		State.PID.Over=LData32/100;			//超调区

		LData32=Pid.SetD*Pid.Die;
		State.PID.Die=LData32/100;			//死区

		LData32=10000*Pid.Change;
		State.PID.ChangeH=LData32/50;
		State.PID.ChangeL=LData32/200;
		if(Error>0)
		{
			if(Error>State.PID.Over)						//E   PB//实际值偏小,超调区
			{
				if(dErr>0)									//越来越大
				{
					if(dErr>State.PID.ChangeH)					//PB  PB
					{							
						KPPB					// 比例项  最大
						KIPB					//积分项最大
						KDPB					//微分项最大
						Mod=1000;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PB PM
					{
						KPPB					// 比例项  最大
						KIPB					//积分项最大
						KDPB					//微分项最大
						Mod=1000;
						Mod2=800;
					}
					else 				//PB PS
					{
						KPPB					// 比例项  最大
						KIPM					//积分项太高容易超调
						KDPM					//微分项最大
						Mod=1000;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PB PZ
				{
					KPPB					// 比例项  最大
					KIPM					//积分项最大
					KDPS					//微分项   0
					Mod=1000;
					Mod2=500;

				}
				else			//DDD=0-dError,变成正
				{
				    if(DDD<State.PID.ChangeL)				//PB NS，dError小负
					{
						KPPM					// 
						KIPS					//
						KDZO					//
						Mod=1000;
						Mod2=400;
					}
					else if(DDD<State.PID.ChangeH)				//PB NM，中负
					{
						KPPS					// 比例项  最大
						KIPS					//积分项最大
						KDZO					//微分项最大 					//小 0 0
						Mod=1000;
						Mod2=200;
					}
					else
					{
						KPPS
						KIZO
						KDZO
						Mod=1000;
						Mod2=0x0;
					}
				}
			}	
			else if(Error>State.PID.Die)						//正常区
			{
				if(dErr>0)
				{
					if(dErr>State.PID.ChangeH)					//PB  PB
					{							
						KPPB					// 比例项  最大
						KIPB					//积分项最大
						KDPB					//微分项最大		//大 大 大
						Mod=800;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PB PM
					{
						KPPB					// 比例项  中
						KIPM					//积分项最大
						KDPM					//微分项最大		/
						Mod=800;
						Mod2=800;
					}
					else 				//PB PS
					{
						KPPM					// 比例项  中
						KIPM					//积分项最中
						KDPM					//微分项最中		/				
						Mod=800;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PB PZ
				{
					KPPM					// 比例项  中
					KIPS					//积分项   中
					KDPS					//微分项   中		/		//中 中 中 
						Mod=800;
						Mod2=500;
				}
				else
				{
					if(DDD<State.PID.ChangeL)				//PB NS
					{
						KPPS					// 比例项  小
						KIPS					//积分项   小
						KDPS					//微分项   小		/				//小  小  -小 
						Mod=800;
						Mod2=400;
					}
					else if(DDD<State.PID.ChangeH)				//PB NM
					{
						KPPS
						KIZO
						KDZO				//0 0 0
						Mod=800;
						Mod2=200;
					}
					else
					{
						KPZO
						KIZO
						KDNS				//0 0 0
						Mod=800;
						Mod2=0x0;
					}
				}
			}
	
			else if(Error>0)						//死区
			{
				if(dErr>0)
				{
					if(dErr>State.PID.ChangeH)					//PS  PB
					{							
						KPPM					// 比例项  中
						KIPM					//积分项   中
						KDPM					//微分项   中		/		//中 中 中 
						Mod=600;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PS PM
					{
						KPPM					// 比例项  中
						KIPM					//积分项   中
						KDPS					//微分项   中		/		//中 中 中 
						Mod=600;
						Mod2=800;
					}
					else 
					{
						KPPS					// 比例项   小
						KIPS					//积分项    小
						KDPS					//微分项    小		/
						Mod=600;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PS PZ
				{
					KPPS					// 比例项   小
					KIPS					//积分项    小
					KDZO					//微分项    0
						Mod=600;
						Mod2=500;
				}
				else
				{
					if(DDD<State.PID.ChangeL)				//PS NS
					{
						KPZO
						KIZO
						KDZO				//0 0 0
						Mod=600;
						Mod2=400;
					}
					else if(DDD<State.PID.ChangeH)				//PS NM
					{
						KPZO					// 比例项   -小
						KINS					//积分项    -小
						KDNS					//微分项    -小
						Mod=600;
						Mod2=200;
					}
					else
					{
						KPNS					// 比例项   -小
						KINS					//积分项    -小
						KDNM					//微分项    -小
						Mod=600;
						Mod2=0;
					}
				}
			}
		}
		else if(Error==0)
		{
			if(dErr>0)
			{
				if(dErr>State.PID.ChangeH)					//PS  PB
				{							
					KPPM					// 比例项  中
					KIPM					//积分项   中
					KDPM					//微分项   中		/		//中 中 中 
						Mod=500;
						Mod2=1000;
				}
				else if(dErr>State.PID.ChangeL)				//PS PM
				{
					KPPS					// 比例项  中
					KIPS					//积分项   中
					KDPM					//微分项   中		/		//中 中 中			//中 大 大
						Mod=500;
						Mod2=800;
				}
				else 				//PS PS
				{
					KPPS					// 比例项   小
					KIPS					//积分项    小
					KDPS					//微分项    小
						Mod=500;
						Mod2=600;
				}
			}
			else if(dErr==0)				//PS PZ
			{
				KPZO
				KIZO
				KDZO				//0 0 0
						Mod=500;
						Mod2=500;
			}
			else
			{
				if(DDD<State.PID.ChangeL)				//PS NS
				{
					KPNS					// 比例项   -小
					KINS					//积分项    -小
					KDNS					//微分项    -小
						Mod=500;
						Mod2=400;
				}
				else if(DDD<State.PID.ChangeH)				//PS NM
				{
					KPNS					// 比例项   -小
					KINS					//积分项    -小
					KDNM					//微分项    -小
						Mod=500;
						Mod2=200;
				}
				else
				{
					KPNM					// 比例项   -中
					KINM					//积分项    -中
					KDNM					//微分项    -中
						Mod=500;
						Mod2=0x0;
				}
			}
		}
		else 			//小于0
		{
			if(PPP<State.PID.Die)			  //死区
			{
				if(dErr>0)
				{
					if(dErr>State.PID.ChangeH)					//PS  PB    输出＋
					{							
						KPPS					// 比例项    ＋
						KIPS					//+   可改到M				//???????????????????????????????  标准是KIPS
						KDPM					//＋
						Mod=400;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PS PM		 输出小＋
					{
						KPZO					// 比例项 中
						KIPS		
						KDPS
						Mod=400;
						Mod2=800;
					}
					else 				//PS PS		输出0
					{
						KPZO					// 比例项 中
						KIZO		
						KDZO
						Mod=400;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PS PZ  输出小－
				{
					KPNS					// 比例项 -小
					KINS					//－小
					KDZO					//0
						Mod=400;
						Mod2=500;
				}
				else
				{
					if(DDD<State.PID.ChangeL)				//PS NS  输出中－
					{
						KPNS					// 比例项 中		-
						KINS					//-
						KDNS					//-
						Mod=400;
						Mod2=400;
					}
					else if(DDD<State.PID.ChangeH)				//PS NM
					{
						KPNM					// 比例项 －中
						KINM					//-	
						KDNM					//-
						Mod=400;
						Mod2=200;
					}
					else
					{
						KPNM					// 比例项 中  -
						KINM					//-
						KDNM					//-
						Mod=400;
						Mod2=0x0;
					}
				}
			}
			else if(PPP<State.PID.Over)
			{
				if(dErr>0)
				{
					if(dErr>State.PID.ChangeH)					//PS  PB
					{							
						KPZO					// 
						KIZO		
						KDPS
						Mod=200;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PS PM
					{
						KPNS					// 
						KIZO		
						KDZO
						Mod=200;
						Mod2=800;
					}
					else 				//PS PS		输出变小，张力变大
					{
						KPNS					// 比例项 中	-
						KINS					//    -
						KDNS					//-
						Mod=200;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PS PZ
				{
					KPNM					// 比例项 中	-
					KINS					//-	
					KDNS					//0
						Mod=200;
						Mod2=500;
				}
				else
				{
					if(DDD<State.PID.ChangeL)				//PS NS
					{
						KPNM					// 比例项 中  -
						KINM					//-
						KDNM					//
						Mod=200;
						Mod2=400;
		
					}
					else if(DDD<State.PID.ChangeH)				//PS NM
					{
						KPNB					// 比例项 中  -
						KINM					//-
						KDNM					//
						Mod=200;
						Mod2=200;
					}
					else
					{
						KPNB					// 比例项 中  -
						KINB					//-
						KDNB					//-
						Mod=200;
						Mod2=0x0;
					}
				
				}
			}
			else
			{
				if(dErr>0)
				{
					if(dErr>State.PID.ChangeH)					//PS  PB
					{							
						KPNS					// 
						KIZO		
						Mod=0;
						Mod2=1000;
					}
					else if(dErr>State.PID.ChangeL)				//PS PM
					{
						KPNS					// -
						KINS		
						KDZO
						Mod=0x0;
						Mod2=800;
					}
					else 				//PS PS		输出变小，张力变大
					{
						KPNM					// 比例项 中	-
						KINS					//    -
						KDZO					//-
						Mod=0x0;
						Mod2=600;
					}
				}
				else if(dErr==0)				//PS PZ
				{
					KPNM					// 比例项 中	-
					KINM					//-	
					KDNS					//0
						Mod=0x0;
						Mod2=500;
				}
				else 
				{
					if(DDD<State.PID.ChangeL)				//PS NS
					{
						KPNM					// 比例项 中  -
						KINM					//-
						KDNM					//-
						Mod=0x0;
						Mod2=400;
		
					}
					else if(DDD<State.PID.ChangeH)				//PS NM
					{
						KPNB					// 比例项 中  -
						KINB					//-
						KDNB					//
						Mod=0x0;
						Mod2=200;
					}
					else
					{
						KPNB					// 比例项 中  -
						KINB					//-
						KDNB					//-
						Mod=0x0;
						Mod2=0x0;
					}
				}
			}
		}
	}
	else if(Param.Basic.Mode==1)
	{
		if(Error>=0)
		{
			if(dErr>0)
			{
				KPPM					// 比例项 中  -
				KIPM					//-
				KDPM					//-
					Mod=0x99;
			}
			else
			{
				KPPM					// 比例项 中  -
				KIPM					//-
					Mod=0x99;
			}
		}
		else
		{
			if(dErr>0)
			{
				KPNM					// 比例项 中  -
				KINM					//-
					Mod=0x99;
			}
			else
			{
				KPNM					// 比例项 中  -
				KINM					//-
				KDNM					//-
					Mod=0x99;
			}
		}
	}
	else if(Param.Basic.Mode==4)			//手动  /恒定
	{
//		KPout=10000;
//  		KPout=Param.Basic.SetOut*KPout/1000;		//输出值=设置值（千分比）*10V（10000）/1000	
		KPout=Param.Basic.SetOut*10;
		Pid.SumError=0;
		KDout=0;
	}
}


///////////////////////////////////////卷径控制方式输出//////////////////////////////////////////////
uint DaiOut()
{
uint DAIOut;
ulong LLLL;
//	if(Param.Basic.MostOut==1)		//扭矩输出
	{
		DAIOut=Param.Basic.LeastOut+(Param.Basic.MostOut-Param.Basic.LeastOut)*State.Work.DiaPer/1000;
//		if(Param.Basic.MachMode==2)				//收卷
		{
			if(Param.Basic.ZDLine==0)			//恒张力控制
			{
				DAIOut=DAIOut;					
			}
			if(Param.Basic.ZDLine==1)			//直线锥度
			{
				LLLL=State.Work.DiaPer*Param.Basic.ZDZ;	//FX=F0(1-State.Work.Dia*Param.Basic.ZDZ)
				LLLL=1000000-LLLL;					//State.Work.Dia=DX/DMAX  100.0%
				LLLL=LLLL*DAIOut;
				DAIOut=LLLL/1000000;
			}
			else if(Param.Basic.ZDLine==2)		//曲线
			{
				LLLL=State.Work.DiaPer/2;					//XXX=State.Work.Dia*曲线率
				//huzhiwen comment LLLL=3162-ZDQX[LLLL];
				LLLL=3162000-LLLL*Param.Basic.ZDZ;	//FX=F0*XXXX*Param.Basic.ZDZ)
				LLLL=LLLL*DAIOut;
				DAIOut=LLLL/3162000;
			}
		}
	}
//	else if(Param.Basic.MostOut==2)		//	//速度模式，卷径越大，输出越小。速度控制无法实现锥度控制
	{
	//	if(Param.Basic.MachMode==1)				//放卷
		{
			DAIOut=Param.Basic.LeastOut+(Param.Basic.MostOut-Param.Basic.LeastOut)*(1000-State.Work.DiaPer)/1000;
		}
	//	else if(Param.Basic.Mache==2)				//收卷
		{

		}
	}
	DAIOut=DAIOut*4096/1000;
	return (DAIOut);
}


