/******************************************************************

File		:	sLEDs.h
Author	:	@hiyangdong
Version	:	V1.0
date		:	2015-05-13
brief		:	information of the file

******************************************************************/

#ifndef _SRGB_H_
#define _SRGB_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "cLEDs.h"
#include "sI2C.h"

#define SLAVE_HARD_ADDR				((uint16_t)0x0010)
#define SLAVE_SOFT_ADDR				((uint32_t)0x10010101)

#define sRGB_R_GPIO  GPIOA
#define sRGB_G_GPIO  GPIOA
#define sRGB_B_GPIO  GPIOA
#define sRGB_R_PIN 	 GPIO_PIN_1
#define sRGB_G_PIN 	 GPIO_PIN_2
#define sRGB_B_PIN 	 GPIO_PIN_3
	 
extern void sExecuteComand(Device_DataType *device, I2C_DataType *I2C_Data);

void sRGB_Init(void);
void sRGB_Write(RGB PinColor, LED_State LEDState);
void sRGB_Toggle(RGB PinColor);

#ifdef __cplusplus
}
#endif


#endif
