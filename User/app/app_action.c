
/*++++++++++++++++++++++++++++++++++++++
Action
主循环
邱毅
IO端口，模拟量端口，通信参数   启停、点动、换轴、滤波等功能
 +++++++++++++++++++++++++++++++++++++++*/
#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"
#include "app_action.h"
#include "app_inout.h"
#include "bsp_dac.h"
#include "bsp_adc.h"
#include "app_kernelregs.h"
#include "bsp_led.h"
#include "bsp_enc.h"
#include "app_Port.h"


//开始运行
void StartAction()
{
	if(Param.Basic.Out==1)			//扭矩模式
	{
		State.Action.Start=0;
		State.Action.StartFlg=1;					//经过延时之后转为PID自动输出
		State.Action.Stop=0;						//如果在停机、停止停机
		State.Action.StopFlg=0;
		State.Basic.Run=1;
		RunLedOn
		if(State.Basic.OutCh==0)			
		{
			State.Out.Mode=3;							//自动运行预输出模式
		}
		else
		{
			State.Out.Mode2=3;	
		}
		InitPIDOut();				//PID初始化，使PID输出的I输出(积分），等于当前的输出，速度模式下没有，扭矩下必须的
		State.Basic.DisMode=0;		//刷新界面
	}
	else if(Param.Basic.Out==2)				//速度模式
	{
		State.Basic.Run=2;			//标准PID启动
		RunLedOn

		if(State.Basic.OutCh==0)	//A轴		
		{
			State.Out.Mode=1;     //PID自动控制输出模式
		}
		else						//B轴
		{
		 	State.Out.Mode2=1;     //PID自动控制输出模式
		}
		PID_StateInit();   //速度模式，对输出速度与主速度之间的比值做初始化
	}
	KernelRegs[66]=1;	//断点记忆
	RegsFlag[66] =1;  	//	EEPROM SAVE
}											
	
//停止运行
void StopAction()
{
	if(Param.Basic.Out==1)			//扭矩模式
	{
		State.Action.Stop=0;
		State.Basic.DisMode=0;		//刷新界面
		if(State.Basic.Run==1)		//启动时
		{
			State.Action.Start=0;
			State.Action.StartFlg=0;					//
			State.Basic.Run=0;
			RunLedOff
			if(State.Basic.OutCh==0)			//A轴输出模式
			{
				State.Out.Mode2=0;
				if(State.Basic.OutSave==0)
				{
					State.Out.Mode=8;		//开机预输出
				}
				else
				{
					State.Out.Mode=9;		//开机记忆预输出
				}
			}
			else 	if(State.Basic.OutCh==1) 	//b轴输出模式
			{
				if(State.Basic.OutSave==0)
				{
					State.Out.Mode2=8;		//开机预输出
				}
				else
				{
					State.Out.Mode2=9;		//开机记忆预输出	
				}
				State.Out.Mode=0;
			}
		}
		else if((State.Basic.Run==2)||(State.Basic.Run==4)||(State.Basic.Run==5))			//PID运行时,加速度PID 减速度PID
		{
			State.Action.StopFlg=1;
			State.Basic.PidInit=0;
			if(State.Basic.OutSave==1)			//输出记忆
			{
				if(State.Basic.OutCh==0)			
				{
					Param.Basic.SaveOut=State.Out.Data;		//自动保存输出
				}
				else
				{
					Param.Basic.SaveOut=State.Out.Data2;		//自动保存输出
				}
				//huzhiwen comment SaveSaveOut();
			}
			State.Basic.Run=3;		//停机PID
		}
	}
	else if(Param.Basic.Out==2)			//速度模式
	{
		State.Action.StopFlg=1;
		State.Basic.Run=0;			//已停机  待机状态
		RunLedOff
		State.Out.Mode=0;     //A轴不输出
		State.Out.Mode2=0;     //B轴不输出
//		State.Basic.DisMode=0;		//刷新界面
		if(Param.Basic.AutoClearDia==1)	  //自动清卷径
		{
		 	DaiClear();	  //卷径数据清0
		}
		else if(Param.Basic.AutoClearDia==2)	  //  自动保存卷径
		{
			DiaSave();
		}
	}
 	KernelRegs[66]=0;	//断点记忆清0
	RegsFlag[66] =1;  	//	EEPROM SAVE
}
//点动运行
void JogAction()
{
	if(State.Basic.Run==0)				//仅能在待机时有效
	{
		if(State.Basic.OutCh==0)			//通道1	
		{
				State.Out.Mode=5;					//点动输出模式
				State.Basic.Jog=1;
		}
		else							 //通道2
		{

				State.Out.Mode2=5;				//点动输出模式
				State.Basic.Jog=1;
		}
		State.Basic.DisMode=0;		//刷新界面
		JOGLedOn
	}
}

void JogStop()	  //点动停止
{
	if(State.Basic.Run==0)				//仅能在待机时有效
	{
			if(State.Basic.OutCh==0)	//A轴	
			{
				if(State.Out.Mode==5)
				{
					if(Param.Basic.Out==1)			//扭矩模式
					{
						if(State.Basic.OutSave==1)			//输出记忆
						{
							State.Out.Mode=9;				//记忆输出
						}
						else
						{
							State.Out.Mode=8;				//开机预输出模式
						}
					}
					else 		//速度模式
					{
						 State.Out.Mode=0;     //A轴不输出
					}
		//			State.Basic.DisMode=0;		//刷新界面
				}
			}
			else
			{
				if(State.Out.Mode2==5)
				{
					if(Param.Basic.Out==1)			//扭矩模式
				 	{
						if(State.Basic.OutSave==1)			//输出记忆
						{
							State.Out.Mode2=9;				//记忆输出
						}
						else
						{
							State.Out.Mode2=8;				//开机预输出模式
						}
					}
					else  	//速度模式
					{
						 State.Out.Mode2=0;     //B轴不输出
					}
				}
			}
			JOGLedOff
	}
}

void ChangAction(uint S)			//轴切信号
{
	if(S==1)
	{
		if(State.Basic.OutCh==0)			//轴切输出模式A到B
		{
			State.Work.RollDT=0;		//卷轴转数清0
			State.Work.MainDT=0;		//主轴转数清0
			State.Work.DiaPer=0;		//卷径比清0
			State.Work.Dia=0;
			if(Param.Dai.Way==4)		//比值法2   模拟量比值法 
			{
		   		if(Param.Basic.MachMode==1)			//放卷
				{
				   	State.Work.Dia=Param.Dai.MostDia;
				}
				else  if(Param.Basic.MachMode==2)			//收卷
				{
				 	State.Work.Dia=Param.Dai.LeastDia;
				}
			}

			(* BSP_ENC_CNT)=0;
			State.Basic.OutCh=1;   		//B轴输出  
			State.Basic.DisMode=0;		//刷新界面
			if((State.Basic.Run!=0)&&(State.Basic.Run!=3))		//如果在运行状态
			{
				State.Action.BChange=0;	
				State.Action.BChangeFlg=1;		//B轴切输出	 /A TO B   B启动
				State.Action.AChangeStop=0;
				State.Action.AChangeStopFlg=1;	//A轴切制动输出	   A TO B  A制动
				State.Out.Mode=7;				//A轴切制动输出模式	
				State.Out.Mode2=6;				//B轴切输出模式	
			}
			else
			{
				State.Out.Mode2=State.Out.Mode;
				State.Action.AChangeStopFlg=1;	//A轴切制动输出
				State.Action.AChangeStop=0;
				State.Out.Mode=7;

			}	
			if(State.Action.BChangeStopFlg==1)		//刚换为A轴,B轴制动输出中
			{
				State.Action.BChangeStopFlg=0;
				State.Action.BChangeStop=0;
			}
		}
	}
	else 
	{
		if(State.Basic.OutCh==1)			//轴切输出模式B到A
		{
			State.Work.RollDT2=0;		//卷轴转数清0
			State.Work.MainDT=0;		//主轴转数清0
			State.Work.DiaPer2=0;
			State.Work.Dia2=0;
			if(Param.Dai.Way==4)		//比值法2   模拟量比值法 
			{
		   		if(Param.Basic.MachMode==1)			//放卷
				{
				   	State.Work.Dia2=Param.Dai.MostDia;
				}
				else  if(Param.Basic.MachMode==2)			//收卷
				{
				 	State.Work.Dia2=Param.Dai.LeastDia;
				}
			}
			(* BSP_ENC_CNT)=0;
			State.Basic.OutCh=0;
			State.Basic.DisMode=0;		//刷新界面
			if((State.Basic.Run!=0)&&(State.Basic.Run!=3))		//如果在运行状态
			{
				State.Action.AChange=0;	
				State.Action.AChangeFlg=1;		//A轴切输出
				State.Action.BChangeStop=0;
				State.Action.BChangeStopFlg=1;	//B轴切制动输出
				State.Out.Mode=6;				//A轴切输出模式	
				State.Out.Mode2=7;				//B轴切制动输出模式		
			}
			else
			{
				State.Out.Mode=State.Out.Mode2;
				State.Action.BChangeStopFlg=1;	//B轴切制动输出
				State.Out.Mode2=7;
			}

			if(State.Action.AChangeStopFlg==1)		//刚换为B轴,A轴制动输出中
			{
				State.Action.AChangeStopFlg=0;
				State.Action.AChangeStop=0;
			}			
		}
	}
}


void IOJog(uint S)
{
	if(S==1)
	{
		JogAction();
	}
	else 
	{
		State.Basic.Jog=0;
		JogStop();		
	}
}
//A轴输出速度
void RollASpeed()
{
	if(State.Basic.OutCh==0)			//A轴输出模式
	{
		if(Param.Dai.RollAWay==1)	   // A轴脉冲来自于IO口
		{
			State.Work.RollDT++;		//卷轴转数,方法2不处理
		}
		if(Param.Dai.Way==3)		//比值法1
		{
			State.Work.RollDSpeed=State.Work.RollDS*Param.Dai.RollD;		//转一圈的时间
			State.Work.RollDS=0;
		}
	}
}

void RollBSpeed()	  //B轴输出速度
{
	if(State.Basic.OutCh==1)			//B轴输出模式
	{
		if(Param.Dai.RollBWay==1)	   // B轴脉冲来自于IO口
		{
			State.Work.RollDT2++;		//卷轴转数
		}
		if(Param.Dai.Way==3)		//比值法1
		{
			State.Work.RollDSpeed=State.Work.RollDS2*Param.Dai.RollD;
			State.Work.RollDS2=0;
		}
	}
}

void MainSpeed() //主牵引速度
{
	State.Work.MainDT++;		//主轴转数
	if(Param.Dai.Way==3)		//比值法1
	{
		State.Work.MainDSpeed=State.Work.RollDS*Param.Dai.MainD;
		State.Work.MainDS=0;
	}
}

void RunSwitch( uint RunS)	 //运行切换
{
	if(RunS==1)			//如果有信号
	{
		if((State.Basic.Run==0)||(State.Basic.Run==3))	//如果在待机或停机状态
		{							//启动
			StartAction();
		}
	}
	else if(RunS==0)			//如果有信号
	{
		if(State.Basic.Run!=0)		//如果在停止状态
		{							//启动
			StopAction();
		}
	}
}

void  MainMotFR(uint RunS)
{
	if(RunS==0)			//如果有信号  	//正转
	{
		if(State.Basic.Run==0)				//仅能在待机时有效
		{						
			Param.Basic.MMFR=0;				//0：正转，1反转
		}
	}
	else if(RunS==1)			//如果有信号
	{
		if(State.Basic.Run==0)				//仅能在待机时有效
		{
 			Param.Basic.MMFR=1;				//0：正转，1反转
		}
	}
}

void  DaiClear()	  //卷径数据清0
{
	State.Work.Dia=0;
	State.Work.DiaPer=0;
	State.Work.RollDT=0;
	(* BSP_ENC_CNT)=0;
	State.Work.RollDT2=0;		//卷轴转数清0
	State.Work.MainDT=0;		//主轴转数清0
	State.Work.DiaPer2=0;
	State.Work.Dia2=0;
	if(Param.Dai.Way==4)		//比值法2   模拟量比值法 
	{
		 if(Param.Basic.MachMode==1)			//放卷
		{
			State.Work.Dia2=Param.Dai.MostDia;
			State.Work.Dia=Param.Dai.MostDia;
		}
		else  if(Param.Basic.MachMode==2)			//收卷
		{
			State.Work.Dia2=Param.Dai.LeastDia;
			State.Work.Dia=Param.Dai.LeastDia;
		}
	}
	KernelRegs[65]=0;  //
	RegsFlag[65] =1;
}

void DiaSave()
{
	u16 DiaDataSave;
//	if(State.Basic.OutCh==0)  //A轴
	DiaDataSave=1+State.Work.DiaPer/5;	   //把	DiaPer由0～1000转换成0~200,以便能够以8位(0~256）储存

 //   else  if(State.Basic.OutCh==0)  //B轴
	DiaDataSave=DiaDataSave+(((State.Work.DiaPer2/5)+1)<<8);
	KernelRegs[65]=DiaDataSave;
	RegsFlag[65] =1;	 //往EEPROM里写
}

void IOInput()			//IO输入
{
uint8_t PS1,PS2,PS3,PS4,PS5,PS6,PS7,PS8,PS9;

		PS1=(1-GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5));
		PS2=(1-GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_6));
		PS3=(1-GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_7));
		PS4=(1-GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_8));
		PS5=(1-GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_9));
		PS6=(1-GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12));
		PS7=(1-GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6));	 //高速输入口1
		PS8=(1-GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7));	  //高速输入口3
		PS9=(1-GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5));	  //高速输入口3
		Port.DIn.Bits.S7=PS7;
		Port.DIn.Bits.S8=PS8;
		Port.DIn.Bits.S9=PS9;

		if(Port.DIn.Bits.S1!=PS1)		  //运行信号
		{
			Port.DIn.Bits.S1=PS1;			//读信号
			RunSwitch(Port.DIn.Bits.S1);
		}

		if(Port.DIn.Bits.S2!=PS2)		 	//	点动控制
		{
			Port.DIn.Bits.S2=PS2;			//读信号
			IOJog(Port.DIn.Bits.S2);
		}
		if(Port.DIn.Bits.S3!=PS3)		 	//	正反转控制
		{
			Port.DIn.Bits.S3=PS3;			//读信号
			MainMotFR(Port.DIn.Bits.S3);		   	//主输出电机正反转控制
		}
		if(Port.DIn.Bits.S4!=PS4)		  	//	输出选择，0:A轴,1:B轴
		{
			Port.DIn.Bits.S4=PS4;			//读信号
			ChangAction(Port.DIn.Bits.S4);
		}

		if(Port.DIn.Bits.S5!=PS5)	 //主轴脉冲输入
		{
			Port.DIn.Bits.S5=PS5;			//读信号
			if(Param.Dai.RollMWay==1)	   // M轴脉冲来自于IO口5
			{
				if(Param.Dai.Way==3)		//比值法1
				{
					if(Port.DIn.Bits.S5==1)		//计算速度
					{
				//		MainSpeed();
						State.Work.MainDT++;		//主轴脉冲数			
					}
				}
			}
			else if(Param.Dai.RollBWay==1)	   // B轴脉冲来自于IO口5
			{
				if(State.Basic.OutCh==1)			//B轴输出模式
				{
					if(Param.Dai.Way==2)		//卷料厚度累加法
					{

						if(Port.DIn.Bits.S5==1)		//脉冲数+1
						{
							State.Work.RollDT2=State.Work.RollDT2+1;
						}
					}
					if(Param.Dai.Way==3)		//比值法1
					{
						if(Port.DIn.Bits.S5==1)		//计算速度
						{
						//	RollBSpeed();
							State.Work.RollDT2=State.Work.RollDT2+1;
						}			
					}
				}
			}		
		}
		if(Port.DIn.Bits.S6!=PS6)  //卷轴脉冲输入
		{
			Port.DIn.Bits.S6=PS6;			//读信号

			if(Param.Dai.RollAWay==1)	   // A轴脉冲来自于IO口6
			{
				if(State.Basic.OutCh==0)			//A轴输出模式
				{
					if(Param.Dai.Way==3)		//比值法1
					{
						if(Port.DIn.Bits.S6==1)		//计算速度
						{
						//	RollASpeed();
							State.Work.RollDT=State.Work.RollDT+1;
						}			
					}
					if(Param.Dai.Way==2)		//卷料厚度累加法
					{
						if(Port.DIn.Bits.S6==1)		//脉冲圈数加1
						{
							State.Work.RollDT=State.Work.RollDT+1;
						}
					}
				}
			}		
		}
}



