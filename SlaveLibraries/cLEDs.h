/**
****************************************************************

File		:	cLEDs.h
Author	:	@hiyangdong
Version	:	V1.0
date		:	2015-05-13
brief		:	information of the file

*****************************************************************
*/

#ifndef _CLEDS_H_
#define _CLEDS_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include "main.h"

typedef enum
{
  R = 0,
  G = 1,
  B = 2
}RGB;

typedef enum
{
  ON = 0,
  OFF
}LED_State;


#define cLEDs_R_GPIO  GPIOA
#define cLEDs_G_GPIO  GPIOA
#define cLEDs_B_GPIO  GPIOA
#define cLEDs_R_PIN 	GPIO_PIN_6
#define cLEDs_G_PIN 	GPIO_PIN_5
#define cLEDs_B_PIN 	GPIO_PIN_7

void cLEDs_Init(void);
void cLEDs_Write(RGB PinColor, LED_State LEDState);
void cLEDs_Toggle(RGB PinColor);

#ifdef __cplusplus
}
#endif

#endif
