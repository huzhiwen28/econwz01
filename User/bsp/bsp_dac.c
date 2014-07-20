#include <ucos_ii.h>
#include "stm32f10x.h"
#include "bsp_dac.h"
#include "stm32f10x_spi.h"
#include "app_kernelregs.h"
#include "app_basicdef.h"

struct _DAC DACDEVICE;
u16 DacDeviceControlCh1;
u16 DacDeviceControlCh2;

/* Defines for the SPI and GPIO pins used to drive the SPI DAC */
#define SPI_DAC                 SPI2
#define SPI_DAC_CLK             RCC_APB1Periph_SPI2
#define SPI_DAC_GPIO            GPIOB
#define SPI_DAC_GPIO_CLK        RCC_APB2Periph_GPIOB
#define SPI_DAC_PIN_SCK         GPIO_Pin_13
#define SPI_DAC_PIN_MISO        GPIO_Pin_14
#define SPI_DAC_PIN_MOSI        GPIO_Pin_15

#if defined (USE_STM3210B_EVAL)
 #define SPI_DAC_CS             GPIO_Pin_12
 #define SPI_DAC_CS_GPIO        GPIOB
 #define SPI_DAC_CS_GPIO_CLK    RCC_APB2Periph_GPIOB
#elif defined (USE_STM3210E_EVAL)
 #define SPI_DAC_CS             GPIO_Pin_12
 #define SPI_DAC_CS_GPIO        GPIOB
 #define SPI_DAC_CS_GPIO_CLK    RCC_APB2Periph_GPIOB
#endif

#define SPI_DACDRV_CS             GPIO_Pin_4
#define SPI_DACDRV_CS_GPIO        GPIOA
#define SPI_DACDRV_CS_GPIO_CLK    RCC_APB2Periph_GPIOA

//5750 PA10

/* Exported macro ------------------------------------------------------------*/
/* Select SPI DAC: Chip Select pin low  */
#define SPI_DAC_CS_LOW()       GPIO_ResetBits(SPI_DAC_CS_GPIO, SPI_DAC_CS)
/* Deselect SPI DAC: Chip Select pin high */
#define SPI_DAC_CS_HIGH()      GPIO_SetBits(SPI_DAC_CS_GPIO, SPI_DAC_CS)

/* Select SPI DAC: Chip Select pin low  */
#define SPI_DACDRV_CS_LOW()       GPIO_ResetBits(SPI_DACDRV_CS_GPIO, SPI_DACDRV_CS)
/* Deselect SPI DAC: Chip Select pin high */
#define SPI_DACDRV_CS_HIGH()      GPIO_SetBits(SPI_DACDRV_CS_GPIO, SPI_DACDRV_CS)


