/*****************************************************************

File			:	YD-Module\BLE_DA14580\BLE.h
Fuction		:	
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/

#ifndef _MI2C_H_
#define _MI2C_H_

#include "main.h"

extern osMutexId mI2C_Handle;

extern uint8_t device_write(Device_DataType *device);
extern uint8_t device_read(Device_DataType *device);


#endif
