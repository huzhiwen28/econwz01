#include "stm32f10x.h"
#include "bsp_extsram.h"
#include "stm32f10x_fsmc.h"
/*
fsmc和i2c1连接的eeprom冲突， 不能同时使用
*/
static FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;	 /* 在函数内定义会出现莫名奇妙的错误 */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configures the FSMC and GPIOs to interface with the SRAM memory.
  *         This function must be called before any write/read operation
  *         on the SRAM.
  * @param  None 
  * @retval None
  */
void FSMC_SRAM_Init(void)
{
  	FSMC_NORSRAMTimingInitTypeDef  p;
  	GPIO_InitTypeDef GPIO_InitStructure; 

	/*使能FSMC总线时钟*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
      
	/*使能FSMC总线使用的GPIO组时钟*/  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | 
                         RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);


    /*FSMC数据线FSMC_D[0:15]初始化，推挽复用输出*/
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15| GPIO_Pin_14;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOD, &GPIO_InitStructure); 
  
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
  	GPIO_Init(GPIOE, &GPIO_InitStructure);
  
    /*FSMC地址线FSMC_A[0:17]初始化，推挽复用输出*/
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | 
                                  GPIO_Pin_14 | GPIO_Pin_15;
  	GPIO_Init(GPIOF, &GPIO_InitStructure);
  
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                  GPIO_Pin_4 | GPIO_Pin_5;
  	GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13; 
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
   
  	/*FSMC NOE和NWE初试化，推挽复用输出*/  
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  	/*FSMC NE1初试化，推挽复用输出*/  
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  	/*FSMC NBL0和NBL1初试化，推挽复用输出*/  
  	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 
  	//GPIO_Init(GPIOE, &GPIO_InitStructure); 
  
/*--------------FSMC 总线 存储器参数配置------------------------------*/
  	p.FSMC_AddressSetupTime = 0;           //地址建立时间    
  	p.FSMC_AddressHoldTime = 0;            //地址保持时间  
  	p.FSMC_DataSetupTime = 1;              //数据建立时间
  	p.FSMC_BusTurnAroundDuration = 0;      //总线恢复时间
  	p.FSMC_CLKDivision = 0;                // 时钟分频因子 
  	p.FSMC_DataLatency = 0;          	   //数据产生时间
  	p.FSMC_AccessMode =  FSMC_AccessMode_A; //FSMC NOR控制器时序
  	
/*--------------FSMC 总线 参数配置------------------------------*/
  	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;                  //使用了FSMC的BANK1的子板块1             
  	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;//禁止地址数据线复用
  	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;           //存储器类型为SRAM
  	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;  //存储器数据宽度为8位
  	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; //关闭突发模式访问
    //等待信号优先级，只有在使能突发访问模式才有效
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low; 
    //关闭Wrapped burst access mode，只有在使能突发访问模式才有效
  	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;         
    //等待信号设置，只有在使能突发访问模式才有效
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;     //使能这个BANK的写操作
    //使能/关闭等待信息设置，只在使能突发访问模式才有效
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;     
  	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; //开启Extend Mode
  	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;     //关闭Write Burst Mode   
  	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;               //读操作时序参数
  	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;                   //写操作时序参数
 	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;

  	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

/*--------------使能BANK1的子板块1------------------------------*/
  	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}


/*外扩sram初始化*/
void BSP_ExtSramInit(void)
{
  FSMC_SRAM_Init();
}


//设置马达速度
//number轴号 0对应起始马达 
//speed m/s 马达速度
void MotorSetSpeed(int number,double speed)
{
	//方向
	u8 dir = 0;
	//每毫米对应的脉冲数量
	u32 ppmm = 100;
	//100M时钟
	u32	clk = 100000000;
	uint64_t maxu32 = 4294967295;
	
	//计算pwm设置值，为多少t，精度为小数点后4位
	u32 pwm;
	u32 pwm2;
	if((speed) >= 0)
		pwm = (double)((uint64_t)maxu32/((uint64_t)1000))*speed;
	else
		pwm = (double)((uint64_t)maxu32/((uint64_t)1000))*(0-speed);

	//fpga内存占用为第0-3字节
 	fpga_write(0,number*12+0);
 	fpga_write(1,(u8)((pwm>>24)&0xff) );
 	fpga_write(0,number*12+1);
 	fpga_write(1,(u8)((pwm>>16)&0xff) );
 	fpga_write(0,number*12+2);
 	fpga_write(1,(u8)((pwm>>8)&0xff) );
 	fpga_write(0,number*12+3);
 	fpga_write(1,(u8)((pwm>>0)&0xff) );

	if(speed >= 0)
	{
		fpga_write(0,4);
 		dir = fpga_read(1);//取得老方向
		dir = dir | (0x01 << number);
		fpga_write(0,4);
 		fpga_write(1,dir);//写方向
	}
	else
	{
		fpga_write(0,4);
 		dir = fpga_read(1);//取得老方向
		dir = dir & (~(0x01 << number));
		fpga_write(0,4);
 		fpga_write(1,dir);//写方向
	}
}

//马达速度生效，这样可以保证所有马达速度一致的刷新
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达速度值刷新
void MotorSpeedFlush(u8 bits)
{
 	//数据刷新位，提示pwm和pwm2数据有改变，刷新到工作寄存器中，否者会出现数据不同步的现象，fpga内存占用第96字节
	fpga_write(0,96);
 	fpga_write(1,bits);
}


//马达PWM输出生效
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达输出脉冲,0表示对应的马达停止输出脉冲
void MotorPwmout(u8 bits)
{
	fpga_write(0,97);
 	fpga_write(1,bits);

}

//马达当前脉冲数量刷新，便于stm32读马达的脉冲数量
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达脉冲数量刷新
void MotorOutCntFlush(u8 bits)
{			   
	fpga_write(0,98);
 	fpga_write(1,bits);
}


//读取马达当前发送的脉冲数量
//number轴号 0对应起始马达 
int MotorReadOutCnt(int number)
{
	int cnt = 0;
	fpga_write(0,number*12+11);
 	cnt += fpga_read(1);
 	fpga_write(0,number*12+10);
 	cnt = cnt | (fpga_read(1))<<8;
 	fpga_write(0,number*12+9);
 	cnt = cnt | (fpga_read(1))<<16;
 	fpga_write(0,number*12+8);
 	cnt = cnt | (fpga_read(1))<<24;
	return cnt;
}

void Motorpingpong(void)
{			   
	fpga_write(0,99);
  	fpga_write(1,0xff);
}

