										   
/*  
邱毅
time:2008,09,26
*/
#ifndef _APP_BASICDEF_H
#define _APP_BASICDEF_H

#include "stm32f10x.h"


typedef unsigned int uint;    
typedef unsigned long ulong;    
typedef unsigned char uchar;



extern uint ADCBuf[];
extern int PidOut;
extern int KPout;
extern int KIout;
extern int KDout;
extern u32  KPSL;
extern struct PID Pid; // PID Control Structure
extern struct ParameterSate Param;
extern	struct state State;
extern struct PortState Port;

extern unsigned int rout; // PID Response (Output)
extern unsigned int rin; // PID Feedback (Input)


extern	uint  OUTDATA[10];
extern 	uint  OUTDATB[10];
extern	long int SETD;

//uint CH2AD[20];

//#define _nop_() {__asm__ volatile("nop");}
#define _nop_() { ;} 
 

void Delay_1ms(void);
void Delay(unsigned int n);
void Write_Cmd(unsigned char cmd);
void Write_Data(unsigned char dat);
uchar Read_Data(void);
void Ini_Lcd(void);
void dis_pic(uchar *pic);
void tuchu(uchar x,uchar y)	;
void Clear_GDRAM(void);
void Draw_TX(unsigned char Yaddr,unsigned char Xaddr,const unsigned char * dp);
void Disp_HZ(uchar addr,const uchar * pt,uchar num);
void Draw_PM(const uchar *ptr);
void Read_Data1(uchar*lcdtemp1,uchar *lcdtemp2);
void ATUO_Menu1(void);
void ZL_Make(void);
void DateDisplay(uchar , uchar, uchar, uchar, uchar );


struct PID {
uint SetD;      // 设定目标Desired value
ulong CycT;
ulong  KP;    // 比例常数Proportional Const
ulong   KI;      // 积分常数Integral Const
ulong   KD;    // 微分常数Derivative Const
ulong   KB;    // 超调区增益
ulong   KS;	//死区增益
int LastError;     // Error[-1]

int PrevError;    // Error[-2]
int  SumError;    // Sums of Errors
int IFull;
int IZero;
uint Pol;

uint Die;			//
uint Over;			//超调区范围1~100% 80%
uint Change;
u16 Start;
u16 KPV;
u16 KPD;
};


struct AutoParam
{
	uint SetAim;
	uint OpenOut;
	uint StartOut;
	uint StartDelayTime;
	uint StopOut;
	uint StopDelayTime;
	uint JogOut;
	uint ChangeOut;
	uint ChangeTime;
	uint ChangeStopTime;
};

struct MaunParam
{
	uint SetAim;
	uint Out;
	uint SetupZero;

};

struct TensilityInputParam
{
	uint Code;
	uint Float;
	uint Unit;
	uint Full;
	uint SetZero;
	uint Test;
	uint Zero;
	uint Line;	
};


struct DiaParam			//卷径控制参数
{
	uint Test;         //0"不检测，1检测
	uint Way;			//卷径测量方法
	uint Ply;			//卷料厚度
	int  RollD;			//卷轴脉冲数
	int  MainD;			//主轴脉冲数
	uint LeastDia;			//最小直径值
	uint MostDia;			//最大直径值
//	uint StartDia;			//开机直径补偿值
//	uint InitDia;		//初始直径
	uint Ratio;				//主轴直径/
	uint RollAWay;			//卷轴A脉冲信号来自于  0：无 1：IO口6   2：编码器口
	uint RollBWay;			//卷轴B脉冲信号来自于  0：无 1：IO口5   2：编码器口
	uint RollMWay;			//主轴脉冲信号来自于  0：无 1：IO口5   2：编码器口

};

struct BasicParam
{
//	uint Code;				//内部密码值
	uint MMFR;				//0：正转，1反转
	uint Mode;				//控制模式，自动、手动、卷径
	uint MachMode;			//1：放卷；2：收卷；3：牵引。
	uint ShowFilter;		//实际张力显示滤波
	uint OutFilter;			//实际张力输出滤波
	uint Out;				//1:扭矩输出，2：速度输出
//	uint UpSpeed;			 //加事增yi
//	uint DownSpeed;
	uint ZeroTest;			//零张力检测值
	uint OverTest;			//过张力检测值
	uint ZeroTestTiems;		//零张力检测时间
	uint OverTestTiems;		//过张力检测时间
	uint ZDZ;				//锥度值τ
	//uint ZDMode;			//控制功能选择
	uint ZDLine;			//锥度特性 1：直线锥度；2：曲线锥度
	uint LeastOut;			//最小输出
	uint MostOut;			//最大输出
	uint SaveOut;			//保存输出
	uint MaxMainSpeedPor;	//卷轴最小时对应主速度的输出比率，（相对主速度的最大输出速度比率）
	uint Chang ;			//0:单轴，1：双轴
	uint KPid;
	uint 	MSPAF;	 //速度前馈参数
	uint MainSpeedWay;  //主速度来自于
	uint SetWay;          	//设置数据来自于
	int MainSpeedZero;         //主速度零位
	uint AutoClearDia;		//运行停机时自动卷径清0
	u16  SetOut;
	u16 CH1K;
	u16 CH2K;
	u16 CH3K;
	u16 CH4K;
	u16 CH2Z;
};

struct AOutPortParam			//卷径控制参数
{
u8  CH1OutMode;
u8  CH2OutMode;
u8  CH3OutMode;
u8  CH4OutMode;

};

struct ZDZParam			//卷径控制参数
{
u16  ZDZ10;
u16  ZDZ20;
u16  ZDZ30;
u16  ZDZ40;
u16  ZDZ50;
u16  ZDZ60;
u16  ZDZ70;
u16  ZDZ80;
u16  ZDZ90;
u16  ZDZ100;

};

struct ParameterSate
{
	struct BasicParam Basic;
	struct DiaParam Dai;
	struct AOutPortParam AOutPort;
	struct TensilityInputParam Tens;
	struct MaunParam Maun;
	struct AutoParam Auto;
	struct ZDZParam	ZDZ;
};
	struct BasicState					//定义基本状态
	{
		uint T5;
		uint PIDCycFLG;
		uint DisplayFLG;
		uint DisMode;
		uint Alarm;
		uint OverFLG;
		uint ZeroFLG;
		uint Mima;          		     //正在输入的密码
		uint Mode;						//1:自动控制模式，2手动，3卷径
		uint KeyFLG:1;					//按键标志	
		uint ADFLG:1;					
		uint Param:3;

		uint Run:3;						//0:停止，1：运行
		uint OutCh:3;					//0:A通道输出，1：B
		uint KeyOutA:3;					//按键输出允许 0：禁止，1：允许
		uint IoOutA:3;					//IO按键输出允许 0：禁止，1：允许//由IO功能设置3控制，不选择时，默认输出允许
		uint OutSave:3;					//输出记忆，0：不记	，1记
		uint SpeedUp:3;					//加速信号，0：不，1：加
		uint SpeedDown:3;				//减
		uint Jog:3;						//点动信号 0:正常，1：点动
		uint PidStart:2;						//点动信号 0:正常，1：点动
		uint CodeError:1;
		uint SetLock:1;
		uint Out:2;						// 1：扭矩信号，2 速度信号
		uint SaveF:1;
		uint OK:1;
		uint ParamERR:1;		//参数设置错误
		uint USBState:1;		//Usb状态
		uint DiaOK:1;			//卷径已找到
		uint ParamERRID;		//参数设置错误ID
	    uint ID;
		uint KeyLTimes;
		uint SaveTimes;		
		uint PidInit;
	};	

	struct ADCState
	{
		uint Buf;				//转换值，结果
		uint CHAL;
		uint Ch1Buf[10];			//通道1
		uint Ch2Buf[10];			//通道2
		uint Ch3Buf[10];			//通道3
		uint Ch4Buf[10];			//通道3
		uint Times;				//转换次数
		uint AdcCh;				//通道
	};

	struct WorkState
	{
		int InData;
		int Tens;
		uint SetData;
		uint MSpeed;				//主角速度信号
		uint RSpeed;				//卷轴角速度信号
		uint Dia;					//A卷径实际值
		u32 DiaPer;					//A卷径百分比
		uint Dia2;					//B卷径实际值
		u32 DiaPer2;				//B卷径百分比
		uint MaunOut;
		uint SpeedContrast;			//速度差信号
		uint ZDZ;
		u32 RollDT;
		u32 RollDT2;
		int64_t MainDT;
		u32 RollDS;
		u32 RollDS2;	//B轴脉冲数
		int64_t MainDS;		  //主轴圈数
		uint RollDSpeed;	 //A轴角速度
		uint RollD2Speed;	 //B轴角速度
		uint MainDSpeed;	//主轴角速度
		u16  EEPROMERR;  //EEPROM故障
		u16  DIAERR;  //卷径故障
	};

	struct ActionSate
	{
		uint StartFlg;
		uint Start;
		uint StopFlg;
		uint Stop;	
		uint JogFlg;
		uint Jog;
		uint AChangeFlg;
		uint AChange;
		uint AChangeStopFlg;
		uint AChangeStop;
		uint BChangeFlg;
		uint BChange;
		uint BChangeStopFlg;
		uint BChangeStop;
		uint OverFLG;
		uint Over;
		uint ZeroFLG;
		uint Zero;

		uint KeyFLG;
		uint Key;

	};

	struct OutState
	{
		uint Mode;		//0：不输出，1:PID自动输出，2手动输出，3：启动预输出，4：停机输出,5点动输出,6轴切输出，7开机预输出
		uint Mode2;		//0：不输出，1:PID自动输出，2手动输出，3：启动预输出，4：停机输出,5点动输出,6轴切输出，7开机预输出
		uint Port;
		uint Data;		//通道A输出
		uint Data2;		//通道B输出
		uint Per;
		uint Per2;
	};

	struct PIDState
	{
	uint LeastOut;			//最小输出
	uint MostOut;			//最大输出		
	uint Over;				//超调区
	uint Die;				//死区
	uint ChangeH;				//变化度高限
	uint ChangeL;				//变化度低限	
	};

	struct StataToPLCBit
	{
	  u16 Run: 1;	  //运行
	  u16 ZeroFLG: 1;	   //欠张力
	  u16 OverFLG: 1;	   //过张力
	  u16 JOG: 1;		 //点动开停
	  u16 XZFX:1;    //输出旋转方向，0～10V：0，0～-10V:1
	  u16 ERR:1;
	  u16 OK:1;			//就绪
	  u16 Save:1;
	  u16 Break:1;       //断点
	  u16 A_B:1;   //0:A轴，1B
	  u16 EepromErr:1;		//EEPROM故障
	  u16 DiaErr:1;		//卷卷径偏差报警
	  u16 ParamC:1;		  //参数有改变，给PLC用
	  u16 ParamErr:1;	//参数校对错误
	  u16 USBState:1;		//Usb状态
	  u16 DiaOK:1;			//卷径已找到
	};

	union  StataToPLCUnion
	{
	 u16 All;
	 struct StataToPLCBit  Bit;
	};

 	struct PLCCommandBit
	{
	  u16 Run: 1;	  //运行
	  u16 Sampling: 1;	   //采样
	  u16 Refurbish: 1;	   //pc 机请求刷参数
	  u16 Off_Line:1;        //PC机离线请求
	  u16 JOG: 1;		 //点动开停
	  u16 XZFX:1;    //输出旋转方向，0～10V：0，0～-10V:1
	  u16 ERR:1;
	  u16 Reset:1;			//复位
	  u16 Save:1;		   //记忆
	  u16 ClearSave:1;     //清除记忆
	  u16 UseSave:1;       //应用记忆
	  u16 ClearDia:1;       //卷径清零
	  u16 TensZero:1;   //张力校零
	  u16 MainSpeedZero:1;  //主速度校零
	  u16 ABChange:1;  		//A/B轴切换
	  u16 DiaSet:1;  		//A/B轴切换
	};
	union  PLCCommandUnion
	{
	 u16 All;
	 struct PLCCommandBit  Bit;
	};

	struct PLCCommandBit2
	{
	  u16 InitParam: 1;	  //恢复出厂设置
	  u16 ADCZero: 1;	   //采样
	  u16 ADCTest: 1;	   //pc 机请求刷参数

	};
	union  PLCCommandUnion2
	{
	 u16 All;
	 struct PLCCommandBit2  Bit;
	};
 // 	union  PLCCommandUnion	PLC_Data;

	struct state
	{
		struct BasicState Basic;
		struct ADCState ADC;
		struct WorkState Work;
		struct ActionSate Action;
		struct OutState Out;
		struct PIDState	PID;
		union  StataToPLCUnion	ToPLC;
		union  PLCCommandUnion	PLC_Command;
		union  PLCCommandUnion2	PLC_Command2;
	//   	union  PLCCommandUnion	PLC_Data;
	};

struct DataInputPort
{
	uint S1:1;
	uint S2:1;
	uint S3:1;
	uint S4:1;
	uint S5:1;
	uint S6:1;
	uint S7:1;
	uint S8:1;
	uint S9:1;
};

union DInput
{
struct DataInputPort Bits;
uint All;
};

struct DataOutputPort
{
uint SO1:1;
uint SO2:1;
};

union DOutput
{
struct DataOutputPort Bits;
uint All;
};

struct AInputPort
{
uint Ch1;
int Ch2;
uint Ch3;
uint Ch4;
};

struct AOutputPort
{
u32 Ch1;
u32 Ch2;
u32 Ch3;
u32 Ch4;
};

struct PortState
{
union DInput  DIn;
union DOutput  DOut;
struct AOutputPort  AOut;
struct AInputPort  AIn;
};


struct PidRunState
{
u16  ErrAbs[500];
int  ErrO[500];
u16  DErr[500];
u16  No;
u16  UseNo;
};



#endif


