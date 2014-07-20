
#include "stm32f10x.h"
#include "bsp_fifo.h"
#include "bsp_usb.h"
//#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"
//#include "platform_config.h"
#include "bsp.h"

void USBCOM_Newmsg(void);

struct _usb USBDEVICE;

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  	USB_Istr();
}


/*取出fifo中的值，返回长度*/
u16 BSP_USBPopoutReadFIFO(struct _fifo* handle,u8* buff,u16 len)
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
	   		__set_PRIMASK(1);  		/* 关中断 */
	  		(handle->fifol)--;
	   		__set_PRIMASK(0);  		/* 开中断 */
		}
  	}
  	return retlen; 
}


/*USB串口初始化*/
void BSP_USBInit()
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);    //允许AFIO时钟

  	/* Enable USB_DISCONNECT GPIO clock */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

   	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
         | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
         | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

  	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
 
  	/* Configure USB pull-up pin */
  	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

    GPIO_ResetBits(USB_DISCONNECT,USB_DISCONNECT_PIN);

  	Set_USBClock();
  	USB_Interrupts_Config();
  	USB_Init();
  	BSP_InitFIFO(&(USBDEVICE.ReadFIFO));
  	USBDEVICE.active = 0;
 }

/*USB串口打开*/
void BSP_USBOpen()
{
  	u8 err = 0;
  	USBDEVICE.active = 1;
  	USBDEVICE.WriteMutex = OSMutexCreate(9,&err);//OSSemCreate(1);
  	USBDEVICE.ReadMutex = OSMutexCreate(9,&err);//OSSemCreate(1); 
}

/*USB串口关闭*/
void BSP_USBClose()
{
  	USBDEVICE.active = 0;
}

/*USB串口读*/
u16 BSP_USBRead(u8* buff,u16 len)
{
   	u16 retval = 0;
   	INT8U  err;
   	OSMutexPend(USBDEVICE.ReadMutex,0,&err);
	retval = BSP_USBPopoutReadFIFO(&(USBDEVICE.ReadFIFO),buff,len);
   	OSMutexPost(USBDEVICE.ReadMutex);
   	return retval;
}

/*USB串口读缓冲区中长度*/
u16 BSP_USBInBuffLen()
{
	u16 retval;
   	INT8U  err;
    OS_CPU_SR  cpu_sr = 0;
   	OSMutexPend(USBDEVICE.ReadMutex,0,&err);
   	OS_ENTER_CRITICAL();
  	retval = 	BSP_FIFOLen(&(USBDEVICE.ReadFIFO));
   	OS_EXIT_CRITICAL();
   	OSMutexPost(USBDEVICE.ReadMutex);
  	return   retval;
}

/*USB串口写,返回0表示一切正常,其他值表示未放入队列的数量值*/
u16 BSP_USBWrite(u8* buff,u16 len)
{
  	u16 index;
   	INT8U  err;
   	OSMutexPend(USBDEVICE.WriteMutex,0,&err);
  	for(index = 0; index < len; index++ )
  	{
     	USART_To_USB_Send_Data(buff[index]);
  	}
   	OSMutexPost(USBDEVICE.WriteMutex);
  	return 0;
}

/*USB驱动通知消费者新的字符收到*/
void BSP_USBNewRecv(void)
{
	USBCOM_Newmsg();
}

