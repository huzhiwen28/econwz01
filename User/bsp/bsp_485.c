
#include <ucos_ii.h>
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "bsp_fifo.h"
#include "bsp_485.h"
#include "bsp_fifo.h"

/*485设备*/
struct _485{
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

struct _485 C485DEVICE;


/*取出fifo中的值，返回长度*/
u16 BSP_485PopoutReadFIFO(struct _fifo* handle,u8* buff,u16 len)
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
      DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,DISABLE);
	  (handle->fifol)--;
      DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);
	}
  }
  return retlen; 
}

//DMA中断
void DMA1_Channel3_IRQHandler(void)
{
  OSIntEnter(); 
  if(DMA_GetITStatus(DMA1_IT_TC3) != RESET)    //
  {
    DMA_InitTypeDef DMA_InitStructure;

	//清标识
    DMA_ClearFlag(DMA1_FLAG_TC3);

    /* 接收使能 */
    //GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	/* nRE = 0 */
    GPIO_SetBits(GPIOF, GPIO_Pin_11); 	/* nRE = 0 */
    /* 发送禁止 */
    //GPIO_ResetBits(GPIOF, GPIO_Pin_10);	/* DE = 0 */ 
    GPIO_SetBits(GPIOF, GPIO_Pin_10);	/* DE = 0 */ 

	if(C485DEVICE.DMARecvUse < 100)
	{
	  BSP_PushinFIFO(&(C485DEVICE.ReadFIFO),C485DEVICE.DMARecvBuff + C485DEVICE.DMARecvUse,100-C485DEVICE.DMARecvUse);
	  C485DEVICE.DMARecvUse = 0;

      DMA_Cmd(DMA1_Channel3, DISABLE);
      DMA_DeInit(DMA1_Channel3);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C485DEVICE.DMARecvBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	  DMA_InitStructure.DMA_BufferSize = 100;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel3, &DMA_InitStructure);

      DMA_Cmd(DMA1_Channel3, ENABLE);
	}
	else
	{
	  C485DEVICE.DMARecvUse = 0;
      DMA_Cmd(DMA1_Channel3, DISABLE);
      DMA_DeInit(DMA1_Channel3);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C485DEVICE.DMARecvBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	  DMA_InitStructure.DMA_BufferSize = 100;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel3, &DMA_InitStructure);

      DMA_Cmd(DMA1_Channel3, ENABLE);
	}     
  }
  OSIntExit();
}

//DMA中断
void DMA1_Channel2_IRQHandler(void)
{
  OSIntEnter(); 
  if(DMA_GetITStatus(DMA1_IT_TC2) != RESET)    //
  {
      //GPIO_SetBits(GPIOF, GPIO_Pin_10);		
      //GPIO_SetBits(GPIOF, GPIO_Pin_11); 	
      GPIO_ResetBits(GPIOF, GPIO_Pin_10);		
      GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	

     DMA_ClearFlag(DMA1_FLAG_TC2);

	if(BSP_FIFOLen(&(C485DEVICE.WriteFIFO)) > 0)
	{
      DMA_InitTypeDef DMA_InitStructure;
	  u16 sendlen = 0;

	  if(BSP_FIFOLen(&(C485DEVICE.WriteFIFO)) > 100)
	  {
	    sendlen = BSP_PopoutFIFO(&(C485DEVICE.WriteFIFO),C485DEVICE.DMASendBuff,100);
	  }
	  else
	  {
	    sendlen = BSP_PopoutFIFO(&(C485DEVICE.WriteFIFO),C485DEVICE.DMASendBuff,BSP_FIFOLen(&(C485DEVICE.WriteFIFO)));
	  }

	  //使能USART 发送DMA
	  USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);

      DMA_Cmd(DMA1_Channel2, DISABLE);

      DMA_DeInit(DMA1_Channel2);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C485DEVICE.DMASendBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	  DMA_InitStructure.DMA_BufferSize = sendlen;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
      DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);

      DMA_Cmd(DMA1_Channel2, ENABLE);
	}
	else
	{
     DMA_Cmd(DMA1_Channel2, DISABLE);
 	 C485DEVICE.sendstate = 0;

     USART_ClearFlag(USART3, USART_FLAG_TXE);     // 清标志
	 USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	}
  }
  OSIntExit();
}

/*******************************************************************/
/*                                                                 */
/* STM32串口中断，只有在发送的最后阶段才使能这个中断,保证数据发送完毕 */
/* 说明：串口3中断                                             */
/*                                                                 */
/*******************************************************************/
void USART3_IRQHandler(void)            //在中断服务程序中，由于主机响应中断时并不知道是哪个中断源发出中断请求，因此必须在中断服务程序中对中断源进行判别，然后分别进行处理。当然，如果只涉及到一个中断请求，是不用做上述判别的。但是无论什么情况，做上述判别是个好习惯
{
  OSIntEnter(); 

  if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)    //若接收数据寄存器满
  {
      USART_ClearFlag(USART3, USART_FLAG_TXE);     // 清标志
	  USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
      USART_ClearFlag(USART3, USART_FLAG_TC);     // 清标志
	  USART_ITConfig(USART3, USART_IT_TC, ENABLE);
  }

  if(USART_GetITStatus(USART3, USART_IT_TC) != RESET)    //数据发送完成
  {
      USART_ClearFlag(USART3, USART_FLAG_TC);     // 清标志
	  USART_ITConfig(USART3, USART_IT_TC, DISABLE);

     //注意：因为RS485的半双工方式，在RS485发送数据完毕后，需要把RS485设置在接收状态，否则收不到主机的数据
      //GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	
      //GPIO_ResetBits(GPIOF, GPIO_Pin_10);	 
      GPIO_SetBits(GPIOF, GPIO_Pin_10);		
      GPIO_SetBits(GPIOF, GPIO_Pin_11); 	
  	 //发送通知写完成
	 OSSemPost(C485DEVICE.WriteFinishMutex);
}
  OSIntExit();
} 

/*******************************************************************************
	函数名：USART3_Configuration
	输  入:
	输  出:
	功能说明：
	初始化串口硬件设备，启用中断
	配置步骤：
	(1)打开GPIO和USART的时钟
	(2)设置USART3两个管脚GPIO模式
	(3)配置USART3数据格式、波特率等参数
	(4)使能USART3接收中断功能
	(5)最后使能USART3功能
*/
void USART3_Configuration(void)
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

   /* Enable the USART2 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       //通道设置为串口3中断
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

   /* Enable the DMA Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;       
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;       
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

	/* 第1步：打开GPIO和USART3部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    /* Enable DMA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* 第2步：将USART3 Tx的GPIO配置为推挽复用模式 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 第3步：将USART3 Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*485器件发送使能和接收使能脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /* 输出 */
    GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* 第4步：配置USART3参数
	    - BaudRate = 115200 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - No parity
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 9600;//57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	/* 第5步：使能 USART3， 配置完毕 */
	USART_Cmd(USART3, ENABLE);

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    USART_ClearFlag(USART3, USART_FLAG_TC);     // 清标志

	//使能USART 接收DMA
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);

  /* 接收使能 */
  //GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	/* nRE = 0 */
  /* 发送禁止 */
  //GPIO_ResetBits(GPIOF, GPIO_Pin_10);	/* DE = 0 */ 
      GPIO_SetBits(GPIOF, GPIO_Pin_10);		
      GPIO_SetBits(GPIOF, GPIO_Pin_11); 	
      //GPIO_ResetBits(GPIOF, GPIO_Pin_10);		
      //GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	

   //接收DMA初始化
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C485DEVICE.DMARecvBuff);
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 100;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);

  DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);

  DMA_Cmd(DMA1_Channel3, ENABLE);

}

/*485串口初始化*/
void BSP_485Init()
{
  u8 err = 0;
  USART3_Configuration(); 
  BSP_InitFIFO(&(C485DEVICE.ReadFIFO));
  BSP_InitFIFO(&(C485DEVICE.WriteFIFO));
  C485DEVICE.active = 0;
  C485DEVICE.sendstate = 0;
  C485DEVICE.DMARecvUse = 0;
  C485DEVICE.WriteMutex = OSMutexCreate(9,&err);//OSSemCreate(1);
  C485DEVICE.ReadMutex = OSMutexCreate(9,&err);//OSSemCreate(1); 
  C485DEVICE.WriteFinishMutex = OSSemCreate(0); 
}

/*485串口打开*/
void BSP_485Open()
{
  C485DEVICE.active = 1;
}

/*485串口关闭*/
void BSP_485Close()
{
  C485DEVICE.active = 0;
}

/*485串口读*/
u16 BSP_485Read(u8* buff,u16 len)
{
   u16 retval = 0;
   INT8U  err;
   OSMutexPend(C485DEVICE.ReadMutex,0,&err);
   retval = BSP_485PopoutReadFIFO(&(C485DEVICE.ReadFIFO),buff,len);
   OSMutexPost(C485DEVICE.ReadMutex);
   return retval;
}

/*485串口读缓冲区中长度*/
u16 BSP_485InBuffLen()
{
   u16 retval;
   INT8U  err;
   OSMutexPend(C485DEVICE.ReadMutex,0,&err);
   DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,DISABLE);
   //接收数量大于已经消耗的数量
   if((100-DMA_GetCurrDataCounter(DMA1_Channel3)) > C485DEVICE.DMARecvUse)
   {
      u8 tt =  C485DEVICE.DMARecvUse;
	  C485DEVICE.DMARecvUse = 100-DMA_GetCurrDataCounter(DMA1_Channel3);
	  BSP_PushinFIFO(&(C485DEVICE.ReadFIFO),C485DEVICE.DMARecvBuff + tt,C485DEVICE.DMARecvUse - tt);
   }
   retval = BSP_FIFOLen(&(C485DEVICE.ReadFIFO));
   DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);
   OSMutexPost(C485DEVICE.ReadMutex);
   return retval;
}

/*485串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_485Write(u8* buff,u16 len)
{
 
  u16 retval = 0;
    INT8U  err;
	OSMutexPend(C485DEVICE.WriteMutex,0,&err);

  DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,DISABLE);

  retval = BSP_PushinFIFO(&(C485DEVICE.WriteFIFO),buff,len);

  //没有开始发送则启动发送
  if(C485DEVICE.sendstate == 0)
  {
      DMA_InitTypeDef DMA_InitStructure;
	  u16 sendlen = 0;
	  C485DEVICE.sendstate = 1;
 
      //GPIO_SetBits(GPIOF, GPIO_Pin_10);		
      //GPIO_SetBits(GPIOF, GPIO_Pin_11); 	
      //GPIO_SetBits(GPIOF, GPIO_Pin_10);		
      //GPIO_SetBits(GPIOF, GPIO_Pin_11); 	
      GPIO_ResetBits(GPIOF, GPIO_Pin_10);		
      GPIO_ResetBits(GPIOF, GPIO_Pin_11); 	
 
 	  if(BSP_FIFOLen(&(C485DEVICE.WriteFIFO)) > 100)
	  {
	    sendlen = BSP_PopoutFIFO(&(C485DEVICE.WriteFIFO),C485DEVICE.DMASendBuff,100);
	  }
	  else
	  {
	    sendlen = BSP_PopoutFIFO(&(C485DEVICE.WriteFIFO),C485DEVICE.DMASendBuff,BSP_FIFOLen(&(C485DEVICE.WriteFIFO)));
	  }
	  //使能USART 发送DMA
	  USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);

      DMA_Cmd(DMA1_Channel2, DISABLE);

      DMA_DeInit(DMA1_Channel2);
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(C485DEVICE.DMASendBuff);
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	  DMA_InitStructure.DMA_BufferSize = sendlen;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
      DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);
      DMA_Cmd(DMA1_Channel2, ENABLE);
  }

   DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);

   //等待写完成
   OSSemPend(C485DEVICE.WriteFinishMutex,0,&err);
   OSMutexPost(C485DEVICE.WriteMutex);
   return retval;
   
}

/*485串口驱动，在大循环中一直运行*/
void BSP_485Run(void)
{
  if(C485DEVICE.active == 1)
  {
  }
}

void BSP_485ClearReadBuff(void)
{
   INT8U  err;
   OSMutexPend(C485DEVICE.ReadMutex,0,&err);
   BSP_ClearFIFO(&(C485DEVICE.ReadFIFO));
   OSMutexPost(C485DEVICE.ReadMutex);
}
