/**
****************************************************************

File	:	sKey.h
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	information of the file

*****************************************************************
*/

#ifndef _SKEY_H_
#define _SKEY_H_

#include "main.h"
#include "cLEDs.h"
#include "sI2C.h"

#define SLAVE_HARD_ADDR				((uint16_t)0x0011)
#define SLAVE_SOFT_ADDR				((uint32_t)0x11010101)

#define KEY_GPIO  GPIOA
#define KEY_PIN 	GPIO_PIN_0

extern void sExecuteComand(Device_DataType *device, I2C_DataType *I2C_Data);
extern void sKey_Init(void);
extern GPIO_PinState sKey_Read(void);



#endif
