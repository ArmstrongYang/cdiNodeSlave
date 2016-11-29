/*****************************************************************

File			:	YD-Module\BLE_DA14580\BLE.h
Fuction		:	
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "stm32f0xx_hal.h"

#define I2C_DATA_MAX	16 

#define COUNT(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

#define I2C_SLAVE_ADDRESS_HARDWARE			8
#define I2C_SLAVE_ADDRESS_SOFTWARE		


#define __HAL_DMA_SET_COUNTER(__HANDLE__, __COUNTER__) ((__HANDLE__)->Instance->CNDTR = (uint16_t)(__COUNTER__))
#define __HAL_DMA_GET_COUNTER(__HANDLE__) ((__HANDLE__)->Instance->CNDTR);

  

#endif
