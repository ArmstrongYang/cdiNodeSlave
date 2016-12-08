/**
****************************************************************

File	:	sKey.c
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	information of the file

*****************************************************************
*/

#include "sKey.h"

/**
****************************************************************

Function：	sExecuteComand
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	slave module command execute api

*****************************************************************
*/
void sExecuteComand(Device_DataType *device, I2C_DataType *I2C_Data)
{

	switch(I2C_Data->cmd){
		case 0x40:	
			device->command = 0x04;
			if(sKey_Read()==GPIO_PIN_SET)
				device->value = 1;
			else
				device->value = 0;
				break;
		default:
				break;
	}
}


/**
****************************************************************

Function：	cKey_Init
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	initialize the RGB pins

*****************************************************************
*/
void sKey_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : KEY_PIN */
  GPIO_InitStruct.Pin = KEY_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(KEY_GPIO, &GPIO_InitStruct);

	deviceMe.type = SLAVE_HARD_ADDR;
	deviceMe.address = SLAVE_SOFT_ADDR;
	deviceMe.command = 0;
	deviceMe.value = 0;
	deviceMe.state = 1;
}

/**
****************************************************************

Function：	sKey_Read
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	write the RGB pins

*****************************************************************
*/
GPIO_PinState sKey_Read(void)
{
	
	return HAL_GPIO_ReadPin(KEY_GPIO,KEY_PIN);
	
}

