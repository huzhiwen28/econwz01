#include "bsp_fifo.h"

/*初始化FIFO*/
u8 BSP_InitFIFO(struct _fifo* handle)
{
    handle->ip = 0;
	handle->op = 0;
	handle->fifol = 0;
	return 0;
}


/*返回0表示一切正常，其他值表示未放入队列的数量值*/
u16 BSP_PushinFIFO(struct _fifo* handle,u8* buff,u16 len)
{
  u16 index =0;
  for(index = 0;index < len; index++)
  {
    if((handle->fifol) < 500)
	{
      if((handle->ip) == 499)
      {
	   handle->FIFO[handle->ip] = buff[index];
	   handle->ip = 0;
	  }
	  else
	  {
  	   handle->FIFO[handle->ip] = buff[index];
	   (handle->ip)++;
	  }
	   (handle->fifol)++;
	}
	else
	{
	  return len -index;
	}
  }
  return 0; 
}

/*取出fifo中的值，返回长度*/
u16 BSP_PopoutFIFO(struct _fifo* handle,u8* buff,u16 len)
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
	  (handle->fifol)--;
	}
  }
  return retlen; 
}

/*返回长度*/
u16 BSP_FIFOLen(struct _fifo* handle)
{
   return handle->fifol;
}

u8 BSP_ClearFIFO(struct _fifo* handle)
{
    handle->ip = 0;
	handle->op = 0;
	handle->fifol = 0;
	return 0;
}
