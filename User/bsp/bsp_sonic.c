#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_sonic.h"

/*超声测距*/
struct _SONIC{
      //测量间隔时间
	  volatile u16 timems;

	  //滤波个数
	  volatile u8 filter;

	  //设备是否打开
	  volatile u8 active;

      //测量得到的数据
	  u16 databuff[10];

	  //指向下一个数据采集位置
	  volatile u8 nextindex;

	  //超声测距步骤
	  volatile u8 step;

  	  //运行步骤
	  volatile u8 runstep;

		//待检查时间
		u32 mstime;
		//时间记录
		u32 oldmstime;

};

struct _SONIC SONICDEVICE;
typedef enum {FALSE = 0, TRUE = !FALSE} bool;

static void StartTimer(unsigned int time)
{
	SONICDEVICE.mstime = time;
	SONICDEVICE.oldmstime = OSTimeGet();
}

static bool CheckTimer()
{
	if((OSTimeGet() - SONICDEVICE.oldmstime) > SONICDEVICE.mstime)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*************************************************
函数: void Timer6_Configuration(void)
功能: TIM6 配置
参数: 无
返回: 无
定时计算：(1 /(72 / (72 - 1 + 1)))us
**************************************************/
void Timer6_Configuration(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);	    //打开TIM5定时器的时钟
    
  TIM_DeInit(TIM6);		                                    //TIMx寄存器重设为缺省值
  
  TIM_TimeBaseStructure.TIM_Period = 60000;		            //自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler=72 - 1;               //TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //采样分频
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  
  //TIM_ARRPreloadConfig(TIM5, ENABLE);                       //允许自动重装载寄存器（ARR）
  //TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);	                //允许TIM5溢出中断
  
  //TIM_Cmd(TIM5, ENABLE);	                                //TIM5开启时钟
}

void Sonic_Configure(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  

  /* 第1步：打开GPIO和AFIO部件的时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);


  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           //抢占式优先级别设置为无抢占优先级


  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;          //指定中断源
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //指定响应优先级别1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	        //使能外部中断通道3
  NVIC_Init(&NVIC_InitStructure);

  /*设置PA5为输入*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA,&GPIO_InitStructure);

  /*设置PE5为输出*/
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Configure EXTI Line5 to generate an interrupt on falling and rising edge */  
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;                   //外部中断通道3
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;	   //双沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;					   //使能
  EXTI_Init(&EXTI_InitStructure);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);  //将PA5连接到外部中断通道5
}


/**********************************************************************
* 名    称：EXTI9_5_IRQHandler()
* 功    能：外部中断通道5中断
* 入口参数： 
* 出口参数：
***********************************************************************/
void EXTI9_5_IRQHandler (void)
{
  OSIntEnter(); 
if(EXTI_GetITStatus(EXTI_Line5) != RESET) 
{
  if(SONICDEVICE.step == 1)
  {
    TIM_SetCounter(TIM6,0);
    TIM_Cmd(TIM6, ENABLE);
	SONICDEVICE.step = 2;
  }
  else if(SONICDEVICE.step == 2)
  {
    SONICDEVICE.databuff[SONICDEVICE.nextindex] = TIM_GetCounter(TIM6);
	if(SONICDEVICE.nextindex == 9)
	{
	  SONICDEVICE.nextindex = 0;
	}
	else
	{
	  SONICDEVICE.nextindex ++;
	}
    TIM_Cmd(TIM6, DISABLE);
	SONICDEVICE.step = 0;
  }
  EXTI_ClearFlag(EXTI_Line5);			       //清除中断标志（必须）
  EXTI_ClearITPendingBit(EXTI_Line5);
 }
  OSIntExit();
}

/*超声波测距初始化
u8 filter :平均法滤波，滤波的单位个数1-10，默认1
u16 timems :毫秒单位的超声波测试间隔时间，时间最好大于80ms，以免回波引起误测试
*/
void BSP_SonicInit(u8 filter,u16 timems)
{
  int i = 0;
  Sonic_Configure();
  Timer6_Configuration();
  for(i = 0; i < 10; i++)
  {
     SONICDEVICE.databuff[i] = 0;
  }
   
  SONICDEVICE.active = 0;
  SONICDEVICE.nextindex = 0;

  if(filter > 10)
  {
    SONICDEVICE.filter = 10;
  }
  else
  {
    SONICDEVICE.filter = filter;
  }

  SONICDEVICE.timems = timems;

  SONICDEVICE.step = 0;
  SONICDEVICE.runstep = 0;

}

/*超声波测距打开*/
void BSP_SonicOpen(void)
{
  SONICDEVICE.active = 1;
}

/*超声波测距关闭*/
void BSP_SonicClose(void)
{
  SONICDEVICE.active = 0;
}

/*超声波测距读距离0.1mm单位*/
u16 BSP_SonicRead(void)
{
   u32 sum = 0;
   u8 cnt = 0;
   u8 index = 0;

   if(SONICDEVICE.nextindex == 0)
   {
     index = 9;
   }
   else
   {
     index = SONICDEVICE.nextindex -1;
   }
   for(cnt = 0; cnt < SONICDEVICE.filter;cnt ++)
   {
     sum += SONICDEVICE.databuff[index];
	 if(index == 0)
	 {
	   index = 9;
	 }
	 else
	 {  index --;
	 }
   }
   
   return (sum* 17/(SONICDEVICE.filter*10)) ;
}

/*超声波测距运行*/
void BSP_SonicRun(void *p_arg)
{
    BSP_SonicInit(1,100);
	BSP_SonicOpen();
    while(1)
	{
		 if(SONICDEVICE.active == 1)
		 {
		   if(SONICDEVICE.runstep == 0)
		   {
//		 	 GPIO_WriteBit(GPIOE, GPIO_Pin_5,(BitAction)1);
		     StartTimer(1);
			 SONICDEVICE.runstep = 1;
			 SONICDEVICE.step = 1;
		   }
		   else if(SONICDEVICE.runstep == 1)
		   {
		     if(CheckTimer() == TRUE)
			 {
//		       GPIO_WriteBit(GPIOE, GPIO_Pin_5,(BitAction)0);
			   SONICDEVICE.runstep = 2;
			   StartTimer(SONICDEVICE.timems);
			 }
		   }
		   else if(SONICDEVICE.runstep == 2)
		   {
		     if(CheckTimer() == TRUE)
			 {
			   SONICDEVICE.runstep = 0;
			 }
		   }
		 }
		OSTimeDlyHMSM(0, 0, 0, 1);
 	}
}

