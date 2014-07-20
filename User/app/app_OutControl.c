
/*输出流程控制
邱毅
2012 03 27
*/
#include <ucos_ii.h>
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "bsp.h"
#include "app_basicdef.h"
#include "app_pid.h"
#include "app_action.h"
#include "app_inout.h"
#include "app_kernelregs.h"
#include "app_OutControl.h"
#include "bsp_enc.h"
#include "app_Port.h"


#define LedOn       GPIO_ResetBits(GPIOE, GPIO_Pin_2);
#define LedOff       GPIO_SetBits(GPIOE, GPIO_Pin_2);

extern OS_EVENT *Tim5Msg; //定时器5处理通知

int aa,bb,cc;
uint a;
uint OldOutA,OldOutB;
 long LongData;
u16  JogTimes=0;		//点动时间，点动时梯型升降速时用
u16  DiaTiems=0;		//脉冲比值法算卷径时，计时用
u16  MainSpeedTimes=0;		//主轴速度来自于脉冲时，计时用

u32 RunTimes;	//运行时间，欠张力检测用

 u16 Leda;
void OutControl()
{
		u8 err;
   		//等待通知
   		OSSemPend(Tim5Msg,0,&err);
//		if(State.Basic.PIDCycFLG==1)			//已到PID控制周期
		{		
			for(aa=1;aa<10;aa++)
			{
				bb=aa-1;
				OUTDATA[bb]=OUTDATA[aa];				//A输出替换
				OUTDATB[bb]=OUTDATB[aa];				//B输出替换
			}
//////////////////////////////卷径计算//////////////////////////////////////////////
			if(Param.Dai.Way==0)		//不计算卷径
			{
				State.Work.Dia=0;
				State.Work.DiaPer=0;
				State.Work.Dia2=0;
				State.Work.DiaPer2=0;
			}
			else if(Param.Dai.Way==1)		//来自于模拟量端口信号的百分比
			{
				if(State.Basic.OutCh==0)			//A轴输出模式
				{
					if(Param.Basic.MachMode==1)				//放卷
					{
						LongData=(Param.Dai.MostDia-Param.Dai.LeastDia)*State.Work.DiaPer;	//	DAI=原始卷径＋（最大卷径－原始卷径）×当前卷径百分比	
					}
					else  if(Param.Basic.MachMode==2)				//放卷
					{
						LongData=(Param.Dai.MostDia-Param.Dai.LeastDia)*State.Work.DiaPer;	//	DAI=原始卷径＋（最大卷径－原始卷径）×当前卷径百分比	
					}
					State.Work.Dia=Param.Dai.LeastDia+LongData/1000;
				}
				else
				{
					if(Param.Basic.MachMode==1)				//放卷
					{
						LongData=(Param.Dai.MostDia-Param.Dai.LeastDia)*State.Work.DiaPer2;	//	DAI=原始卷径＋（最大卷径－原始卷径）×当前卷径百分比	
					}
					else  if(Param.Basic.MachMode==2)				//放卷
					{
						LongData=(Param.Dai.MostDia-Param.Dai.LeastDia)*State.Work.DiaPer2;	//	DAI=原始卷径＋（最大卷径－原始卷径）×当前卷径百分比	
					}
					State.Work.Dia2=Param.Dai.LeastDia+LongData/1000;
				}
			}
			else if(Param.Dai.Way==2)		//卷料厚度累加法
			{	
				if(State.Basic.OutCh==0)			//A轴输出模式
				{
					if(Param.Dai.RollAWay==1)	   // A轴脉冲来自于IO口6
					{
												  	//脉冲数已在IO口处处理
					}
					else if(Param.Dai.RollAWay==2)	   // A轴脉冲来自于2：编码器口
					{
						if((* BSP_ENC_CNT)>=0)
						{
							State.Work.RollDT= 	State.Work.RollDT+(* BSP_ENC_CNT);
						}
						else
						{
							State.Work.RollDT= State.Work.RollDT+(0-(* BSP_ENC_CNT));
						}
					}
					LongData=(State.Work.RollDT)/Param.Dai.RollD;		//卷轴转数
					if(Param.Basic.MachMode==1)				//放卷
					{
						State.Work.Dia=Param.Dai.MostDia-(LongData*Param.Dai.Ply*2)/100;	   //卷径单位(0.1mm）=卷料厚度（um）/100 
						LongData=(State.Work.Dia-Param.Dai.LeastDia)*1000;
						State.Work.DiaPer=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);		//	 1/1000
					}
					else if(Param.Basic.MachMode==2)				//收卷
					{
						State.Work.Dia=Param.Dai.LeastDia+(LongData*Param.Dai.Ply*2)/100;			//,直径乘以2，卷径的单位是0。1mm，厚度是0，01mm，再除10
						LongData=(State.Work.Dia-Param.Dai.LeastDia)*1000;
						State.Work.DiaPer=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);
					}
				}
				else if(State.Basic.OutCh==1)			//B轴输出模式
				{
					if(Param.Dai.RollBWay==1)	   // B轴脉冲来自于IO口6
					{
														//脉冲数已在IO口处处理
					}
					else if(Param.Dai.RollBWay==2)	   // B轴脉冲来自于2：编码器口
					{
						if((* BSP_ENC_CNT)>=0)
						{
							State.Work.RollDT2=State.Work.RollDT2+ (* BSP_ENC_CNT);
						}
						else
						{
							State.Work.RollDT2= State.Work.RollDT2+(0-(* BSP_ENC_CNT));
						}
					}						 	//	DAI=原始卷径＋（卷轴脉冲数/卷径每转脉冲数）×卷径厚度
						LongData=State.Work.RollDT2/Param.Dai.RollD;		//卷轴转数，乘100以去小数
						if(Param.Basic.MachMode==1)				//放卷
						{
							State.Work.Dia2=Param.Dai.MostDia-(LongData*Param.Dai.Ply*2)/100;	   //1000=100*10= 转数*100   卷径单位=卷径单位/10
							LongData=(State.Work.Dia2-Param.Dai.LeastDia)*1000;
							State.Work.DiaPer2=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);		//	 1/1000
						}
						else if(Param.Basic.MachMode==2)				//收卷									   
						{
							State.Work.Dia2=Param.Dai.LeastDia+(LongData*Param.Dai.Ply*2)/100;			//,直径乘以2，卷径的单位是0。1mm，厚度是0，01mm，再除10，加上上一行的乘了100倍，总除1000
							LongData=(State.Work.Dia2-Param.Dai.LeastDia)*1000;
							State.Work.DiaPer2=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);
						}
				}											
			}
			else if(Param.Dai.Way==3)		//比值法1
			{													//	DAI=主轴角速度×主轴半径/卷轴角速度   卷径×卷轴角速度=主轴角速度×主轴半径
																//State.Work.MainDSpeed:主轴转一圈的时间，角速度的倒数
																//State.Work.RollSpeed:卷轴转一圈的时间，角速度的倒数
																//  Dai/State.Work.RollSpeed=Param.Dai.MainD/State.Work.MainDSpeed
																//  Dai=（Param.Dai.MainD/State.Work.MainDSpeed）×State.Work.RollSpeed
																//	   =(State.Work.RollSpeed/State.Work.MainDSpeed)*Param.Dai.MainD 
				if(DiaTiems<10000)	   //50秒计算一次
				{
				  	DiaTiems++;
				}
				else
				{
				 	DiaTiems=0;		
					  //先计算主轴角速度	 ,50秒转的圈数
					if(Param.Basic.MainSpeedWay==3)	  			//   主速度来自于3：来自于脉冲信号
					{
						if(Param.Dai.RollMWay==1)	   // M轴脉冲来自于IO口5
						{
							State.Work.MainDSpeed=((State.Work.MainDT-State.Work.MainDS)*10);	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
							State.Work.MainDS= State.Work.MainDT;
						}
						else if(Param.Dai.RollMWay==2)	   // M轴脉冲来自于  2：编码器口
						{
					   		//State.Work.MainDSpeed=  每秒转的脉冲数 *10

							State.Work.MainDSpeed=(State.Work.MainDSpeed*50)/ Param.Dai.MainD;
								//主速度，每1秒计算一次，这边50秒计算一次，所以要*50
						}
					}
					else
					{
						if(Param.Dai.RollMWay==1)	   // M轴脉冲来自于IO口5
						{
							State.Work.MainDSpeed=((State.Work.MainDT-State.Work.MainDS)*10)/Param.Dai.MainD;	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
							State.Work.MainDS= State.Work.MainDT;
						}
						else if(Param.Dai.RollMWay==2)	   // M轴脉冲来自于  2：编码器口
						{
	
							if((* BSP_ENC_CNT)>=0)
							{
								State.Work.MainDT= 	(* BSP_ENC_CNT)/Param.Dai.MainD;
							}
							else
							{
								State.Work.MainDT=(0-(* BSP_ENC_CNT))/Param.Dai.MainD;
							}
							 //主轴总圈数
							State.Work.MainDSpeed=(State.Work.MainDT-State.Work.MainDS)*10;	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
							State.Work.MainDS= State.Work.MainDT;
						}
					}

					if(State.Basic.OutCh==0)			//A轴输出模式
					{	
						if(Param.Dai.RollAWay==1)	   // A轴脉冲来自于IO口6
						{						 		//计算A轴角速度	
							State.Work.RollDSpeed= ((State.Work.RollDT-State.Work.RollDS)*10)/Param.Dai.RollD ;
							State.Work.RollDS= State.Work.RollDT;
						}
						else if(Param.Dai.RollAWay==2)	   // A轴脉冲来自于2：编码器口
						{
							if((* BSP_ENC_CNT)>=0)
							{
								State.Work.RollDT= (* BSP_ENC_CNT)/Param.Dai.RollD;
							}
							else
							{
								State.Work.RollDT=(0-(* BSP_ENC_CNT))/Param.Dai.RollD;
							}
  						 	//A轴总圈数
							State.Work.RollDSpeed=(State.Work.RollDT-State.Work.RollDS)*10;	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
							State.Work.RollDS= State.Work.RollDT;	   //DT  DS
						}
						LongData=State.Work.MainDSpeed*Param.Dai.Ratio;		//	 Param.Dai.Ratio 主轴直径
						//线速度=主轴角速度*  主轴直径
						State.Work.Dia = LongData/State.Work.RollDSpeed;
						//卷 径=线速度/卷轴角速度
						LongData=State.Work.Dia*1000;

						State.Work.DiaPer=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);

					}
					else if(State.Basic.OutCh==1)			//B轴输出模式
					{							 		//计算B轴角速度	
						if(Param.Dai.RollBWay==1)	   // A轴脉冲来自于IO口6
						{
							State.Work.RollD2Speed= ((State.Work.RollDT2-State.Work.RollDS2)*10)/Param.Dai.RollD ;
							State.Work.RollDS2= State.Work.RollDT2;
						}
						else  if(Param.Dai.RollBWay==2)		  // B轴脉冲来自于2：编码器口
						{
 							if((* BSP_ENC_CNT)>=0)
							{
								State.Work.RollDT2= (* BSP_ENC_CNT)/Param.Dai.RollD;
							}
							else
							{
								State.Work.RollDT2= (0-(* BSP_ENC_CNT))/Param.Dai.RollD;
							}
  						 	//B轴总圈数
							State.Work.RollD2Speed=(State.Work.RollDT2-State.Work.RollDS2)*10;	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
							State.Work.RollDS2= State.Work.RollDT2;
						}
						LongData=State.Work.MainDSpeed*Param.Dai.Ratio;		//	 Param.Dai.Ratio 主轴直径
						//线速度=主轴角速度*  主轴直径
						State.Work.Dia2 = LongData/State.Work.RollD2Speed;
						//卷 径=线速度/卷轴角速度
						LongData=State.Work.Dia2*1000;

						State.Work.DiaPer2=LongData/(Param.Dai.MostDia-Param.Dai.LeastDia);
					}
				}
         
			}
			else if(Param.Dai.Way==4)		//比值法2   模拟量比值法  //在PID的程序中已经计算
			{													//	DAI=主轴角速度×主轴半径/卷轴角速度   卷径×卷轴角速度=主轴角速度×主轴半径
																//State.Work.MSpeed:主轴角速百分比
																//State.Work.RSpeed:卷轴角速百分比
																//  Dai=（State.Work.MSpeed/State.Work.RSpeed）×State.Work.RollSpeed
																//	   =(State.Work.MSpeed/State.Work.MainDSpeed)*State.Work.RSpeed
			}
			else if(Param.Dai.Way==5)		//5：来自超声波传感器  暂不支持
			{
			}
			else if(Param.Dai.Way==6)	   //6：6: 来自通信  KernelRegs[64]在  通过参数程序中已实现
			{
			}
//////////////////////////////锥度计算///////////////////////////////////////////////
			if(Param.Basic.MachMode==1)				//放卷锥度    无锥度 恒张力控制
			{
				Pid.SetD=Param.Auto.SetAim;
	//			Param.Basic.ZDLine=0;
			}
			else if(Param.Basic.MachMode==2)			//收卷锥度  张力越来越小			
			{
				if(Param.Basic.ZDLine==0)			//恒张力控制
				{
					Pid.SetD=Param.Auto.SetAim;					
				}
				if(Param.Basic.ZDLine==1)			//直线锥度
				{
					if(State.Basic.OutCh==0)			//A轴输出模式	
					{
					 	LongData=State.Work.DiaPer;
					}
					else if(State.Basic.OutCh==1)			//B轴输出模式	
					{
						LongData=State.Work.DiaPer2;
					}

					if(LongData<=100)		//0~10%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ10+((100-Param.ZDZ.ZDZ10)*(100-LongData))/100))/100;
					}
					else if(LongData<=200)		//10~20%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ20+((Param.ZDZ.ZDZ10-Param.ZDZ.ZDZ20)*(200-LongData))/100))/100;
					}
					else if(LongData<=300)		//20~30%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ30+((Param.ZDZ.ZDZ20-Param.ZDZ.ZDZ30)*(300-LongData))/100))/100;
					}
					else if(LongData<=400)		//30~40%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ40+((Param.ZDZ.ZDZ30-Param.ZDZ.ZDZ40)*(400-LongData))/100))/100;
					}
					else if(LongData<=500)		//40~50%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ50+((Param.ZDZ.ZDZ40-Param.ZDZ.ZDZ50)*(400-LongData))/100))/100;
					}
					else if(LongData<=600)		//50~60%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ60+((Param.ZDZ.ZDZ50-Param.ZDZ.ZDZ60)*(400-LongData))/100))/100;
					}
					else if(LongData<=700)		//60~70%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ70+((Param.ZDZ.ZDZ60-Param.ZDZ.ZDZ70)*(400-LongData))/100))/100;
					}
					else if(LongData<=800)		//70~80%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ80+((Param.ZDZ.ZDZ70-Param.ZDZ.ZDZ80)*(400-LongData))/100))/100;
					} 
					else if(LongData<=900)		//80~90%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ90+((Param.ZDZ.ZDZ80-Param.ZDZ.ZDZ90)*(400-LongData))/100))/100;
					}
					else if(LongData<=1000)		//90~100%以内
					{
						Pid.SetD=(Param.Auto.SetAim*(Param.ZDZ.ZDZ100+((Param.ZDZ.ZDZ90-Param.ZDZ.ZDZ100)*(400-LongData))/100))/100;
					}
					else
					{
						Pid.SetD=( Param.Auto.SetAim*Param.ZDZ.ZDZ100)/100;
					}
				}
			}
			else if(Param.Basic.MachMode==3)			//牵引   恒张力控制
			{
				Pid.SetD=Param.Auto.SetAim;
			}
////////////////////////////////////////////////////////////////////////////////////

			if(State.Action.StopFlg==2)
			{
  				State.Action.StopFlg=0;
//				PID_StateSave();	  //停机后，保存PID数据
			}
			if(State.Out.Mode==0)						////开机预输出模式
			{
				OUTDATA[9]=0;
				if(State.Basic.OutCh==0)  //A轴
				{
					if(JogTimes>0)
					{
						LongData=10000;
						LongData=Param.Auto.JogOut*LongData;
						LongData=LongData/100;
					 	OUTDATA[9]=JogTimes*LongData/1000;
						JogTimes-- ;
						if(JogTimes>0)
						{
							JogTimes-- ;
						}
						if(JogTimes>0)
						{
							JogTimes-- ;
						}
					}
				}			
			}
			else if(State.Out.Mode==1)						////自动控制输出模式
			{
				if(State.Basic.PIDCycFLG==1)			//已到PID控制周期
				{
					if(State.Basic.Run==2)			//正常PID
					{
						OUTDATA[9]=PIDOut(State.Work.Tens);
					}
					else if(State.Basic.Run==3)			//停机PID	  扭矩
					{
						LongData=State.Out.Data;
						LongData=LongData*Param.Auto.StopOut;
						LongData=LongData/1000;
						if(State.Basic.PidInit==0)
						{
							InitPIDOut();
							State.Basic.PidInit=1;
						}
						LongData=PIDOut(State.Work.Tens);
						OUTDATA[9]=LongData;
					}
				}
			}
			else if(State.Out.Mode==2)				//手动输出模式
			{
			 	if((KernelRegs[77]>1000)&&(KernelRegs[77]<=2000))
				{
					OUTDATA[9]=	KernelRegs[77]*10;
				}
			}
			else if(State.Out.Mode==3)				//启动预输出模式
			{
				LongData=10000;
				LongData=Param.Auto.OpenOut*LongData;		//开机输出
				LongData=LongData/100;
				LongData=Param.Auto.StartOut*LongData;		//启动输出＝开机输出×启动增益
				LongData=LongData/100;
				OUTDATA[9]=LongData;
			}
/*			else if(State.Out.Mode==4)				//停机预输出模式
			{
			}
*/
			else if(State.Out.Mode==5)				//点动输出模式
			{
				LongData=10000;
				LongData=Param.Auto.JogOut*LongData;
				LongData=LongData/100;
				if(JogTimes<=1)		  //5ms*1=5ms, 在500ms以内，慢速
				{
				 	LongData=LongData/10;
					JogTimes++;
				}
				else if(JogTimes<1000)		  //5ms*100=500ms, 在500ms以内，慢速
				{
					if(JogTimes<(Param.Auto.JogOut*9))
					{
				 		LongData=LongData/10+JogTimes*10;
					}
					JogTimes++;
				}
				else
				{
					LongData=LongData;
				}

				OUTDATA[9]=LongData;
			}
			else if(State.Out.Mode==6)				//轴切输出模式
			{								   //13632765713  13714858621
				LongData=10000;
				LongData=Param.Auto.ChangeOut*LongData;
				LongData=LongData/100;
				OUTDATA[9]=LongData;
			}
			else if(State.Out.Mode==7)				//轴切制动输出模式
			{
				OUTDATA[9]=OUTDATA[8];
			}
			else if(State.Out.Mode==8)				//开机输出模式
			{
				if(JogTimes>0)
				{
				 	JogTimes=0;
				}
				LongData=10000;
				LongData=Param.Auto.OpenOut*LongData;
				LongData=LongData/100;
				OUTDATA[9]=LongData;
			}	
			else if(State.Out.Mode==9)				//记忆输出模式
			{
				if(JogTimes>0)
				{
				 	JogTimes=0;
				}
			}
			else if(State.Out.Mode==10)				////保持输出模式
			{
				OUTDATA[9]=OUTDATA[8];
			}	

//**************************************************************************
			if(State.Out.Mode2==0)				////不输出模式
			{
				OUTDATB[9]=0;
				if(State.Basic.OutCh==1)  //B轴
				{
					if(JogTimes>0)
					{
						LongData=10000;
						LongData=Param.Auto.JogOut*LongData;
						LongData=LongData/100;
					 	OUTDATA[9]=JogTimes*LongData/1000;
						JogTimes-- ;
						if(JogTimes>0)
						{
							JogTimes-- ;
						}
						if(JogTimes>0)
						{
							JogTimes-- ;
						}
					}
				}
			}
			else if(State.Out.Mode2==1)				////PID输出模式
			{
				if(State.Basic.PIDCycFLG==1)			//已到PID控制周期
				{
					if(State.Basic.Run==2)			//正常PID
					{
						OUTDATB[9]=PIDOut(State.Work.Tens);
					}
					else if(State.Basic.Run==3)			//停机PID
					{
						LongData=State.Out.Data2;
						LongData=LongData*Param.Auto.StopOut;
						LongData=LongData/1000;
						if(State.Basic.PidInit==0)
						{
							InitPIDOut();
							State.Basic.PidInit=1;
						}
						LongData=PIDOut(State.Work.Tens);
						OUTDATB[9]=LongData;
					}
				}
			}
			else if(State.Out.Mode2==2)				//手动输出模式
			{
			 	if((KernelRegs[78]>1000)&&(KernelRegs[78]<=2000))
				{
					OUTDATB[9]=	KernelRegs[78]*10;
				}
			}
			else if(State.Out.Mode2==3)				//启动预输出模式
			{
				LongData=10000;
				LongData=Param.Auto.OpenOut*LongData;		//开机输出
				LongData=LongData/100;
				LongData=Param.Auto.StartOut*LongData;		//启动输出＝开机输出×启动增益
				LongData=LongData/100;
				OUTDATB[9]=LongData;
			}
/*
			else if(State.Out.Mode2==4)				//停机预输出模式
			{
			}
*/
			else if(State.Out.Mode2==5)				//点动输出模式
			{
				LongData=10000;
				LongData=Param.Auto.JogOut*LongData;
				LongData=LongData/100;
				if(JogTimes<=1)		  //5ms*1=5ms, 在500ms以内，慢速
				{
				 	LongData=LongData/10;
					JogTimes++;
				}
				else if(JogTimes<1000)		  //5ms*100=500ms, 在500ms以内，慢速
				{
					if(JogTimes<(Param.Auto.JogOut*9))
					{
				 		LongData=LongData/10+JogTimes*10;
					}
					JogTimes++;
				}
				else
				{
					LongData=LongData;
				}
				OUTDATB[9]=LongData;
			}
			else if(State.Out.Mode2==6)				//轴切输出模式
			{
				LongData=10000;
				LongData=Param.Auto.ChangeOut*LongData;
				LongData=LongData/100;
		
				OUTDATB[9]=LongData;
			}
			else if(State.Out.Mode2==7)				//轴切制动输出模式
			{
				OUTDATB[9]=OUTDATB[8];
			}
			else if(State.Out.Mode2==8)				//开机输出模式
			{
				LongData=10000;
				LongData=Param.Auto.OpenOut*LongData;
				LongData=LongData/100;
				OUTDATB[9]=LongData;
			}
	
			else if(State.Out.Mode2==9)				//记忆输出模式
			{
			}
			else if(State.Out.Mode2==10)				//保持输出模式
			{
				OUTDATB[9]=OUTDATB[8];
			}

			State.Out.Data=OutFilter(1);			//输出滤波

			State.Out.Per=State.Out.Data/100;			//输出百分比
	
			State.Out.Data2=OutFilter(2);			//输出滤波

			State.Out.Per2=LongData/100;			//输出百分比
//			if((OldOutA!=State.Out.Data)||(State.Out.Data2!=OldOutB))//输出值有改变
			{
				if(State.Basic.Out==1)		  //输出允许
				{
					OldOutA=State.Out.Data;
					OldOutB=State.Out.Data2;
				}
				else
				{
					OldOutA=0;
					OldOutB=0;
				}
	//			OutPut();					//卷轴输出
			}
			State.Basic.PIDCycFLG=0;
		}
		if(State.Basic.OverFLG==0)
		{
			if((State.Work.Tens>0)&(Param.Basic.OverTest>0))
			{
				if(State.Work.Tens>Param.Basic.OverTest)			//过张力检测	
				{
					if(State.Action.OverFLG==0)
					{
						State.Action.OverFLG=1;
						State.Action.Over=0;
					}
				}
			}
		}
		else if(State.Basic.OverFLG==1)						//	//过张力时
		{
			if(State.Work.Tens>0)
			{
				if(State.Work.Tens<Param.Basic.OverTest)			//不过张力检测	
				{
					if(State.Action.OverFLG==0)
					{
						State.Action.OverFLG=2;
						State.Action.Over=0;
					}
				}
			}
			else
			{
				if(Param.Basic.OverTest>0)
				{
					if(State.Action.OverFLG==0)
					{
						State.Action.OverFLG=2;
						State.Action.Over=0;
					}
				}
				else if(State.Work.Tens>Param.Basic.OverTest)
				{
					if(State.Action.OverFLG==0)
					{
						State.Action.OverFLG=2;
						State.Action.Over=0;
					}
				}
			}
		}
//***************************欠张力检测****************************************************************
		if(State.Basic.Run!=0)		  //运行状态
		{
	   		RunTimes++;
		}
		else
		{
		 	RunTimes=0;
		}
		if((RunTimes/200)>KernelRegs[5])		  //30秒后，再检测
		{
			if(State.Basic.ZeroFLG==0)
			{
				if((State.Work.Tens>0)&&(Param.Basic.ZeroTest>0))
				{
				 	if(State.Work.Tens<Param.Basic.ZeroTest)		//欠张力检测
					{
						if(State.Action.ZeroFLG==0)
						{
							State.Action.ZeroFLG=1;
							State.Action.Zero=0;
						}
					}
				}
				else if((State.Work.Tens<0)&(Param.Basic.ZeroTest>=0))
				{
					if(State.Action.ZeroFLG==0)
					{
						State.Action.ZeroFLG=1;
						State.Action.Zero=0;
					}
				}
			}
			else if(State.Basic.ZeroFLG==1)
			{
				if((State.Work.Tens>0)&(Param.Basic.ZeroTest>=0))
				{
				 	if(State.Work.Tens>Param.Basic.ZeroTest)		//欠张力检测
					{
						if(State.Action.ZeroFLG==0)
						{
							State.Action.ZeroFLG=2;
							State.Action.Zero=0;
						}
					}
				}
			}
		}
//**************************************************************************
		if((State.Basic.OverFLG==1)||(State.Basic.ZeroFLG==1))
		{
			if(State.Basic.Alarm==0)
			{
				State.Basic.Alarm=1;
//				LedOn(7);
			}
		}
		else
		{
			if(State.Basic.Alarm==1)
			{
				State.Basic.Alarm=0;
//				LedOff(7);
			}
		}
			if(State.Basic.OverFLG==1)				//过张力
			{
//					DOUT1=0;						//端口SO1输出
				GPIO_SetBits(GPIOA, GPIO_Pin_0);	//继电器1输出
			}
			else
			{
				 GPIO_ResetBits(GPIOA, GPIO_Pin_0);		 //继电器1输出高,断开
			}
			if(State.Basic.ZeroFLG==1)         //欠张力
			{
 				GPIO_SetBits(GPIOA, GPIO_Pin_1);	//继电器2输出
			}
			else
			{
 				 GPIO_ResetBits(GPIOA, GPIO_Pin_1);		 //继电器2输出高,断开
			}
			if(Leda<=100)
			{
				
				Leda++;
			}

			else
			{
			 	Leda=0;
				if(cc==0)
				{
//				  	GPIO_SetBits(GPIOA, GPIO_Pin_0);
 					XTLedOn;
				 	cc++;
				}
				else
				{
//				 	GPIO_ResetBits(GPIOA, GPIO_Pin_0);	
					XTLedOff;
				  	cc=0;
				}
			}
			if(MainSpeedTimes<=19)		   //100mS种
			{
			  	MainSpeedTimes++;
			}
			else
			{
				MainSpeedTimes=0;
			   	if(Param.Basic.MainSpeedWay==3)	  			//   主速度来自于3：来自于脉冲信号
				{	
	//				if(Param.Dai.Way!=3)		//比值法1	  上面已处理
					{
	  					if(Param.Dai.RollMWay==1)	   // M轴脉冲来自于IO口5
						{
								//来自于 IO口5 无 效
						}
						else if(Param.Dai.RollMWay==2)	   // M轴脉冲来自于  2：编码器口
						{
	
							if((* BSP_ENC_CNT)>=0)
							{
								State.Work.MainDT= 	(* BSP_ENC_CNT);
							}
							else
							{
								State.Work.MainDT=(0-(* BSP_ENC_CNT));
							}
							 //主轴总圈数
							 if( State.Work.MainDT>=State.Work.MainDS)
							 {
								State.Work.MainDSpeed=(State.Work.MainDT-State.Work.MainDS)*100;	 //   *10是为了去小数，下同 Param.Dai.MainD主轴脉冲数
 								State.Work.MainDS= State.Work.MainDT;									//再乘于10，因为100ms，是 KernelRegs[15]单位的1/10
							}
							else
							{
								State.Work.MainDS= State.Work.MainDT;
								State.Work.MainDSpeed=0;
							}
							
						}	//State.Work.MainDSpeed=  每秒转的脉冲数 （*10）的10倍
						LongData=(State.Work.MainDSpeed*1000);		 //	 State.Work.MSpeed  主速度  0～10000 ，上面已*100，所以只用再乘1000
						State.Work.MSpeed=LongData/KernelRegs[15];
					}
				}
			}
	}
