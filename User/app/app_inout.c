#include "stm32f10x.h"
#include "app_basicdef.h"
#include "app_pid.h"


void Out1DAC(uint DAC1)
{
/*
	uint A1;
	A1=0x1000|DAC1;
	Delay1us(2);
	DAC=0;
	SPI_SS_PORT=0;
	writeSPI2(A1);
	SPI_SS_PORT=1;
	DAC=1;
	Delay1us(2);
*/
}

void Out2DAC(uint DAC2)
{
/*	uint A2;
	A2=0x5000|DAC2;
	Delay1us(2);
	DAC=0;
	SPI_SS_PORT=0;
	writeSPI2(A2);
	SPI_SS_PORT=1;
	DAC=1;
	Delay1us(2);
*/
}

void Out3DAC(uint DAC3)
{
/*	uint A3;
	A3=0x9000|DAC3;
	Delay1us(2);
	DAC=0;
	SPI_SS_PORT=0;
	writeSPI2(A3);
	SPI_SS_PORT=1;
	DAC=1;
	Delay1us(2);
*/
}

void Out4DAC(uint DAC4)
{
/*	uint A4;
	A4=0xD000|DAC4;
	Delay1us(2);
	DAC=0;
	SPI_SS_PORT=0;
	writeSPI2(A4);
	SPI_SS_PORT=1;
	DAC=1;
	Delay1us(2);
*/
}

void InitPWM1(uint Speed )
{
}

void InitPWM2(uint Speed )
{
}


