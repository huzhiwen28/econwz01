
#include <ucos_ii.h>
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "bsp_fifo.h"
#include "bsp_232c.h"
//#include "hw_config.h"
#include "bsp_fifo.h"

/*232C设备*/
struct _232c{
      struct _fifo ReadFIFO;
	  struct _fifo WriteFIFO;
	  u8 DMARecvBuff[100];
	  u8 DMASendBuff[100];
	  //DMA中消费的数量
	  volatile u16 DMARecvUse;
	  volatile u8 active;

	  /*发送状态，0，没有在发送 1，发送中*/
	  volatile u8 sendstate;
	 
      OS_EVENT *WriteMutex;//写锁
      OS_EVENT *ReadMutex; //读锁
      OS_EVENT *WriteFinishMutex; //写完成通知
};

struct _232c C232DEVICE;


/*取出fifo中的值，返回长度*/
u16 BSP_232CPopoutReadFIFO(struct _fifo* handle,u8* buff,u16 len)
{
  u16 index =0;
  u16 retlen=0;
  if(len > (handle->fifol))
  {
     len = handle->fifol;
  }
  retlen = len;
  for(index = 0;index < retlen; index++)
  {
    if((handle->fifol) > 0)
	{
	  buff[index] = handle->FIFO[handle->op];
      if((handle->op) == 499)
      {
	   handle->op = 0;
	  }
	  else
	  {
	   (handle->op)++;
	  }
      DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,DISABLE);
	  (handle->fifol)--;
      DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,ENABLE);
	}
  }
  return retlen; 
}

//DMA中断
void DMA1_Channel6_IRQHandler(void)
{
  OSIntEnter(); 
  if(DMA_GetITStatus(DMA1_IT_TC6) != RESET)    //
  {
    DMA_InitTypeDef DMA_InitStructure;
	//清标识
    DMA_ClearFlag(DMA1_FLAG_TC6);
	if(C232DEVICE.DMARecvUse < 100)
	{
	  BSP_PushinFIFO(&(C232DEVICE.ReadFIFO),C232DEVICE.DMARecvBuff + C232DEVICE.DMARecvUse,100-C232DEVICE.DMARecvUse);
	  C232DEVICE.DMARecvUse = 0;

      DMA_Cmd(DMA1_Channel6, DISABLE);
      DMA_DeInit(DMA1_Channel6);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C232DEVICE.DMARecvBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	  DMA_InitStructure.DMA_BufferSize = 100;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

      DMA_Cmd(DMA1_Channel6, ENABLE);
	}
	else
	{
	  C232DEVICE.DMARecvUse = 0;
      DMA_Cmd(DMA1_Channel6, DISABLE);
      DMA_DeInit(DMA1_Channel6);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C232DEVICE.DMARecvBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	  DMA_InitStructure.DMA_BufferSize = 100;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

      DMA_Cmd(DMA1_Channel6, ENABLE);
	}     
  }
  OSIntExit();
}

//DMA中断
void DMA1_Channel7_IRQHandler(void)
{
  OSIntEnter(); 
  if(DMA_GetITStatus(DMA1_IT_TC7) != RESET)    //
  {
     DMA_ClearFlag(DMA1_FLAG_TC7);

	if(BSP_FIFOLen(&(C232DEVICE.WriteFIFO)) > 0)
	{
      DMA_InitTypeDef DMA_InitStructure;
	  u16 sendlen = 0;

	  if(BSP_FIFOLen(&(C232DEVICE.WriteFIFO)) > 100)
	  {
	    sendlen = BSP_PopoutFIFO(&(C232DEVICE.WriteFIFO),C232DEVICE.DMASendBuff,100);
	  }
	  else
	  {
	    sendlen = BSP_PopoutFIFO(&(C232DEVICE.WriteFIFO),C232DEVICE.DMASendBuff,BSP_FIFOLen(&(C232DEVICE.WriteFIFO)));
	  }

	  //使能USART 发送DMA
	  USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);

      DMA_Cmd(DMA1_Channel7, DISABLE);

      DMA_DeInit(DMA1_Channel7);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C232DEVICE.DMASendBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	  DMA_InitStructure.DMA_BufferSize = sendlen;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
      DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);

      DMA_Cmd(DMA1_Channel7, ENABLE);
	}
	else
	{
     DMA_Cmd(DMA1_Channel7, DISABLE);
 	 C232DEVICE.sendstate = 0;
	 //发送通知写完成
	 OSSemPost(C232DEVICE.WriteFinishMutex);
	}
  }
  OSIntExit();
}

/*******************************************************************************
	函数名：USART2_Configuration
	输  入:
	输  出:
	功能说明：
	初始化串口硬件设备，启用中断
	配置步骤：
	(1)打开GPIO和USART的时钟
	(2)设置USART两个管脚GPIO模式
	(3)配置USART数据格式、波特率等参数
	(4)使能USART接收中断功能
	(5)最后使能USART功能
*/
void USART2_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

   /*中断*/
   NVIC_InitTypeDef NVIC_InitStructure;
  
   /* Set the Vector Table base location at 0x08000000 */
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				 //中断分组设置

   /* Enable the DMA Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;       
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;       
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

	/* 第1步：打开GPIO和USART2部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    /* Enable DMA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* 第2步：将USART2 Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART2 Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* 第4步：配置USART2参数
	    - BaudRate = 115200 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - No parity
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 115200;//115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

    /* 若接收数据寄存器满，则产生中断 */
    //USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART2， 配置完毕 */
	USART_Cmd(USART2, ENABLE);

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    USART_ClearFlag(USART2, USART_FLAG_TC);     // 清标志

	//使能USART 接收DMA
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);

  //接收DMA初始化
  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C232DEVICE.DMARecvBuff);
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 100;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

  DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,ENABLE);

  DMA_Cmd(DMA1_Channel6, ENABLE);


}


/*232C串口初始化*/
void BSP_232CInit()
{
  u8 err = 0;
  //__set_FAULTMASK(1);
  //NVIC_SystemReset();

  USART2_Configuration(); 
  BSP_InitFIFO(&(C232DEVICE.ReadFIFO));
  BSP_InitFIFO(&(C232DEVICE.WriteFIFO));
  C232DEVICE.active = 0;
  C232DEVICE.sendstate = 0;
  C232DEVICE.DMARecvUse = 0;
  C232DEVICE.WriteMutex = OSMutexCreate(9,&err);//OSSemCreate(1);
  C232DEVICE.ReadMutex = OSMutexCreate(9,&err);//OSSemCreate(1); 
  C232DEVICE.WriteFinishMutex = OSSemCreate(0); 
 }

/*232C串口打开*/
void BSP_232COpen()
{
  C232DEVICE.active = 1;
}

/*232C串口关闭*/
void BSP_232CClose()
{
  C232DEVICE.active = 0;
}

/*232C串口读*/
u16 BSP_232CRead(u8* buff,u16 len)
{
   u16 retval = 0;
   INT8U  err;
   OSMutexPend(C232DEVICE.ReadMutex,0,&err);
   retval = BSP_232CPopoutReadFIFO(&(C232DEVICE.ReadFIFO),buff,len);
   OSMutexPost(C232DEVICE.ReadMutex);
   return retval;
}

/*232C串口读缓冲区中长度*/
u16 BSP_232CInBuffLen()
{
   u16 retval;
   INT8U  err;
   OSMutexPend(C232DEVICE.ReadMutex,0,&err);
  	DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,DISABLE);
   //接收数量大于已经消耗的数量
   if((100-DMA_GetCurrDataCounter(DMA1_Channel6)) > C232DEVICE.DMARecvUse)
   {
      u8 tt =  C232DEVICE.DMARecvUse;
	  C232DEVICE.DMARecvUse = 100-DMA_GetCurrDataCounter(DMA1_Channel6);
	  BSP_PushinFIFO(&(C232DEVICE.ReadFIFO),C232DEVICE.DMARecvBuff + tt,C232DEVICE.DMARecvUse - tt);
   }
   retval = BSP_FIFOLen(&(C232DEVICE.ReadFIFO));
   DMA_ITConfig(DMA1_Channel6,DMA_IT_TC,ENABLE);
   OSMutexPost(C232DEVICE.ReadMutex);
   return retval;
}

/*232C串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_232CWrite(u8* buff,u16 len)
{
  u16 retval = 0;
    INT8U  err;
    OSMutexPend(C232DEVICE.WriteMutex,0,&err);

  DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,DISABLE);

  retval = BSP_PushinFIFO(&(C232DEVICE.WriteFIFO),buff,len);

  //没有开始发送则启动发送
  if(C232DEVICE.sendstate == 0)
  {
      DMA_InitTypeDef DMA_InitStructure;
	  u16 sendlen = 0;
	  C232DEVICE.sendstate = 1;
	  if(BSP_FIFOLen(&(C232DEVICE.WriteFIFO)) > 100)
	  {
	    sendlen = BSP_PopoutFIFO(&(C232DEVICE.WriteFIFO),C232DEVICE.DMASendBuff,100);
	  }
	  else
	  {
	    sendlen = BSP_PopoutFIFO(&(C232DEVICE.WriteFIFO),C232DEVICE.DMASendBuff,BSP_FIFOLen(&(C232DEVICE.WriteFIFO)));
	  }
	  //使能USART 发送DMA
	  USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);

      DMA_Cmd(DMA1_Channel7, DISABLE);

      DMA_DeInit(DMA1_Channel7);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C232DEVICE.DMASendBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	  DMA_InitStructure.DMA_BufferSize = sendlen;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
      DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);
      DMA_Cmd(DMA1_Channel7, ENABLE);
  }

   DMA_ITConfig(DMA1_Channel7,DMA_IT_TC,ENABLE);
   //等待写完成
   OSSemPend(C232DEVICE.WriteFinishMutex,0,&err);
   OSMutexPost(C232DEVICE.WriteMutex);
   return retval;
}

/*232C串口驱动，在大循环中一直运行*/
void BSP_232CRun(void)
{
  if(C232DEVICE.active == 1)
  {
  }
}


void BSP_232CClearReadBuff(void)
{
   INT8U  err;
   OSMutexPend(C232DEVICE.ReadMutex,0,&err);
   BSP_ClearFIFO(&(C232DEVICE.ReadFIFO));
   OSMutexPost(C232DEVICE.ReadMutex);
}
