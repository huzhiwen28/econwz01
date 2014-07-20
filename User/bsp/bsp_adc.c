#include "stm32f10x.h"
#include "bsp_adc.h"
#include "stm32f10x_dma.h"
#define ADC1_DR_Address    ((u32)0x4001244C)

 struct _ADC ADCDEVICE;

/*ADC数据指针，作为外部访问的接口*/
u16* BSP_ADC1;
u16* BSP_ADC2;
u16* BSP_ADC3;
u16* BSP_ADC4;

/*******************************************************************************
	函数名：ADC_Configuration
	输  入:
	输  出:
	功能说明：
*/
static void ADC_Configuration(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable DMA clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);              //72M/6=12,ADC最大时间不能超过14M

  /* Configure PC.0、PC.1、PC.2、PC.3 as analog input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;//外设地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(ADCDEVICE.ADCDataTab);//内存地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//dma传输方向单向
  DMA_InitStructure.DMA_BufferSize = 4*30;//设置DMA在传输时缓冲区的大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置DMA的外设递增模式，一个外设
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置DMA的内存递增模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据字长
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//内存数据字长
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//设置DMA的传输模式：连续不断的循环模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//设置DMA的优先级别
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//设置DMA的2个memory中的变量互相访问
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//独立工作模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//扫描方式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//外部触发禁止
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据右对齐
  ADC_InitStructure.ADC_NbrOfChannel = 4;//用于转换的通道数
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channels configuration [规则模式通道配置]*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道10,规则采样顺序值为1,采样时间为239.5周期
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道11,规则采样顺序值为2,采样时间为239.5周期
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道12,规则采样顺序值为3,采样时间为239.5周期
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5); //ADC1,ADC通道13,规则采样顺序值为4,采样时间为239.5周期

  /* Enable ADC1 DMA [使能ADC1 DMA]*/
  ADC_DMACmd(ADC1, ENABLE);
 
  /* Enable ADC1 [使能ADC1]*/
  ADC_Cmd(ADC1, ENABLE); 

  /* Enable ADC1 reset calibaration register */  
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);		  //注意此句的位置，网上有反映如果把此句放在ADC1配置和校准之前，会出现错位现象
   
  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/*ADC初始化*/
void BSP_ADCInit(void)
{
  ADC_Configuration();

  BSP_ADC1 = ADCDEVICE.ADCDataTab;
  BSP_ADC2 = ADCDEVICE.ADCDataTab + 1;
  BSP_ADC3 = ADCDEVICE.ADCDataTab + 2;
  BSP_ADC4 = ADCDEVICE.ADCDataTab + 3;
}

/*ADC打开*/
void BSP_ADCOpen(void)
{
  ADCDEVICE.active = 1;
}

/*ADC关闭*/
void BSP_ADCClose(void)
{
  ADCDEVICE.active = 0;
}

