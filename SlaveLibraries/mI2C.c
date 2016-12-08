/*****************************************************************

File			:	YD-Module\dataStruct\dataStruct.c
Fuction		:	Power management chip
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/
#include "mI2C.h"


osMutexId mI2C_Handle;


uint8_t device_write(DeviceDataType *device)
{
	
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_I2C_StateTypeDef i2c_state;
	I2C_DataType i2c_data;
	
	i2c_data = update_I2CFrame(device);
	osMutexWait(mI2C_Handle,osWaitForever);
	i2c_state = HAL_I2C_GetState(&hi2c1); 
	if(i2c_state==HAL_I2C_STATE_READY||i2c_state==HAL_I2C_STATE_BUSY_RX) { 
		while(status!=HAL_OK){
			status = HAL_I2C_Master_Transmit_DMA(&hi2c1, i2c_data.addr_hard, i2c_data.frame,i2c_data.size);
			if(status!=HAL_OK)
				osDelay(100);
		}
	}
	osMutexRelease(mI2C_Handle);
	return status;
}
uint8_t device_read(DeviceDataType *device)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_I2C_StateTypeDef i2c_state;
	I2C_DataType i2c_data;
	
	i2c_data = update_I2CFrame(device);
	osMutexWait(mI2C_Handle,osWaitForever);
	i2c_state = HAL_I2C_GetState(&hi2c1);  
	if(i2c_state==HAL_I2C_STATE_READY||i2c_state==HAL_I2C_STATE_BUSY_RX){
		while(status != HAL_OK){
			status = HAL_I2C_Master_Transmit_DMA(&hi2c1, i2c_data.addr_hard, i2c_data.frame, i2c_data.size);
			if(status!=HAL_OK)
				osDelay(100);
		}
	}
	osDelay(100);
	status = HAL_I2C_Master_Receive_DMA(&hi2c1, i2c_data.addr_hard, i2c_data.frame,i2c_data.size);
	osDelay(100);
	device->value = *(int*)(i2c_data.frame+I2C_DATA_INDEX+2);
	if(status != HAL_OK){
		i2c_state = HAL_I2C_GetState(&hi2c1);  
		if(i2c_state != HAL_I2C_ERROR_AF)
    {}
	}
	osMutexRelease(mI2C_Handle);
	return status;
}

////# 2 I2C receive the return data
//void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	I2C_Data.status = HAL_I2C_Master_Receive_DMA(&hi2c1, I2C_Data.addr_hard, I2C_Data.data_read,I2C_FRAME_SIZE);
//}

////# 3 I2C received the return data and tranfer to UART
//void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	memcpy(UART_Data.data_send,I2C_Data.data_read,I2C_FRAME_SIZE);
//	I2C_Data.status = HAL_UART_Transmit_DMA(&huart1,UART_Data.data_send,I2C_FRAME_SIZE);
//}

////# 4 UART start new receiving
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)  
//{
//	
//	I2C_Data.status = HAL_UART_Receive_DMA(&huart1,UART_Data.data_read,I2C_FRAME_SIZE);
//	
//}
