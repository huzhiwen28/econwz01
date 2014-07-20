
/*  
邱毅
time:2008,09,26
胡志文移植 20120215
*/

#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"
#include "bsp_TimesInt.h"
#include "app_kernelregs.h"
#include "bsp_eeprom.h"
#include "app_PidOut.h"

uint PPP,DDD;
uint Mod,Mod2;
int	Error,dErr;
long PidErrAbs;
long PidErrO;
int  ErrAbs;
int	 ErrO;

//struct PidRunState PidRun;

u32 MSpeedP;
u32  KPSL;

void InitPIDOut()
{
	if(State.Basic.OutCh==0)		
	{
 		 Pid.SumError=State.Out.Data;
	}
	else 
	{
 		 Pid.SumError=State.Out.Data2;
	}
	  Pid.Start=0;
	RUNState.No=0;	 
	RUNState.UseNo=0;
}


uint PIDOut(int NextPoint )
{
static	int dError[5];
u16 JdE;
static	u16 MSPTimes=100;    //	判断卷径用
static	u32 LMS[5];	//上5次主速度
u16  MSPADD;   //加减速度标志,判断升降速用
static	u32  KPDS=100;    //动态PID系数
u32 LDataP;
u32 DiaOpenOut;       //根据卷径算出的开环输出值
u32  ABSKIOut;			//PID输出的KI输出 的 绝	对 值
	LDataP=10000;  //10V
	LDataP=LDataP*Param.Basic.MostOut;
	Pid.IFull=LDataP/100;				//积分饱和＝最大输出值，即Pid.IFull为最大输出值
	State.PID.MostOut=Pid.IFull;

	LDataP=10000;  //10V
	LDataP=LDataP*Param.Basic.LeastOut;
	Pid.IZero=LDataP/100;				//积分饱和＝最大输出值，即Pid.IFull为最大输出值
//	State.PID.MostOut=Pid.IFull;
		
    if(Param.Basic.Out==1)			//扭矩输出状态
	{
		if((Param.Basic.MachMode==1)||(Param.Basic.MachMode==2)||(Param.Basic.MachMode==3))				//放卷锥度  放卷时在扭矩输出状态，张力如果大于设置值，输出变小
		{
			if(Pid.Pol==1)								//PID的极性
			{
					Error = Pid.SetD-NextPoint;           // 偏差，如果设置值大于现实值，输出变大，如果设置值小于实际值，输出变小
			}
			else
			{
					Error = NextPoint-Pid.SetD;           // 偏差，如果设置值大于现实值，输出变小，如果设置值小于实际值，输出变大
			}
		}
	}
	else if(Param.Basic.Out==2)				//速度输出状态
	{
		if(Param.Basic.MachMode==1)				//放卷锥度  放卷时在速度输出状态，张力如果大于设置值，输出变大
		{
			if(Pid.Pol==2)								//PID的极性，，反极性
			{
					Error = Pid.SetD-NextPoint;           // 偏差，如果设置值大于现实值，输出变小，如果设置值小于实际值，输出变大

			}
			else								//出产默认值
			{	
					Error = NextPoint-Pid.SetD ;           // 偏差，如果设置值大于现实值，输出变大，如果设置值小于实际值，输出变小
												//实际值变大时，应加快速度，变小时减小速度
			}
		}
		else if((Param.Basic.MachMode==2)||(Param.Basic.MachMode==3))				//收卷锥度  张力若大于设置值，应减小速度，张力小了，应加速
		{
			if(Pid.Pol==1)								//PID的极性
			{
					Error = Pid.SetD-NextPoint;           // 偏差，设置值大于实际值时，输出增大，加速，使卷轴变紧
			}
			else
			{
					Error = NextPoint-Pid.SetD ;           // 偏差，
			}
		}
	}
	if(Pid.Start==0)	 //PID初始化
	{
		Pid.LastError=Error;
		Pid.Start=1;
		LMS[1]=State.Work.MSpeed;
		LMS[2]=State.Work.MSpeed;
		LMS[3]=State.Work.MSpeed;
		LMS[4]=State.Work.MSpeed;
	}
//****************************主速度升降速判断**************************************************
	MSPADD=100;						   //根据MSPADD的值来判断加减速。>103升，<97降
	for(JdE=0;JdE<4;JdE++)
	{
		  dError[JdE]=dError[JdE+1];	//微分数组，与速度判断无关
		  if((LMS[JdE+1]-LMS[JdE])>5)		 //主速度判断，后一个速度大于前一个速度0.005V
		  {
				MSPADD++;					 //加减速度判明
		  }
		  else if((LMS[JdE]-LMS[JdE+1])>5)
		  {
				MSPADD--;
		  }
	  LMS[JdE]=LMS[JdE+1];					//主速度 数组		  
	}
	LMS[4]=State.Work.MSpeed;		//当前主速度
	if((LMS[4]-LMS[3])>5)		 //主速度
	{
		MSPADD++;					 //加减速度判明
	}
	else if((LMS[3]-LMS[4])>5)
	{
		MSPADD--;
	}
//*************************微分与不完全微分*************************************************
	dError[4] = Error - Pid.LastError;             // 当前微分＝当前误差－上次误差
	Pid.LastError=Error;						  //记录当前误差，以备下次计算微分

	if(Error>=0)								 //误差为正
	{
		PPP=Error;
	}
	else
	{
		PPP=0-Error;
	}
	dErr=dError[4]*5+dError[3]*4+dError[2]*3+dError[1]*2+dError[0];	//微分滤波
	dErr=dErr/15;									//不完全微分
	if(dErr>=0)									   //微分为正
	{
		DDD=dErr;
	}
	else
	{
		DDD=0-dErr;
	}

	if(RUNState.UseNo<499)	 //0~498时
	{
		 if(RUNState.UseNo==0)
	 	{
	 		PidErrAbs=0;
			PidErrO=0;
	 	}
		RUNState.ErrAbs[RUNState.No]= PPP;		 //绝对值
	    RUNState.ErrO[RUNState.No]=Error; 		//有符号值
		PidErrAbs=PidErrAbs+RUNState.ErrAbs[RUNState.No];
		PidErrO=PidErrO+RUNState.ErrO[RUNState.No];
		ErrAbs=	PidErrAbs/(RUNState.UseNo+1);		//每个PID周期的误差绝对值的平均
		ErrO=PidErrO/(RUNState.UseNo+1);		//每个PID周期的平均误差绝对值
		RUNState.No++;
		RUNState.UseNo++;
	}
	else		//0~499已用满
	{
		if(PidErrAbs>=RUNState.ErrAbs[RUNState.No])
		{
			PidErrAbs=PidErrAbs-RUNState.ErrAbs[RUNState.No];
		}
		else
		{
			PidErrAbs=0;
		}
		PidErrO=PidErrO-RUNState.ErrO[RUNState.No];
		RUNState.ErrAbs[RUNState.No]= PPP;		 //绝对值
		RUNState.ErrO[RUNState.No]=Error; 		//有符号值	
		PidErrAbs=PidErrAbs+RUNState.ErrAbs[RUNState.No];
		PidErrO=PidErrO+RUNState.ErrO[RUNState.No];
		ErrAbs=	PidErrAbs/500;		//每个PID周期的误差绝对值的平均
		ErrO=PidErrO/(RUNState.UseNo+1);		//每个PID周期的平均误差绝对值
		if(RUNState.No<RUNState.UseNo)
		{
			RUNState.No++;	
		}
		else
		{
		  	RUNState.No=0;
		}
	}
	PidOut_Cap();
//************************************积分跟踪****************************************************8
	KIout=Pid.SumError;			//积分输出
	if(KIout>=0)
	{
		ABSKIOut=KIout;
	}
	else
	{
 		ABSKIOut=0-KIout;
	}
	if(Param.Basic.Out==2)				//速度输出状态
	{									//只有在速度模式下才有对速度的跟踪
		if(State.Work.MSpeed>200)	//主速度大于0.2V
		{
			if(KIout>=(PidOut/100))			//	  30这个值、/////以后要改成动态的比例关系  ,改为总输出值的1/20
			{
				if(ErrAbs<((State.PID.Over*35)/10))	   //误差在超调区以内
				{
					MSPTimes++;
					if(State.Basic.DiaOK==0)		//卷径未算出
					{
						MSPTimes++;
					}
					if(KIout>=(PidOut/12))		  //
					{
					  	MSPTimes++;
						if(State.Basic.DiaOK==0)		//卷径未算出
						{
							MSPTimes++;
						}
					}
					if(KIout>(PidOut/8))
					{
		  			  	MSPTimes++;
						if(State.Basic.DiaOK==0)		//卷径未算出
						{
							MSPTimes++;
						}
					}
					if(KIout>=(PidOut/4))
					{
					  	MSPTimes++;
						if(State.Basic.DiaOK==0)		//卷径未算出
						{
							MSPTimes++;
						}
					}
					if(KIout>=(PidOut/2))
					{
					  	MSPTimes++;
						if(State.Basic.DiaOK==0)		//卷径未算出
						{
							MSPTimes++;
						}
					}
				}
			}
			else if(KIout<0)
			{
				if(ErrAbs<((State.PID.Over*35)/10))
				{
					if(ABSKIOut>=((PidOut+ABSKIOut)/100))			 //	KIout<0,但其绝对值大于PID输出的1/20
					{
						MSPTimes--;
						if(State.Basic.DiaOK==0)		//卷径未算出
						{
							MSPTimes--;
						}
						if(ABSKIOut>=((PidOut+ABSKIOut)/12))
						{
						  	MSPTimes--;
							if(State.Basic.DiaOK==0)		//卷径未算出
							{
								MSPTimes--;
							}
						}
						if(ABSKIOut>=((PidOut+ABSKIOut)/8))
						{
						  	MSPTimes--;
							if(State.Basic.DiaOK==0)		//卷径未算出
							{
								MSPTimes--;
							}
						}
						if(ABSKIOut>=((PidOut+ABSKIOut)/4))
						{
						  	MSPTimes--;	
							if(State.Basic.DiaOK==0)		//卷径未算出
							{
								MSPTimes--;
							}
						}
						if(ABSKIOut>=((PidOut+ABSKIOut)/2))
						{
						  	MSPTimes--;
							if(State.Basic.DiaOK==0)		//卷径未算出
							{
								MSPTimes--;
							}
						}
					}
				}
			}
		}
		if(MSPTimes>108)	 //积分大于0比较
		{
	//		if((MSPADD<103)&&(MSPADD>97))	  //匀速度时
			{
	//			if(MSpeedP<(Param.Basic.MaxMainSpeedPor*10))		  //已达最大主速度比例
				{
					MSpeedP++;
				}
			}
			MSPTimes=100;
		}
		else if(MSPTimes<92)
		{
	//		if((MSPADD<103)&&(MSPADD>97))	  //匀速度时
			{
	//			if(MSpeedP>(Param.Basic.MaxMainSpeedPor/100))		  //最小30/1000主速度比例
				{
					MSpeedP--;
				}
			}
			MSPTimes=100;
		}
		if(State.Work.MSpeed<=10000);          //主速度10V以内
		{
		    if((Param.Dai.Way==0)||(Param.Dai.Way==4))		//不计算卷径以及模拟量比值法	
			{
				if(Param.Basic.Out==2)				//速度输出状态
				{
					DiaOpenOut=(MSpeedP*((LMS[4]+LMS[3]+LMS[2]+LMS[1]+LMS[0])/5))/1000;	   //匀速度时，做速度的滤波，使输出稳定
					if(Param.Basic.MaxMainSpeedPor>0)
					{
						Pid.IFull=((LMS[4]+LMS[3]+LMS[2]+LMS[1]+LMS[0])/5)*Param.Basic.MaxMainSpeedPor/100;
						Pid.IFull=Pid.IFull+Pid.IFull/20;			//105%	 ,适当大一些，消初误差
					}
					if(Param.Dai.Way==4)
					{
						if(ErrAbs<(State.PID.Over*3))	   //误差在超调区以内
						{
							if(ABSKIOut<=(DiaOpenOut/5))	 //积分输出较小
							{
						//		MSpeedP=(Param.Basic.MaxMainSpeedPor*Param.Dai.LeastDia*10)/State.Work.Dia;	   //初始值为最小卷
								State.Basic.DiaOK=1;
								if(State.Basic.OutCh==0)			//A轴输出模式
								{
								 	State.Work.Dia=(Param.Basic.MaxMainSpeedPor*Param.Dai.LeastDia*10)/MSpeedP;
									if(Param.Dai.MostDia>Param.Dai.LeastDia)
									{
										State.Work.DiaPer=((State.Work.Dia-Param.Dai.LeastDia)*1000)/(Param.Dai.MostDia-Param.Dai.LeastDia);		//	 1/1000
									}
								}
								else		   //B轴输出
								{
								 	State.Work.Dia2=(Param.Basic.MaxMainSpeedPor*Param.Dai.LeastDia*10)/MSpeedP;
									if(Param.Dai.MostDia>Param.Dai.LeastDia)
									{
										State.Work.DiaPer2=((State.Work.Dia2-Param.Dai.LeastDia)*1000)/(Param.Dai.MostDia-Param.Dai.LeastDia);		//	 1/1000
									}
								}
							}
						}
					}
				}
			}
			else if((Param.Dai.Way==1)||(Param.Dai.Way==2)||(Param.Dai.Way==3)||(Param.Dai.Way==5)||(Param.Dai.Way==6))		//计算卷径
			{
				DiaOpenOut=((LMS[4]+LMS[3]+LMS[2]+LMS[1]+LMS[0])/5)*Param.Basic.MaxMainSpeedPor/100;	 //卷径最小叶对应主速度的输出值
				if(Param.Basic.MaxMainSpeedPor>0)
				{
					Pid.IFull=DiaOpenOut+DiaOpenOut/10;				//110%	 ,适当大一些，消初误差
				}
				if(State.Basic.OutCh==0)			//A轴输出模式
				{
					if((State.Work.Dia>0)&&(State.Work.Dia>= Param.Dai.LeastDia))
					{
						DiaOpenOut=(DiaOpenOut*Param.Dai.LeastDia)/State.Work.Dia; //根据卷径大小算出来的开环输出值
					}
				}
				else  if(State.Basic.OutCh==1)			//B轴输出模式
				{
 					if((State.Work.Dia2>0)&&(State.Work.Dia2>= Param.Dai.LeastDia))
					{
						DiaOpenOut=(DiaOpenOut*Param.Dai.LeastDia)/State.Work.Dia2; //根据卷径大小算出来的开环输出值
					}
				}
				DiaOpenOut=DiaOpenOut* MSpeedP/1000;
				if((MSpeedP>1250)||	(MSpeedP<750))	 //误差在25%以外，卷径设置错误
				{
				 	State.Work.DIAERR=1 ;
				}
				else
				{
				  	State.Work.DIAERR=0;
				}
			}
		}
		if(	Pid.IFull>State.PID.MostOut)
		{
			Pid.IFull=State.PID.MostOut;		  //积分饱和=最大输出值
		}
		
//*********************************速度前馈计算***************************************************************
		if((MSPADD<103)&&(MSPADD>97))	  //匀速度时
		{
		}
		else if(MSPADD>=103)
		{
			 DiaOpenOut=DiaOpenOut+((Param.Basic.MSPAF*(LMS[4]-LMS[0]))/100);		
		}	//	                速度前馈百分比   加速度值					 加速系数
		else if(MSPADD<=97)
		{
			DiaOpenOut=DiaOpenOut-((Param.Basic.MSPAF*(LMS[0]-LMS[4]))/100);		
		}	//				主速度给定                速度前馈百分比   加速度值					 加速系数
	}
	else if(Param.Basic.Out==1)				//扭矩输出状态
	{
	 	DiaOpenOut=0;
		//*********************************速度前馈计算***************************************************************
		if((MSPADD<103)&&(MSPADD>97))	  //匀速度时
		{

		}
		else if(MSPADD>=103)
		{
				
		}	//	                速度前馈百分比   加速度值					 加速系数
		else if(MSPADD<=97)
		{
			
		}
	}

	if(Param.Dai.Way==0)		//不计算卷径
	{
		KPDS=1+(((State.Work.MSpeed/10000)*Pid.KPV)/100);
	}
	else
	{
		KPDS=1+(((State.Work.MSpeed/10000)*Pid.KPV)/100);		   
	}

	KPDS=300+(((State.Work.MSpeed/10000)*Pid.KPV)/100);
	KPSL= KPDS;
 /*
	if(KPout>=0)
	{
		KPout=KPout*KPSL/300;
	}
	else
	{
	  	KPout=0-(((0-KPout)*KPSL)/300);
	}
	*/
	if(KIout>=0)
	{
   		if(KIout>Pid.IFull)
		{
			KIout=Pid.IFull;
			Pid.SumError= KIout;
		}
	}
	else
	{
   		if((0-KIout)>Pid.IFull)
		{
			KIout=0-Pid.IFull;
			Pid.SumError= KIout;
		}
	}
	 if(Param.Basic.Out==1)			//扭矩输出状态
	 {
		if(KIout<Pid.IZero)
		{
		  	KIout=Pid.IZero;
			Pid.SumError=KIout;
		}
	 }
	KIout=KIout;
    if(Param.Basic.Out==1)				//扭矩输出状态
	{
    	PidOut=KPout+KDout+KIout;
	}
	else if(Param.Basic.Out==2)				//速度输出状态
	{
		if(Param.Basic.MaxMainSpeedPor>0)
		{
			PidOut=KPout+KDout+KIout+DiaOpenOut;
		}
		else
		{
     		PidOut=KPout+KDout+KIout;
		}
	}	

	if((PidOut<0x7fffffff)&(PidOut>0))
	{
		if(PidOut>(Pid.IFull+200))
		{
			PidOut=Pid.IFull+200;
		}
	}
	else
	{
		PidOut=0;
	}
	if(PidOut<State.PID.LeastOut)
	{
		PidOut=State.PID.LeastOut;
	}
	KernelRegs[95]=KPout;
 	KernelRegs[96]=KIout;
  	KernelRegs[97]=KDout;
//	KernelRegs[93]=ErrAbs;
//	KernelRegs[94] =MSPTimes;
	KernelRegs[98]=MSpeedP;
//	KernelRegs[99]=MSpeedP;
	KernelRegs[89]=	Error/10;
//	KernelRegs[90]=	DDD;
	return (PidOut);
}


void  PID_StateSave()
{
}

void  PID_StateRead()
{
u16 Dat1;
u8 PidStateBuf[10];
//	BSP_EEPROMReadByte( PidStateBuf,10,400, 0x00A0);
    if(PidStateBuf[1]==(~PidStateBuf[3]))
	{
		Dat1=PidStateBuf[1];
	}
	else
	{
	}
    if(PidStateBuf[0]==(~PidStateBuf[2]))
	{
		MSpeedP=Dat1+(PidStateBuf[0]<<8);
	}
	else
	{
	}

}

void PID_StateInit()
{
  	if((Param.Dai.Way==0))		//不计算卷径以及模拟量比值法
	{
	 	MSpeedP=Param.Basic.MaxMainSpeedPor*4;	   //初始值为最小卷径时主速度的40%
	}
	else if((Param.Dai.Way==4)||(Param.Dai.Way==3))		   //模拟量比值法和迈冲比值法时
	{
		if(State.Basic.OutCh==0)			//A轴输出模式
		{
			if((State.Work.Dia>0)&(State.Work.Dia>=Param.Dai.LeastDia))
			{
	 	 		MSpeedP=(Param.Basic.MaxMainSpeedPor*Param.Dai.LeastDia*10)/State.Work.Dia;	   //初始值为最小卷径时主速度的40%
			}
			else
			{
				if(Param.Basic.MachMode==1)			//放卷
				{									//放卷时快一些
	 		 		MSpeedP=Param.Basic.MaxMainSpeedPor*6;	   //初始值为最小卷径时主速度的60%
				}
				else							   //收卷时慢一些
				{
	 		 		MSpeedP=Param.Basic.MaxMainSpeedPor*4;	   //初始值为最小卷径时主速度的40%
				}
			}
		}
		else if(State.Basic.OutCh==1)			//B轴输出模式
		{
			if((State.Work.Dia2>0)&(State.Work.Dia2>=Param.Dai.LeastDia))
			{
	 	 		MSpeedP=(Param.Basic.MaxMainSpeedPor*Param.Dai.LeastDia*10)/State.Work.Dia2;	   //初始值为最小卷径时主速度的40%
			}
			else
			{
				if(Param.Basic.MachMode==1)			//放卷
				{									//放卷时快一些
	 		 		MSpeedP=Param.Basic.MaxMainSpeedPor*6;	   //初始值为最小卷径时主速度的60%
				}
				else							   //收卷时慢一些
				{
	 		 		MSpeedP=Param.Basic.MaxMainSpeedPor*4;	   //初始值为最小卷径时主速度的40%
				}
			}
		}
	}
	else
	{
		MSpeedP=1000;
	}
	RUNState.No=0;	 			//用于统计误差的数组计数。
	RUNState.UseNo=0;
//	 PID_StateSave();
}

