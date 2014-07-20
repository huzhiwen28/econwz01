#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_enc.h"
#include "stm32f10x_encoder.h"
int64_t* BSP_ENC_CNT;
struct _ENC ENCDEVICE;

/*ENC初始化*/
void BSP_ENCInit(void)
{
  ENC_Init();
  ENCDEVICE.currentCount = 0;
  BSP_ENC_CNT = &(ENCDEVICE.currentCount);
}

/*ENC打开*/
void BSP_ENCOpen(void)
{
  ENCDEVICE.active = 1;
}

/*ENC关闭*/
void BSP_ENCClose(void)
{
  ENCDEVICE.active = 0;
}

/*ENC运行驱动*/
void BSP_ENCRun(void *p_arg)
{

  while(1)
  {
	  if(ENCDEVICE.active == 1)
	  {
	    ENCDEVICE.currentCount += Enc_GetCount();
	  }
	OSTimeDlyHMSM(0, 0, 0, 10);
  }
}


