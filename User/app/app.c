

#include <includes.h>	/* 该文件包含了所有必需的.h文件 */
#include <stdio.h>
#include <stdio.h>
#include "bsp_232c.h"
#include "bsp_485.h"
#include "bsp_usb.h"
#include "bsp_eeprom.h"
#include "bsp_dac.h"
#include "bsp_GPIO.h"
#include "bsp_sonic.h"
#include "bsp_enc.h"
#include "bsp_adc.h"
#include "bsp_TimesInt.h"
#include "bsp_extsram.h"
#include "app_kernelregs.h"
#include "app_usbcom.h"
#include "app_backend.h"
#include "app_usbgather.h"
#include "app_usbmodbusmaster.h"
#include "app_usbmodbusslave.h"
#include "app_485com.h"
#include "app_485modbusslave.h"
#include "app_OutControl.h"
#include "app_inout.h"
#include "app_action.h"
#include "app_pid.h"
#include "app_Port.h"
#include "app_encryption.h"
#include <math.h>

/*
*********************************************************************************************************
*                                       模块内全局变量
*********************************************************************************************************
*/

/* 定义每个任务的堆栈空间，app_cfg.h文件中宏定义栈大小 */
static OS_STK AppTaskStartStk[APP_TASK_START_STK_SIZE];
static OS_STK AppTask232CStk[APP_TASK_232C_STK_SIZE];
static OS_STK AppTaskUSBCOMStk[APP_TASK_USBCOM_STK_SIZE];
static OS_STK AppTaskUSBGatherStk[APP_TASK_USBGATHER_STK_SIZE];
static OS_STK AppTaskHeartBeatStk[APP_TASK_HEARTBEAT_STK_SIZE];
static OS_STK AppTaskServotimerStk[APP_TASK_SERVOTIMER_STK_SIZE];
static OS_STK AppTaskServoStk[APP_TASK_SERVO_STK_SIZE];

/*
*********************************************************************************************************
*                                      函数声明
*********************************************************************************************************
*/

static void AppTaskCreate(void);
static void AppTaskStart(void *p_arg);
static void AppTask232C(void *p_arg);

void printint(int num);
u32 myplusecnt = 0;

OS_EVENT *Servotimermsg; //伺服周期通知

double beginspeed = 0.0;

struct tptp{
	u8 ptptype;//0 无 1 5段	2 4段 3 7段 4 6段
	double t0,t1,t2,t3,t4,t5,t6,tv;
	double ahis,vhis,acur,vcur;
	double J,V,S;
	int Ta,Ts;
};

struct tjog{
	u8 jogtype;//0 无 1 2段	2 3段
	double t0,t1,t2,t3,t4,t5,t6,tv;//tv是遍历用的变量
	double ahis,vhis,acur,vcur;
	double J,V,S;
	int Ta,Ts;
	u8 jogstate;//jog状态机 0无 1升速阶段 2匀速阶段 3降速阶段
};

struct tmotor{
	u8 status;//0空闲 1ptp 2jog 3stop 4gear 5gearptp 6gearjog 7gearstop
	struct tptp ptpdata;
	struct tjog jogdata;
	//OS_EVENT *ServoMsg; //伺服周期处理通知
}motor;


//所有的停止都采用jogstop来做，暂时不针对加速度来做停止算法，只针对速度来做
//停止算法只根据时间来停止，而不关注停止指令后的位移
u8 jogstop(int Ta,int Ts,double J)
{
	double V = 0;
	if(motor.status == 1)
	{
		V =  motor.ptpdata.vcur;
	}
	else if(motor.status == 2)
	{
		V =  motor.jogdata.vcur;
	}
	else
	{
		return 1;
	}
	 
	 {
		 //计算曲线规划数据,V0,S0等都是指只有Ts的4段曲线情况
		 double A = J * Ts;
		 double V0 = A * Ts;
		 double S0 = 2*V0*Ts;
	
		 //速度不够，加速阶段只有Ts
		 if(V < V0)
		 {
		 	double Tsnew = sqrt(V/J);
			double S0new = 2*V*Tsnew;
	
			{	
				//按2段分别计算曲线上每个伺服时间点的v
				motor.jogdata.vhis = V;
				motor.jogdata.ahis = 0;
	
				//每段时间长度分别为
				motor.jogdata.t0 = Tsnew;
				motor.jogdata.t1 = Tsnew;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 1;
				motor.status = 2;//jog状态
			}
		 }
		 //速度足够的大,加速和减速阶段有Ta阶段
		 else
		 {
		 	double S0new = V*Ta;
			{
				motor.jogdata.vhis = V;
				motor.jogdata.ahis = 0;

				//每段时间长度分别为
				motor.jogdata.t0 = Ts;
				motor.jogdata.t1 = (V-V0)/A;
				motor.jogdata.t2 = Ts;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 2;
				motor.status = 2;//jog状态
			}
		 }
		motor.jogdata.jogstate = 3;//停止JOG运动
	 }
	 return 0;
}



//中断指令之后移动固定位置的停止jogstopins来做，暂时不针对加速度来做停止算法，只针对速度来做
u8 jogstopins(int Ta,int Ts,double J,double S)
{
	double V = 0;
	//取得当前速度
	if(motor.status == 1)
	{
		V =  motor.ptpdata.vcur;
	}
	else if(motor.status == 2)
	{
		V =  motor.jogdata.vcur;
	}
	else
	{
		return 1;
	}
	 
	 //曲线规划
	 {

		 //计算曲线规划数据,V0,S0等都是指只有Ts的4段曲线情况
		 double A = J * Ts;
		 double V0 = A * Ts;
		 double S0 = 2*V0*Ts;
	
		 //速度不够，加速阶段只有Ts，且Ts有修改
		 if(V < V0)
		 {
		 	double Tsnew = sqrt(V/J);
			double S0new = V*Tsnew;
	
			//5段S曲线,有匀速阶段
			if(S0new <= S)
			{	
				//按5段分别计算曲线上每个伺服时间点的v
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;
	
				//每段时间长度分别为
				motor.jogdata.t0 = Tsnew;
				motor.jogdata.t1 = Tsnew;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 1;
				motor.status = 2;//jog状态
			}
			else if(S0new > S)//4段曲线,距离不够，达不到最大速度，Tsnew要改变
			{
	 			//按4段分别计算曲线上每个伺服时间点的v
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;
	
				Tsnew = pow((S/(J)),1.0/3);
	
				//每段时间长度分别为
				motor.jogdata.t0 = Tsnew;
				motor.jogdata.t1 = Tsnew;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 1;
				motor.status = 2;//jog状态
			}
		 }
		 else if((V>= V0) && (S< S0))//4段曲线，速度够距离不够，Ts要修改无匀速阶段
		 {
	 	 	double Tsnew;
	
			//按4段分别计算曲线上每个伺服时间点的v
			motor.jogdata.vhis = 0;
			motor.jogdata.ahis = 0;
	
			Tsnew = pow((S/(J)),1.0/3);
	
			//每段时间长度分别为
			motor.jogdata.t0 = Tsnew;
			motor.jogdata.t1 = Tsnew;

			motor.jogdata.tv = 0;
			motor.jogdata.jogtype = 1;
			motor.status = 2;//jog状态
		 }
		 //速度和路程都足够的大,加速和减速阶段有Ta阶段,7段速度曲线
		 else if((V >= V0) && (S >= S0))
		 {
		 	double S0new = V*Ta;
		 	if(S0new <= S)//距离够，可以达到V,有7段
			{
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;

				//每段时间长度分别为
				motor.jogdata.t0 = Ts;
				motor.jogdata.t1 = (V-V0)/A;
				motor.jogdata.t2 = Ts;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 2;
				motor.status = 2;//jog状态
			}
			//达不到速度V，只有6段,Ta长度要改变，无匀速阶段
			else
			{
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;

				//每段时间长度分别为
				motor.jogdata.t0 = Ts;
				motor.jogdata.t1 = sqrt(S/(J*Ts) + Ts*Ts/4 )- 1.5*Ts;
				motor.jogdata.t2 = Ts;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 2;
				motor.status = 2;//jog状态
			}
		 }
		motor.jogdata.jogstate = 3;//停止JOG运动
	 }
	 return 0;
}

//点动
u8 jog(int T,double V)
{
	if(motor.status != 0)
		return 1;
	 
	 //jog状态机
	 {

		 //计算曲线规划数据,V0,S0等都是指只有Ts的4段曲线情况
		 double A = J * Ts;
		 double V0 = A * Ts;
		 double S0 = 2*V0*Ts;
	
		 //速度不够，加速阶段只有Ts
		 if(V < V0)
		 {
		 	double Tsnew = sqrt(V/J);
			double S0new = 2*V*Tsnew;
	
			{	
				//按5段分别计算曲线上每个伺服时间点的v
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;
	
				//每段时间长度分别为
				motor.jogdata.t0 = Tsnew;
				motor.jogdata.t1 = Tsnew;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 1;
				motor.status = 2;//jog状态
			}
		 }
		 //速度足够的大,加速和减速阶段有Ta阶段
		 else
		 {
		 	double S0new = V*Ta;
			{
				motor.jogdata.vhis = 0;
				motor.jogdata.ahis = 0;

				//每段时间长度分别为
				motor.jogdata.t0 = Ts;
				motor.jogdata.t1 = (V-V0)/A;
				motor.jogdata.t2 = Ts;

				motor.jogdata.tv = 0;
				motor.jogdata.jogtype = 2;
				motor.status = 2;//jog状态
			}
		 }
		motor.jogdata.jogstate = 1;//启动JOG
	 }
	 return 0;
}

//点到点移动
u8 ptp(int T,double V,double S)
{	 
	if(motor.status != 0)
		return 1;

	motor.ptpdata.J = 0;
	motor.ptpdata.V = V;
	motor.ptpdata.S = S;
	 //ptp状态机
	 {

		 //计算曲线规划数据,V0,S0等都是指只有Ts的4段曲线情况
		int Ts1 = floor(T/2);
		double S0 = 2*V*Ts1;
		double V0 = V;
		int Ty = 0;
		int Tss = 0;

		//4段曲线,无法达到最高速V		
		if(S0 >=S)
		{
			motor.ptpdata.vhis = 0;
			motor.ptpdata.ahis = 0;

			V0 = S/(2*Ts1);
			motor.ptpdata.J = V0/(Ts1*Ts1);
			motor.ptpdata.t0 = Ts1;
			motor.ptpdata.t1 = Ts1;
			motor.ptpdata.t2 = Ts1;
			motor.ptpdata.t3 = Ts1;
			
			//开始
			motor.ptpdata.tv = 0;
			motor.ptpdata.ptptype = 2;
			motor.status = 1;//ptp状态
		 }
		 else //可以达到最高速V,按5段分别计算曲线规划
		 {
			motor.ptpdata.vhis = 0;
			motor.ptpdata.ahis = 0;

			//匀速段时间，取整
			Ty = floor((S-S0)/V);
			
			//为了保持时间整数，取得真正的V,会对于指令的V做修整
			V0 = S/(2*Ts1 + Ty);

			motor.ptpdata.V = V0;
			motor.ptpdata.J = V0/(Ts1*Ts1);;
			motor.ptpdata.t0 = Ts1;
			motor.ptpdata.t1 = Ts1;
			motor.ptpdata.t2 = Ty;
			motor.ptpdata.t3 = Ts1;
			motor.ptpdata.t4 = Ts1;

			//开始
			motor.ptpdata.tv = 0;
			motor.ptpdata.ptptype = 1;
			motor.status = 1;//ptp状态
		 }
	 }
	 
	 KernelRegs[90] = motor.ptpdata.t0*100;
	 KernelRegs[91] = motor.ptpdata.t1*100;
	 KernelRegs[92] = motor.ptpdata.t2*100;
	 KernelRegs[93] = motor.ptpdata.t3*100;
	 KernelRegs[94] = motor.ptpdata.t4*100;
	 KernelRegs[95] = motor.ptpdata.t5*100;
	 KernelRegs[96] = motor.ptpdata.t6*100;

	 return 0;
}


//轴驱动任务
static void ServoTask(void *p_arg)
{
	u8 err;
	int cnt = 0;
	motor.status = 0;

	while (1) 
	{
		 //ptp状态机
		 if(motor.status == 1)
		 {
	 		//按5段分别计算曲线上每个伺服时间点的v
			if(motor.ptpdata.ptptype == 1)
			{
				//遍历曲线的时间变量
				{
				   //加加速阶段
				   if(motor.ptpdata.tv < motor.ptpdata.t0)
				   {
				   	//加速度改变值
					double achg = motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
	
				   }
				   //减减速阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1)) && (motor.ptpdata.tv>= motor.ptpdata.t0))
				   {
				   	//加速度改变值
					double achg = 0-motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   }
				   //匀速阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2)) && (motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1)))
				   {
	
					MotorSetSpeed(0,motor.ptpdata.V);
	
					motor.ptpdata.vcur = motor.ptpdata.V;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
					motor.ptpdata.ahis = 0;
	
				   }
				   //减减速度阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3)) && (motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2)))
				   {
				   	//加速度改变值
					double achg = 0-motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   }
				   //加加速度阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3+motor.ptpdata.t4)) && (motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3)))
				   {
				   	//加速度改变值
					double achg = motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   }
				   //执行完毕，等位移完成后，设置速度为0，此处暂时不等位移条件，直接设置速度
				   else	if(motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3+motor.ptpdata.t4))
				   {
					MotorSetSpeed(0,0);
	
				   	motor.ptpdata.vcur = 0;
				   	//设置速度到fpga
				   	motor.status = 0;
				   }
				   motor.ptpdata.tv++;
				}
	
			}
			else if(motor.ptpdata.ptptype == 2)
			{
				{
				   //加加速阶段
				   if(motor.ptpdata.tv < motor.ptpdata.t0)
				   {
				   	//加速度改变值
					double achg = motor.ptpdata.J*1;
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
	 				MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
	
				   }
				   //减减速阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1)) && (motor.ptpdata.tv>= motor.ptpdata.t0))
				   {
				   	//加速度改变值
					double achg = 0-motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   }
				   //减减速度阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2)) && (motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1)))
				   {
				   	//加速度改变值
					double achg =0 -motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   	
				   }
				   //加加速度阶段
				   else if((motor.ptpdata.tv<(motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3)) && (motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2)))
				   {
				   	//加速度改变值
					double achg = motor.ptpdata.J*1;
	
					//速度改变值
					double vchg = motor.ptpdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.ptpdata.acur = motor.ptpdata.ahis+achg;
	
	
					//当前时间点速度
					motor.ptpdata.vcur = motor.ptpdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.ptpdata.vcur);
	
					//保持历史值
					motor.ptpdata.ahis = motor.ptpdata.acur;
					motor.ptpdata.vhis = motor.ptpdata.vcur;
				   }
				   //执行完毕，等位移完成后，设置速度为0，此处暂时不等位移条件，直接设置速度
				   else	 if(motor.ptpdata.tv >= (motor.ptpdata.t0+motor.ptpdata.t1+motor.ptpdata.t2+motor.ptpdata.t3))
				   {
					MotorSetSpeed(0,0);
	
				   	motor.ptpdata.vcur = 0;
				   	//设置速度到fpga
				   	motor.status = 0;
				   }
				   motor.ptpdata.tv++;
				}
	
			}
		 }
		 //jog状态机
		 else if(motor.status == 2)
		 {
			//JOG启动阶段
			if(motor.jogdata.jogstate == 1)
			{
		 		//按2段分别计算曲线上每个伺服时间点的v
				if( motor.jogdata.jogtype == 1)
				{
					//遍历曲线的时间变量
					{
					   //加加速阶段
					   if(motor.jogdata.tv < motor.jogdata.t0)
					   {
					   	//加速度改变值
						double achg = motor.jogdata.J*1;
		
						//速度改变值
						double vchg = motor.jogdata.ahis*1 + achg*1/2;
		
						//当前时间点加速度
					   	motor.jogdata.acur = motor.jogdata.ahis+achg;
		
		
						//当前时间点速度
						motor.jogdata.vcur = motor.jogdata.vhis+vchg;
		
						//设置速度到fpga
						MotorSetSpeed(0,motor.jogdata.vcur);
		
						//保持历史值
						motor.jogdata.ahis = motor.jogdata.acur;
						motor.jogdata.vhis = motor.jogdata.vcur;
						motor.ptpdata.tv++;
					   }
					   //减减速阶段
					   else if((motor.jogdata.tv<(motor.jogdata.t0+motor.jogdata.t1)) && (motor.jogdata.tv>= motor.jogdata.t0))
					   {
					   	//加速度改变值
						double achg = 0-motor.jogdata.J*1;
		
						//速度改变值
						double vchg = motor.jogdata.ahis*1 + achg*1/2;
		
						//当前时间点加速度
					   	motor.jogdata.acur = motor.jogdata.ahis+achg;
		
						//当前时间点速度
						motor.jogdata.vcur = motor.jogdata.vhis+vchg;
		
						//设置速度到fpga
						MotorSetSpeed(0,motor.jogdata.vcur);
		
						//保持历史值
						motor.jogdata.ahis = motor.jogdata.acur;
						motor.jogdata.vhis = motor.jogdata.vcur;
						motor.ptpdata.tv++;
					   }
					   else if(motor.jogdata.tv>=(motor.jogdata.t0+motor.jogdata.t1))
					   {
						//当前时间点速度
						motor.jogdata.vcur = motor.jogdata.V;
						motor.jogdata.jogstate = 2;
						//设置速度到fpga
						MotorSetSpeed(0,motor.jogdata.vcur);
						motor.jogdata.vhis = motor.jogdata.vcur;
						motor.ptpdata.tv++;
					   }
					}
				}
			}
			//匀速阶段
			else if(motor.jogdata.jogstate == 2)
			{
				//当前时间点速度
				motor.jogdata.vcur = motor.jogdata.V;
				//设置速度到fpga
				MotorSetSpeed(0,motor.jogdata.vcur);
				motor.jogdata.vhis = motor.jogdata.vcur;
			}
			//降速阶段
			else if(motor.jogdata.jogstate == 3)
			{
	
		 		//按2段分别计算曲线上每个伺服时间点的v
				if( motor.jogdata.jogtype == 1)
				{
				   //减减速度阶段
				   if((motor.jogdata.tv<(motor.jogdata.t0)) && (motor.jogdata.tv >= (0)))
				   {
				   	//加速度改变值
					double achg = 0-motor.jogdata.J*1;
	
					//速度改变值
					double vchg = motor.jogdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.jogdata.acur = motor.jogdata.ahis+achg;
	
					//当前时间点速度
					motor.jogdata.vcur = motor.jogdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.jogdata.vcur);
	
					//保持历史值
					motor.jogdata.ahis = motor.jogdata.acur;
					motor.jogdata.vhis = motor.jogdata.vcur;
				   }
				   //加加速度阶段
				   else if((motor.jogdata.tv<(motor.jogdata.t0+motor.jogdata.t1)) && (motor.ptpdata.tv >= (motor.ptpdata.t0)))
				   {
				   	//加速度改变值
					double achg = motor.jogdata.J*1;
	
					//速度改变值
					double vchg = motor.jogdata.ahis*1 + achg*1/2;
	
					//当前时间点加速度
				   	motor.jogdata.acur = motor.jogdata.ahis+achg;
	
	
					//当前时间点速度
					motor.jogdata.vcur = motor.jogdata.vhis+vchg;
	
					//设置速度到fpga
					MotorSetSpeed(0,motor.jogdata.vcur);
	
					//保持历史值
					motor.jogdata.ahis = motor.jogdata.acur;
					motor.jogdata.vhis = motor.jogdata.vcur;
				   }
				   else if(motor.jogdata.tv>=(motor.jogdata.t0+motor.jogdata.t1)) 
				   {
					MotorSetSpeed(0,0);
	
				   	motor.ptpdata.vcur = 0;
				   	//设置速度到fpga
				   	motor.status = 0;
				   }
				}
			}
		}
		//输出使能
	 	MotorPwmout(0xff);
		//速度改变
		MotorSpeedFlush(0xff);

		//等待伺服定时器通知,便于同步处理
   		OSSemPend(Servotimermsg,0,&err);
		//servo数据切换
		Motorpingpong();

		//通知刷新所有的轴的位置
		MotorOutCntFlush(0xff);

		//读轴的当前位置
		cnt = MotorReadOutCnt(0);
		KernelRegs[81] = cnt & 0x0000ffff;
		KernelRegs[82] = (cnt >> 16) & 0x0000ffff;

		cnt = MotorReadOutCnt(1);
		KernelRegs[83] = cnt & 0x0000ffff;
		KernelRegs[84] = (cnt >> 16) & 0x0000ffff;
		KernelRegs[86] = motor.ptpdata.vcur*1000;
 		//OSTimeDlyHMSM(0, 0, 0, 1);

	}
}

//伺服驱动定时器任务
static void ServotimerTask(void *p_arg)
{
	//Servotimermsg = OSSemCreate(0);
	while(1)
	{
		OSTimeDlyHMSM(0, 0, 0, 1);
		OSSemPost(Servotimermsg);		//发送通知
	}
}

/*
*********************************************************************************************************
*                                      函数定义
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	CPU_INT08U  err;
	int i;
	//等fpga配置完毕
	for(i = 0; i < 1000000; i ++);
	
	/* 初始化"uC/OS-II"内核 */

	OSInit();
 	SystemInit();
 	Servotimermsg = OSSemCreate(0);

	//BSP_232CInit();
	//BSP_232COpen();
	BSP_ExtSramInit();
 	BSP_USBInit();
	BSP_USBOpen();


	App_Backend_Init();
	USB_ModbusMaster_Init();
	
	/* 禁止所有的中断 */
	BSP_IntDisAll();


	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreateExt(AppTaskStart,	/* 启动任务函数指针 */
                    (void *)0,		/* 传递给任务的参数 */
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1], /* 指向任务栈栈顶的指针 */
                    APP_TASK_START_PRIO,	/* 任务的优先级，必须唯一，数字越低优先级越高 */
                    APP_TASK_START_PRIO,	/* 任务ID，一般和任务优先级相同 */
                    (OS_STK *)&AppTaskStartStk[0],/* 指向任务栈栈底的指针。OS_STK_GROWTH 决定堆栈增长方向 */
                    APP_TASK_START_STK_SIZE, /* 任务栈大小 */
                    (void *)0,	/* 一块用户内存区的指针，用于任务控制块TCB的扩展功能
                       （如任务切换时保存CPU浮点寄存器的数据）。一般不用，填0即可 */
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); /* 任务选项字 */
					/*  定义如下：
						OS_TASK_OPT_STK_CHK      使能检测任务栈，统计任务栈已用的和未用的
						OS_TASK_OPT_STK_CLR      在创建任务时，清零任务栈
						OS_TASK_OPT_SAVE_FP      如果CPU有浮点寄存器，则在任务切换时保存浮点寄存器的内容
					*/                  

	/* 指定任务的名称，用于调试。这个函数是可选的 */
	OSTaskNameSet(APP_TASK_START_PRIO, APP_TASK_START_NAME, &err);

	/* 启动多任务系统，控制权交给uC/OS-II */
	OSStart();
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器(在BSP_Init中实现)
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{
//	uint8_t i, j;

	(void)p_arg;   /* 仅用于避免编译器告警，编译器不会产生任何目标代码 */

	/* BSP 初始化。 BSP = Board Support Package 板级支持包，可以理解为底层驱动。*/
	BSP_Init();

	/* 检测CPU能力，统计模块初始化。该函数将检测最低CPU占有率
	注意：此函数会延迟100ms再返回 */
#if (OS_TASK_STAT_EN > 0)
	OSStatInit();
#endif

	/* 创建应用程序的任务 */
	AppTaskCreate();

	/* 任务主体，必须是一个死循环 */
	while (1)     
	{
		OSTimeDlyHMSM(20, 0, 0, 0);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	CPU_INT08U      err;


	
	OSTaskCreateExt(AppTask232C,
                    (void *)0,
                    (OS_STK *)&AppTask232CStk[APP_TASK_232C_STK_SIZE - 1],
                    APP_TASK_232C_PRIO,
                    APP_TASK_232C_PRIO,
                    (OS_STK *)&AppTask232CStk[0],
                    APP_TASK_232C_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSTaskNameSet(APP_TASK_232C_PRIO, APP_TASK_232C_NAME, &err);

	OSTaskCreateExt(ServotimerTask,
                    (void *)0,
                    (OS_STK *)&AppTaskServotimerStk[APP_TASK_SERVOTIMER_STK_SIZE - 1],
                    APP_TASK_SERVOTIMER_PRIO,
                    APP_TASK_SERVOTIMER_PRIO,
                    (OS_STK *)&AppTaskServotimerStk[0],
                    APP_TASK_SERVOTIMER_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSTaskNameSet(APP_TASK_SERVOTIMER_PRIO, APP_TASK_SERVOTIMER_NAME, &err);

	OSTaskCreateExt(ServoTask,
                    (void *)0,
                    (OS_STK *)&AppTaskServoStk[APP_TASK_SERVO_STK_SIZE - 1],
                    APP_TASK_SERVO_PRIO,
                    APP_TASK_SERVO_PRIO,
                    (OS_STK *)&AppTaskServoStk[0],
                    APP_TASK_SERVO_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSTaskNameSet(APP_TASK_SERVO_PRIO, APP_TASK_SERVO_NAME, &err);

	OSTaskCreateExt(App_HeartbeatTask_Run,
                    (void *)0,
                    (OS_STK *)&AppTaskHeartBeatStk[APP_TASK_HEARTBEAT_STK_SIZE - 1],
                    APP_TASK_HEARTBEAT_PRIO,
                    APP_TASK_HEARTBEAT_PRIO,
                    (OS_STK *)&AppTaskHeartBeatStk[0],
                    APP_TASK_HEARTBEAT_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
	OSTaskNameSet(APP_TASK_HEARTBEAT_PRIO, APP_TASK_HEARTBEAT_NAME, &err);

	OSTaskCreateExt(USBCOM_Run,
                    (void *)0,
                    (OS_STK *)&AppTaskUSBCOMStk[APP_TASK_USBCOM_STK_SIZE - 1],
                    APP_TASK_USBCOM_PRIO,
                    APP_TASK_USBCOM_PRIO,
                    (OS_STK *)&AppTaskUSBCOMStk[0],
                    APP_TASK_USBCOM_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSTaskNameSet(APP_TASK_USBCOM_PRIO, APP_TASK_USBCOM_NAME, &err);

	OSTaskCreateExt(USB_Gather_Run,
                    (void *)0,
                    (OS_STK *)&AppTaskUSBGatherStk[APP_TASK_USBGATHER_STK_SIZE - 1],
                    APP_TASK_USBGATHER_PRIO,
                    APP_TASK_USBGATHER_PRIO,
                    (OS_STK *)&AppTaskUSBGatherStk[0],
                    APP_TASK_USBGATHER_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSTaskNameSet(APP_TASK_USBGATHER_PRIO, APP_TASK_USBGATHER_NAME, &err);

}



/*
*********************************************************************************************************
*	函 数 名: AppTask232C
*	功能说明: 按键检测任务。这个任务检测USER键和WAKEUP键的状态，并将消息发送到
*				用户界面任务：AppTaskUserIF。
*				这个按键检测带简单的滤波功能，仅供参考。
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTask232C(void *p_arg)
{
	u16 oldreg79 = 0;	//历史寄存器79，控制位
	u16 oldreg00 = 0;	//历史寄存器00，速度值mm/s
	u16 oldreg01 = 0;	//历史寄存器01，速度值mm/s
	u16 oldreg02 = 0;	//历史寄存器02，速度值mm/s
	u16 oldreg03 = 0;	//历史寄存器03，速度值mm/s
	u16 oldreg04 = 0;	//历史寄存器04，速度值mm/s
	u16 oldreg05 = 0;	//历史寄存器05，速度值mm/s
	u16 oldreg06 = 0;	//历史寄存器06，速度值mm/s
	u16 oldreg07 = 0;	//历史寄存器07，速度值mm/s
	u8 speedchg = 0;
	int ii=0;
	short passclk = 0;
	u8 speedstate = 0;
	int cnt = 0;

	(void)p_arg;	//避免编译器告警
 
 	//延迟300ms等待fpga准备就绪，串行epcs需要时间？
 	//OSTimeDlyHMSM(0, 0, 0, 300);

	for(ii = 0;ii <100;ii++)
	{
		KernelRegs[ii] = 0;
	}

	while (1) 
	{
	    u8 buff[30];
		u16 l = 0;
		int iii = 0;
		speedchg = 0;
		
	/*	if(speedstate == 0)
		{

			if(KernelRegs[00] > 500)
			{
				speedstate = 1;
			}
			else
			{
				KernelRegs[00]++;
			}
		}
		else
		{
			if(KernelRegs[00] < -500)
			{
				speedstate = 0;
			}
			else
			{
				KernelRegs[00]--;
			}
		}
	*/	
		
		

		if((((oldreg79 >> 4) & 0x0001) == 0x0000) &&(((KernelRegs[79]>>4) & 0x0001) == 0x0001))
		{
			ptp(KernelRegs[0],(double)(KernelRegs[1])/10,(double)(KernelRegs[2]));
		}
		oldreg79 = KernelRegs[79];

		OSTimeDlyHMSM(0, 0, 0, 20);	 /* 也可以调用 OSTimeDly() 函数实现延迟 */				  
	}
}


/*
*********************************************************************************************************
*                                   定义应用程序钩子函数(HOOK)
*
* 说明 : 这些函数被 os_cpu_c.c 中钩子(HOOK)调用。
*
* 返回 : 无
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)

/* 当一个任务创建完成时，uCOS内核调用该函数。调用时中断是禁止状态 */
void  App_TaskCreateHook (OS_TCB *ptcb)
{
    (void)ptcb;
}

/* 当一个任务被删除后，uCOS内核调用该函数。调用时中断是禁止状态 */
void  App_TaskDelHook (OS_TCB *ptcb)
{
    (void)ptcb;
}

/* 这个函数被idle任务(CPU空闲时)调用。用户可以添加代码让CPU进入睡眠模式。调用时中断是使能状态*/
#if OS_VERSION >= 251
void  App_TaskIdleHook (void)
{
}
#endif

/* 该函数被uC/OS-II的统计任务调用，每秒调用一次。用户可以添加额外的统计代码 */
void  App_TaskStatHook (void)
{
}

/* 该函数在任务切换时被调用。调用时中断是禁止状态 */
#if OS_TASK_SW_HOOK_EN > 0
void  App_TaskSwHook (void)
{
}
#endif

/* 这个函数被 OS_TCBInit()调用，在初始化最后的任务控制块时调用。中断可能是禁止的也可能是使能的 */
#if OS_VERSION >= 204
void  App_TCBInitHook (OS_TCB *ptcb)
{
    (void)ptcb;
}
#endif

/* 这个函数在每次滴答时被调用。中断可能是禁止的也可能是使能的 */
#if OS_TIME_TICK_HOOK_EN > 0
void  App_TimeTickHook (void)
{
}
#endif
#endif
