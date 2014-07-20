/*2012 03 26
定时器初试化
邱毅
资源
T2:用于设置PID周期以及PID中断
*/
#include <ucos_ii.h>
#include "bsp_TimesInt.h"
#include "stm32f10x.h"
#include "app_basicdef.h"
#include "bsp_adc.h"
#include "app_kernelregs.h"
#include "app_pid.h"
#include "bsp_led.h"
#include "app_action.h"

OS_EVENT *Tim4Msg; //定时器4处理通知
OS_EVENT *Tim5Msg; //定时器5处理通知

/*************************************************
函数: void Timer2_Configuration(void)
功能: TIM2 配置
参数: 无
返回: 无
定时计算：(1 /(72 / (36 - 1 + 1))) * 2000 us = 1000us  = 1ms
**************************************************/
void Timer5_Configuration(u32 Tims_MS)
{
//u32 T5_Per;

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  NVIC_InitTypeDef NVIC_InitStructure;


  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x1);          //设置中断向量在FLASH中  
    //配置TIM4中断
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 


  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	    //打开TIM4定时器的时钟
//   T5_Per=2000*Tims_MS;
  TIM_DeInit(TIM5);		                                    //TIMx寄存器重设为缺省值
  
  TIM_TimeBaseStructure.TIM_Period = 10000;		            //自动重装载寄存器周期的值	  5ms  主循环周期
  TIM_TimeBaseStructure.TIM_Prescaler=36 - 1;               //TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //采样分频
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  
  TIM_ARRPreloadConfig(TIM5, ENABLE);                       //允许自动重装载寄存器（ARR）
  TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);	                //允许TIM2溢出中断
  
  TIM_Cmd(TIM5, ENABLE);                                //TIM4开启时钟
  Tim5Msg = OSSemCreate(0); 

}


/*************************************************
函数: void Timer4_Configuration(void)
功能: TIM4 配置
参数: 无
返回: 无
定时计算：(1 /(72 / (36 - 1 + 1))) * 2000 us = 1000us  = 1ms
1ms定时器，应用层使用
**************************************************/
void Timer4_Configuration()
{

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  NVIC_InitTypeDef NVIC_InitStructure;

  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x1);          //设置中断向量在FLASH中  

  //配置TIM4中断
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 


  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	    //打开TIM4定时器的时钟
    
  TIM_DeInit(TIM4);		                                    //TIMx寄存器重设为缺省值
  
  TIM_TimeBaseStructure.TIM_Period = 2000;		            //自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler=36 - 1;               //TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //采样分频
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_ARRPreloadConfig(TIM4, ENABLE);                       //允许自动重装载寄存器（ARR）
  TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);	                //允许TIM2溢出中断
  
  TIM_Cmd(TIM4, ENABLE);                                //TIM4开启时钟

  Tim4Msg = OSSemCreate(0); 

}



/*************************************************
函数: void TIM2_IRQHandler(void)
功能: TIM2中断处理函数
参数: 无
返回: 无
说明：一个周期中断1次
**************************************************/
void TIM5_IRQHandler(void)
{
  static u16 PIDCYC=0;
//  static u16  JJJJJ=0;
  OSIntEnter(); 

   if(TIM_GetITStatus(TIM5,TIM_IT_Update)!=RESET)
   {      
      TIM_ClearITPendingBit(TIM5,TIM_IT_Update);	 //清除中断标志
   }
   State.Basic.T5=1;
	   PIDCYC=PIDCYC+5;
	if(PIDCYC<Pid.CycT)
	{
	   ;

	}
	else
	{
		PIDCYC=0;
   		State.Basic.PIDCycFLG=1;
//		 JOGLedOff		
	}
	    OSSemPost(Tim5Msg);		//发送通知
 /*				//测试代码
		if(JJJJJ<10)
		{
	   JOGLedOn
	   JJJJJ++;
		}
		else  if(JJJJJ<20)
		{
	   	JOGLedOff
		JJJJJ++;
		}
		else
		{
		JJJJJ=0;
		}
	*/

  OSIntExit();

}

void TIM4_ProcessRun(void *p_arg)
{
   	INT8U  err;
	static u32 Tim4s;
	static u16 AdcT;


    u32 ADC_Data=0;
	u32 ADC_Data1=0;
    u32 ADC_Data2=0;
	u32 ADC_Data3=0;
    u16 Abit=0;
	u16 Max;
	u16 Min;

	while(1)
	{
   		//等待通知
   		OSSemPend(Tim4Msg,0,&err);
	ADC_Data=0;
	ADC_Data1=0;
    ADC_Data2=0;
	ADC_Data3=0;
    Abit=0;
	Max=0;
	Min=0;

	   for(Abit=0;Abit<120;Abit=Abit+4)
	   {
		  ADC_Data=ADC_Data+ADCDEVICE.ADCDataTab[Abit];	  
		  ADC_Data1=ADC_Data1+ADCDEVICE.ADCDataTab[Abit+1];	
		  ADC_Data2=ADC_Data2+ADCDEVICE.ADCDataTab[Abit+2];	
		  ADC_Data3=ADC_Data3+ADCDEVICE.ADCDataTab[Abit+3];	  
	   }
	   for(Abit=0;Abit<4;Abit++)
	   {
		 	   State.ADC.Ch1Buf[Abit]=State.ADC.Ch1Buf[Abit+1];
	 	 	   State.ADC.Ch2Buf[Abit]=State.ADC.Ch2Buf[Abit+1];
		 	   State.ADC.Ch3Buf[Abit]=State.ADC.Ch3Buf[Abit+1];
		 	   State.ADC.Ch4Buf[Abit]=State.ADC.Ch4Buf[Abit+1];
	   }
	   State.ADC.Ch1Buf[4]=ADC_Data/30;
	   State.ADC.Ch2Buf[4]=ADC_Data1/30;
	   State.ADC.Ch3Buf[4]=ADC_Data2/30;
	   State.ADC.Ch4Buf[4]=ADC_Data3/30;
	    if((State.ADC.Ch1Buf[4]+State.ADC.Ch2Buf[4]+State.ADC.Ch3Buf[4]+State.ADC.Ch4Buf[4])<=10)
		{
		 	State.Basic.USBState=1;		// USB调试状态中
		}
		else
		{
			if(State.Basic.USBState==1)
			{
			     __set_FAULTMASK(1);
				  NVIC_SystemReset();
				State.Basic.USBState=0;	// USB调试状态中
			}

		}
		AdcT++;
		if(AdcT>=5)
		{
			AdcT=0;
			Max=State.ADC.Ch1Buf[0];
			Min=State.ADC.Ch1Buf[0];
		    for(Abit=1;Abit<=4;Abit++)
			{
			    if(State.ADC.Ch1Buf[Abit]>Max)
				{
					Max=State.ADC.Ch1Buf[Abit];
				}
			    if(State.ADC.Ch1Buf[Abit]<Min)
				{
					Min=State.ADC.Ch1Buf[Abit];
				}
			}
			Port.AIn.Ch1=(State.ADC.Ch1Buf[0]+State.ADC.Ch1Buf[1]+State.ADC.Ch1Buf[2]+State.ADC.Ch1Buf[3]+State.ADC.Ch1Buf[4]-Max-Min)/3;
			Port.AIn.Ch1=Port.AIn.Ch1*2000/Param.Basic.CH1K;
			Max=State.ADC.Ch2Buf[0];
			Min=State.ADC.Ch2Buf[0];
		    for(Abit=1;Abit<=4;Abit++)
			{
			    if(State.ADC.Ch2Buf[Abit]>Max)
				{
					Max=State.ADC.Ch2Buf[Abit];
				}
			    if(State.ADC.Ch2Buf[Abit]<Min)
				{
					Min=State.ADC.Ch2Buf[Abit];
				}
			}
			Port.AIn.Ch2=(State.ADC.Ch2Buf[0]+State.ADC.Ch2Buf[1]+State.ADC.Ch2Buf[2]+State.ADC.Ch2Buf[3]+State.ADC.Ch2Buf[4]-Max-Min)/3;
			Port.AIn.Ch2=(Port.AIn.Ch2-Param.Basic.CH2Z)*2000/Param.Basic.CH2K;	
			Max=State.ADC.Ch3Buf[0];
			Min=State.ADC.Ch3Buf[0];
		    for(Abit=1;Abit<=4;Abit++)
			{
			    if(State.ADC.Ch3Buf[Abit]>Max)
				{
					Max=State.ADC.Ch3Buf[Abit];
				}
			    if(State.ADC.Ch3Buf[Abit]<Min)
				{
					Min=State.ADC.Ch3Buf[Abit];
				}
			}
			Port.AIn.Ch3=(State.ADC.Ch3Buf[0]+State.ADC.Ch3Buf[1]+State.ADC.Ch3Buf[2]+State.ADC.Ch3Buf[3]+State.ADC.Ch3Buf[4]-Max-Min)/3;
			Port.AIn.Ch3=Port.AIn.Ch3*2000/Param.Basic.CH3K;

			Max=State.ADC.Ch4Buf[0];
			Min=State.ADC.Ch4Buf[0];
		    for(Abit=1;Abit<=4;Abit++)
			{
			    if(State.ADC.Ch4Buf[Abit]>Max)
				{
					Max=State.ADC.Ch4Buf[Abit];
				}
			    if(State.ADC.Ch4Buf[Abit]<Min)
				{
					Min=State.ADC.Ch4Buf[Abit];
				}
			}
			Port.AIn.Ch4=(State.ADC.Ch4Buf[0]+State.ADC.Ch4Buf[1]+State.ADC.Ch4Buf[2]+State.ADC.Ch4Buf[3]+State.ADC.Ch4Buf[4]-Max-Min)/3;
		    Port.AIn.Ch4=Port.AIn.Ch4*2000/Param.Basic.CH4K;
		}	
//************************************呼吸灯****8		
	   Tim4s++;	   
	   	if(Tim4s<=200)
	   	{
 //  			GPIO_SetBits(GPIOE, GPIO_Pin_2);
	   	}
	   	else if(Tim4s<=400)
	   	{
//			GPIO_ResetBits(GPIOE, GPIO_Pin_2);	
	  	}
	   	else 
	   	{
	   		Tim4s=0;
	   	}
			
	 	if(State.Action.StartFlg==1)
		{
			if(State.Action.Start>=Param.Auto.StartDelayTime)
			{
				State.Action.StartFlg=0;
				State.Action.Start=0;
				State.Basic.Run=2;					//PID运行时
				State.Basic.PidStart=1;			//PID启动，设置积分初值为当前输出值
//				if(State.Basic.Mode==1)
				{
					if(State.Basic.OutCh==0)		
					{
						State.Out.Mode=1;							//PID输出模式
					}
					else
					{
						State.Out.Mode2=1;							//PID输出模式
					}
//					InitPIDOut();		//启动积分项初始化
//					PID_StateInit();	 //放在ACTION里
				}
			}
			else
			{
				State.Action.Start++;
			}
		}
		
		if(State.Action.StopFlg==1)
		{
			if(Param.Basic.Out==1)			//扭矩模式
			{
				if(State.Action.Stop>=Param.Auto.StopDelayTime)
				{
					State.Action.StopFlg=0;
					State.Action.Stop=0;
					State.Basic.Run=0;			//待机状态
					RunLedOff
					if(Param.Basic.AutoClearDia==1)	  //停机时自动清卷径
					{
		 				DaiClear();	  //卷径数据清0
					}
					else  	if(Param.Basic.AutoClearDia==2)	  //停机时自动保存卷径
					{
					   	DiaSave();
					}
					if(State.Basic.OutCh==0)		
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
					else
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
				}
				else
				{
					State.Action.Stop++;
				}
			}
			else if(Param.Basic.Out==2)			//速度模式
			{
			 	if(State.Action.Stop>=500)			   //200MS后
				{
				 	if(State.Basic.Run==0)			//已停机  待机状态
					{
	 					State.Action.StopFlg=2;
						State.Action.Stop=0;
/*						if(Param.Basic.AutoClearDia==1)	  //自动清卷径		   已在ACTION里实现
						{
						 	DaiClear();	  //卷径数据清0
						}
						else if(Param.Basic.AutoClearDia==2)	  //  自动保存卷径
						{
							DiaSave();
						}
	*/
					}
				}
				else
				{
					State.Action.Stop++;
				}
			}
	
		}
		
		if(State.Action.AChangeFlg==1)	//	B TO A  A启动
		{
				if(State.Action.AChange>=Param.Auto.ChangeTime)
				{
					State.Action.AChangeFlg=0;
					State.Action.AChange=0;
					State.Out.Mode=1;							//A轴PID输出模式
					InitPIDOut();		//轴切积分项初始化
					State.Basic.DisMode=0;		//刷新界面
				}
				else
				{
					State.Action.AChange++;
				}
		}
		
		if(State.Action.AChangeStopFlg==1)	//	A TO B  A制动
		{
			if(State.Action.AChangeStop>=Param.Auto.ChangeStopTime)
			{
				State.Action.AChangeStopFlg=0;
				State.Action.AChangeStop=0;
				State.Out.Mode=0;							//A轴无输出模式
				State.Basic.DisMode=0;		//刷新界面
			}
			else
			{
				State.Action.AChangeStop++;
			}
		}
		if(State.Action.BChangeFlg==1)			 //A TO B   B启动
		{
			if(State.Action.BChange>=Param.Auto.ChangeTime)
			{
				State.Action.BChangeFlg=0;
				State.Action.BChange=0;
				State.Out.Mode2=1;							//B轴PID输出模式
				InitPIDOut();				//PID初始化，使PID输出的I输出(积分），等于当前的输出，速度模式下没有，扭矩下必须的
				State.Basic.DisMode=0;		//刷新界面
			}
			else
			{
				State.Action.BChange++;
			}
		}
	
		if(State.Action.BChangeStopFlg==1)		 //	B TO A  B制动
		{
			if(State.Action.BChangeStop>=Param.Auto.ChangeStopTime)
			{
				State.Action.BChangeStopFlg=0;
				State.Action.BChangeStop=0;
				State.Out.Mode2=0;							//B轴切制动输出模式
			}
			else
			{
				State.Action.BChangeStop++;
			}
		}
		if(State.Action.OverFLG==1)						//过张力检测
		{
			if(State.Work.Tens>Param.Basic.OverTest)			//过张力检测	
			{
				State.Action.Over++;
				if(State.Action.Over>=Param.Basic.OverTestTiems)
				{
	
					State.Basic.OverFLG=1;
		//			State.Basic.Alarm=1;
					State.Action.OverFLG=0;
					State.Action.Over=0;
				}
			}
			else
			{
				State.Action.OverFLG=0;
				State.Action.Over=0;
			}
		}
		else if(State.Action.OverFLG==2)						//不过过张力检测
		{
			if(State.Work.Tens<Param.Basic.OverTest)			//不过过张力检测	
			{
				State.Action.Over++;
				if(State.Action.Over>=Param.Basic.OverTestTiems)
				{
	
					State.Basic.OverFLG=0;						//恢复正常
	//				State.Basic.Alarm=1;
					State.Action.OverFLG=0;
					State.Action.Over=0;
				}
			}
			else
			{
				State.Action.OverFLG=0;
				State.Action.Over=0;
			}
		}
		if(State.Action.ZeroFLG==1)						//欠张力检测
		{
			if(State.Work.Tens<Param.Basic.ZeroTest)			//欠张力检测	
			{
				State.Action.Zero++;
				if(State.Action.Zero>=Param.Basic.ZeroTestTiems)
				{
	
					State.Basic.ZeroFLG=1;
	//				State.Basic.Alarm=1;
					State.Action.ZeroFLG=0;
					State.Action.Zero=0;
				}
			}
			else
			{
				State.Action.ZeroFLG=0;
				State.Action.Zero=0;
			}
		}
		else if(State.Action.ZeroFLG==2)						//欠张力检测
		{
			if(State.Work.Tens>Param.Basic.ZeroTest)			//欠张力检测	
			{
				State.Action.Zero++;
				if(State.Action.Zero>=Param.Basic.ZeroTestTiems)
				{
	
					State.Basic.ZeroFLG=0;
	//				State.Basic.Alarm=1;
					State.Action.ZeroFLG=0;
					State.Action.Zero=0;
				}
			}
			else
			{
				State.Action.ZeroFLG=0;
				State.Action.Zero=0;
			}
		}
	}
}

/*************************************************
函数: void TIM4_IRQHandler(void)
功能: TIM4中断处理函数
参数: 无
返回: 无
说明：一个周期中断1次
**************************************************/
void TIM4_IRQHandler(void)
{

  OSIntEnter(); 

   if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)
   {      
      TIM_ClearITPendingBit(TIM4,TIM_IT_Update);	 //清除中断标志
   }
	//发送通知
	OSSemPost(Tim4Msg);
 
  OSIntExit();
	
}





