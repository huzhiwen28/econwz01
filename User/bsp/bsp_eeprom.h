#ifndef __EEPROM_H
#define __EEPROM_H 

/* Includes ------------------------------------------------------------------*/	   
#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_fifo.h"
/*EEPROM通过I2C连接
  使用IO模拟方法读写eeprom
  移植自冰凌科技
*/

/*EEPROM设备*/
struct _EEPROM{
	  /*设备是否打开*/
	  u8 active;
      OS_EVENT *IICMutex;//写锁
};

/*EEPROM初始化*/
void BSP_EEPROMInit(void);

/*EEPROM打开*/
void BSP_EEPROMOpen(void);

/*EEPROM关闭*/
void BSP_EEPROMClose(void);

/*******************************************************************************
* Function Name  : BSP_EEPROMWriteByte
* Description    : 写一字节数据
* Input          : - SendByte: 待写入数据
*           	   - WriteAddress: 待写入地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功写入,=0失败
*******************************************************************************/           
u8 BSP_EEPROMWriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress);	 //


/*******************************************************************************
* Function Name  : BSP_EEPROMReadByte
* Description    : 读取一串数据
* Input          : - pBuffer: 存放读出数据
*           	   - length: 待读出长度
*                  - ReadAddress: 待读出地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功读入,=0失败
*******************************************************************************/          
u8 BSP_EEPROMReadByte(u8* pBuffer,   u16 length,   u16 ReadAddress,  u8 DeviceAddress);

/*启动时读取eeprom接口用*/
u8 BSP_EEPROMReadByte2(u8* pBuffer,   u16 length,   u16 ReadAddress,  u8 DeviceAddress);

u8 SendWordEEPROM(u16 Adss,u16 SData);
u16 ReadWordEEPROM(u16 Adss);

#endif

