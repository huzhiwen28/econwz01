#ifndef __KERNELREGS_H
#define __KERNELREGS_H
#include <ucos_ii.h>
#include "stm32f10x.h"

//需要记忆的内存数量
#define NEEDSAVENUM 71

//每个控制的寄存器数量
#define REGSNUM 111

//控制器寄存器
extern volatile short KernelRegs[];
extern OS_EVENT *RegsWriteMutex;//写锁
//加密串6个字节的seed，和2个字节的crc码
extern u8 Encryption[8];

//写加密串标识	 0不写 1写
extern u8 WriteEncry;


//寄存器状态修改标识 0没改变 1改变
extern char RegsFlag[];


//寄存器内容初始化
void App_RegsInit(void);

//寄存器内容后台刷新运行
void App_RgesRun(void *p_arg);

void  ParamInit(void);

#endif
