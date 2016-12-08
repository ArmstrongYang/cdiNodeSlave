/**
****************************************************************

File	:	cLEDs.c
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	information of the file

*****************************************************************
*/

#include "cLEDs.h"

/**
****************************************************************

Function：cLEDs_Init
Author	:	@hiyangdong
Version	:	V2.0
date		:	2016-07-17 22:47:33
brief		:	initialize the RGB pins, add the cLEDs_R_GPIO et al.

*****************************************************************
*/
void cLEDs_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = cLEDs_R_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(cLEDs_R_GPIO, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = cLEDs_G_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(cLEDs_G_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = cLEDs_B_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(cLEDs_B_GPIO, &GPIO_InitStruct);
	
	cLEDs_Write(R,OFF);	
	cLEDs_Write(G,OFF);	
	cLEDs_Write(B,OFF);	

}

/**
****************************************************************

Function：cLEDs_Write
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	write the RGB pins

*****************************************************************
*/
void cLEDs_Write(RGB PinColor, LED_State LEDState)
{
	GPIO_PinState PinState;
	if(LEDState==0)
	{
		PinState = GPIO_PIN_RESET;
	}
	else
		PinState = GPIO_PIN_SET;
	
	if(PinColor==R)
	{
		  HAL_GPIO_WritePin(cLEDs_R_GPIO,cLEDs_R_PIN,PinState);
	}
	else if(PinColor==G)
	{
		  HAL_GPIO_WritePin(cLEDs_G_GPIO,cLEDs_G_PIN,PinState);
	}	
	else if(PinColor==B)
	{
		  HAL_GPIO_WritePin(cLEDs_B_GPIO,cLEDs_B_PIN,PinState);
	}
}

/**
****************************************************************

Function：	cLEDs_Toggle
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	Toggle the RGB pins

*****************************************************************
*/
void cLEDs_Toggle(RGB PinColor)
{
	if(PinColor==R)
	{
		  HAL_GPIO_TogglePin(cLEDs_R_GPIO,cLEDs_R_PIN);
	}
	else if(PinColor==G)
	{
		  HAL_GPIO_TogglePin(cLEDs_G_GPIO,cLEDs_G_PIN);
	}	
	else if(PinColor==B)
	{
		  HAL_GPIO_TogglePin(cLEDs_B_GPIO,cLEDs_B_PIN);
	}
}

