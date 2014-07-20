#ifndef __EXTSRAM_H
#define __EXTSRAM_H
#include "stm32f10x.h"


/*外扩sram地址*/
#define EXT_SRAM_ADDR    ((u32)0x60000000)


#define fpga_write(offset, data) *((volatile u8*)(EXT_SRAM_ADDR + (offset << 16))) = data

#define fpga_read(offset)       *((volatile u8*)(EXT_SRAM_ADDR + (offset << 16)))

/*内存读写方法
  Addr 为偏移地址，0-12287(512K字节)
 
  写内存例子: *(u16 *) (EXT_SRAM_ADDR + Addr) = 1;
  读内存例子: u16 val = *(__IO u16*) (EXT_SRAM_ADDR + Addr);
 
 
 具体内存中存放的信息是什么，由使用者自己自由使用分配，请灵活使用指针
*/
/*外扩sram初始化*/
void BSP_ExtSramInit(void);

//设置马达速度
//number轴号 0对应起始马达 
//speed mm/s 马达速度
void MotorSetSpeed(int number,double speed);

//马达速度生效，这样可以保证所有马达速度一致的刷新
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达速度值刷新
void MotorSpeedFlush(u8 bits);

//马达PWM输出生效
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达输出脉冲,0表示对应的马达停止输出脉冲
void MotorPwmout(u8 bits);

//马达当前脉冲数量刷新，便于stm32读马达的脉冲数量
//bits中的位对应相应的马达，bit0-7表示马达1至马达8，位值为1表示对应的马达脉冲数量刷新
void MotorOutCntFlush(u8 bits);

//读取马达当前发送的脉冲数量
//number轴号 0对应起始马达 
int MotorReadOutCnt(int number);

void Motorpingpong(void);
#endif

