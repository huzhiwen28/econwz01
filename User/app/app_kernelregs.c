#include <ucos_ii.h>
#include "app_kernelregs.h"
#include "bsp_eeprom.h"


volatile short KernelRegs[REGSNUM];
OS_EVENT *RegsWriteMutex;//写锁

char RegsFlag[REGSNUM];

//加密串6个字节的seed，和2个字节的crc码
u8 Encryption[8];

//写加密串标识	 0不写 1写
u8 WriteEncry;
