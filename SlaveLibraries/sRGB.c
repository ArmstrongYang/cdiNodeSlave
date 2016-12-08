/**
****************************************************************

File	:	sLEDs.c
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	information of the file

*****************************************************************
*/

#include "sRGB.h"

void sExecuteComand(Device_DataType *device, I2C_DataType *I2C_Data)
{
	device->command = 0x04;
	switch(I2C_Data->cmd){
		case 0x40:	sRGB_Write(R,ON);
				break;
		case 0x41:	sRGB_Write(R,OFF);
				break;
		case 0x42:	sRGB_Write(G,ON);
				break;
		case 0x43:	sRGB_Write(G,OFF);
				break;
		case 0x44:	sRGB_Write(B,ON);
				break;
		case 0x45:	sRGB_Write(B,OFF);
				break;
		case 0x46:	sRGB_Toggle(R);
				break;
		default:
				break;
	}
}


/**
****************************************************************

Function：sRGB_Init
Author	:	@hiyangdong
Version	:	V1.0
date		:	2015-05-13
brief		:	initialize the RGB pins

*****************************************************************
*/
void sRGB_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	sRGB_Write(R,OFF);	
	sRGB_Write(G,OFF);	
	sRGB_Write(B,OFF);
}

/**
****************************************************************

Function：sLEDs_Write
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	write the RGB pins

*****************************************************************
*/
void sRGB_Write(RGB PinColor, LED_State LEDState)
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
		  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,PinState);
	}
	else if(PinColor==G)
	{
		  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,PinState);
	}	
	else if(PinColor==B)
	{
		  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,PinState);
	}
}

/**
****************************************************************

Function：sLEDs_Toggle
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	Toggle the RGB pins

*****************************************************************
*/
void sRGB_Toggle(RGB PinColor)
{
	if(PinColor==R)
	{
		  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_2);
	}
	else if(PinColor==G)
	{
		  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
	}	
	else if(PinColor==B)
	{
		  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_3);
	}
}
