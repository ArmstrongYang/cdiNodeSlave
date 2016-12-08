/*****************************************************************

File			:	YD-Module\dataStruct\dataStruct.c
Fuction		:	Power management chip
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/
#include "cI2C.h"

I2C_StateType i2c_State;
I2C_DataType i2c_Data;

uint8_t device_write(Device_DataType *device)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t timeout = 10;
	
	//检查i2c硬件状态
	//	osMutexWait(cI2C_Handle,osWaitForever);
	printf("-----------------------device_write\r\n");
	i2cdata_update(device, &i2c_Data);
	printf("i2c_State_updated\r\n");
	while(HAL_Status!=HAL_OK)
	{
		if(__HAL_I2C_GET_FLAG(&BUS_I2C, I2C_FLAG_BUSY)==SET || i2c_State.error==I2C_ERROR_MAX)
		{
			i2c_State.error++;
			HAL_I2C_DeInit(&hi2c1);        	//释放IO口为GPIO，复位句柄状态标志
			MX_I2C1_Init();          				//重新初始化I2C控制器
			printf("===============I2C_FLAG_BUSY re init\r\n");
			osDelay(10);
		}
		
		if(i2c_State.error>=2*I2C_ERROR_MAX)
		{
			i2c_State.error=0;
			HAL_I2C_DeInit(&hi2c1);        //释放IO口为GPIO，复位句柄状态标志
			MX_I2C1_Init();
			printf("I2C re init fail------------\r\n");
			break;
		}
		
		HAL_Status = HAL_I2C_Master_Transmit(&BUS_I2C,(i2c_Data.addr_hard<<1),i2c_Data.frame,i2c_Data.size, timeout);
		if(HAL_Status==HAL_OK)
		{
//				osMutexRelease(cI2C_Handle);
			i2c_State.error=0;
			return true;
		}
	}
//					osMutexRelease(cI2C_Handle);
	return false;
}

uint8_t device_read(Device_DataType *device)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t timeout = 10;
	
	//检查i2c硬件状态
	//	osMutexWait(cI2C_Handle,osWaitForever);
	printf("-----------------------device_write\r\n");
	i2cdata_update(device, &i2c_Data);
	printf("i2c_State_updated\r\n");
	while(HAL_Status!=HAL_OK)
	{
		if(__HAL_I2C_GET_FLAG(&BUS_I2C, I2C_FLAG_BUSY)==SET || i2c_State.error==I2C_ERROR_MAX)
		{
			i2c_State.error++;
			HAL_I2C_DeInit(&hi2c1);        	//释放IO口为GPIO，复位句柄状态标志
			MX_I2C1_Init();          				//重新初始化I2C控制器
			printf("===============I2C_FLAG_BUSY re init\r\n");
			osDelay(10);
		}
		
		if(i2c_State.error>=2*I2C_ERROR_MAX)
		{
			i2c_State.error=0;
			HAL_I2C_DeInit(&hi2c1);        //释放IO口为GPIO，复位句柄状态标志
			MX_I2C1_Init();
			printf("I2C re init fail------------\r\n");
			break;
		}
		
		HAL_Status = HAL_I2C_Master_Transmit(&BUS_I2C,(i2c_Data.addr_hard<<1),i2c_Data.frame,i2c_Data.size, timeout);
		if(HAL_Status==HAL_OK)
		{
				printf("-------------------HAL_OK 1\r\n");
				osDelay(10);
				HAL_Status = HAL_I2C_Master_Receive(&hi2c1, (i2c_Data.addr_hard<<1), i2c_Data.frame, i2c_Data.size, timeout);
				i2cdata_parse(device, &i2c_Data);
				if(HAL_Status==HAL_OK)
				{
					printf("-------------------HAL_OK 2\r\n");
					i2c_State.error=0;
					//osMutexRelease(cI2C_Handle);
					return true;
				}
		}
	}
//	osMutexRelease(cI2C_Handle);
	return false;
}



//2016-12-08 16:38:18
uint8_t device_readold_version(Device_DataType *device)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t timeout = 10;
	
	//检查i2c硬件状态
	//	osMutexWait(cI2C_Handle,osWaitForever);
	if(HAL_I2C_GetState(&hi2c1)==HAL_I2C_STATE_READY)
	{
		HAL_UART_Transmit(&huart1 , (uint8_t *)"--------------------", 20, 100);  
		i2cdata_update(device, &i2c_Data);
		printf("i2cdata_updated\r\n");
		for(char i=0;i<i2c_Data.size;i++)
		{
			printf("%x ",i2c_Data.frame[i]);
		}
		while(HAL_Status!=HAL_OK)
		{
			printf("xxxxxxxxxxxxxxxxxxxxxxxr\n");
			if(__HAL_I2C_GET_FLAG(&BUS_I2C, I2C_FLAG_BUSY) == SET)
			{
				printf("I2C_FLAG_BUSY\r\n");
				HAL_I2C_DeInit(&hi2c1);        //释放IO口为GPIO，复位句柄状态标志
				MX_I2C1_Init();          //这句重新初始化I2C控制器
				printf("I2C_FLAG_BUSY re inixxxxxxxxxxxx\r\n");
			}

			HAL_Status = HAL_I2C_Master_Transmit(&BUS_I2C, i2c_Data.addr_hard, i2c_Data.frame,i2c_Data.size, timeout);
			if(HAL_Status==HAL_OK)
			{
//				osMutexRelease(cI2C_Handle);
				printf("-------------------HAL_OK 1\r\n");
				osDelay(10);
				HAL_Status = HAL_I2C_Master_Receive(&hi2c1, i2c_Data.addr_hard, i2c_Data.frame, i2c_Data.size, timeout);
				i2cdata_parse(device, &i2c_Data);
				if(HAL_Status==HAL_OK)
				{
					printf("-------------------HAL_OK 2\r\n");
					
					device->value = *(float*)(i2c_Data.frame+I2C_DATA_INDEX+2);
					i2c_State.error=0;
					return true;
				}
			}
			else
			{
				i2c_State.error++;
				printf("i2c_State.error: %d\r\n",i2c_State.error);
				if(i2c_State.error==I2C_ERROR_MAX)
				{
					HAL_I2C_DeInit(&hi2c1);        //释放IO口为GPIO，复位句柄状态标志
					MX_I2C1_Init();
					printf("I2C re init: %d\r\n",HAL_Status);
					osDelay(10);
				}
				if(i2c_State.error>=2*I2C_ERROR_MAX)
				{
					i2c_State.error=0;
					HAL_I2C_DeInit(&hi2c1);        //释放IO口为GPIO，复位句柄状态标志
					MX_I2C1_Init();
					printf("I2C re init fail------------\r\n");
//					osMutexRelease(cI2C_Handle);
					return false;
				}
			}
		}
	}
	printf("I2C state not ready:0x%x\r\n", BUS_I2C.State);
//	osMutexRelease(cI2C_Handle);
	return false;
}

uint8_t device_readx(Device_DataType *device)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_I2C_StateTypeDef i2c_state;
	I2C_DataType i2c_data;
	
//	i2c_data = update_I2CFrame(device);
	osMutexWait(cI2C_Handle,osWaitForever);
	i2c_state = HAL_I2C_GetState(&hi2c1);  
	if(i2c_state==HAL_I2C_STATE_READY||i2c_state==HAL_I2C_STATE_BUSY_RX){
		while(status != HAL_OK){
			printf("device_read starus:%d\r\n",status);
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
	osMutexRelease(cI2C_Handle);
	return status;
}

uint8_t device_write_old(Device_DataType *device)
{
	
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_I2C_StateTypeDef i2c_state;
	I2C_DataType i2c_data;
	
//	i2c_data = update_I2CFrame(device);
//	osMutexWait(cI2C_Handle,osWaitForever);
	i2c_state = HAL_I2C_GetState(&hi2c1); 
	if(i2c_state==HAL_I2C_STATE_READY||i2c_state==HAL_I2C_STATE_BUSY_RX){ 
		while(status!=HAL_OK){
			printf("device_write starus1:%d\r\n",status);
			HAL_UART_Transmit(&huart1 , (uint8_t *)"--------------------", 20, 100);  
//			HAL_UART_Transmit(&huart1 , i2c_data.frame, i2c_data.size, 100);  
			 printf("addr:0x%x\r\n", i2c_data.addr_hard);
			status = HAL_I2C_Master_Transmit_DMA(&hi2c1, i2c_data.addr_hard, i2c_data.frame,i2c_data.size);
			printf("22:%d\r\n",status);
			if(status!=HAL_OK)
			{		printf("33:%d\r\n",status);
				i2c_State.error++;
				if(i2c_State.error>I2C_ERROR_MAX)
				{
					MX_I2C1_Init();
					printf("I2C re init------------\r\n");
					osDelay(1000);
				}
//				HAL_I2C_Master_Transmit_DMA(&hi2c1, i2c_data.addr_hard, i2c_data.frame,i2c_data.size);
				osDelay(100);
			}
		}
	}
//	osMutexRelease(cI2C_Handle);
	printf("44:%d\r\n",status);
	return status;
}

uint8_t device_readold(Device_DataType *device)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_I2C_StateTypeDef i2c_state;
	I2C_DataType i2c_data;
	
//	i2c_data = update_I2CFrame(device);
	osMutexWait(cI2C_Handle,osWaitForever);
	i2c_state = HAL_I2C_GetState(&hi2c1);  
	if(i2c_state==HAL_I2C_STATE_READY||i2c_state==HAL_I2C_STATE_BUSY_RX){
		while(status != HAL_OK){
			printf("device_read starus:%d\r\n",status);
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
	osMutexRelease(cI2C_Handle);
	return status;
}


#define CRC_POLY16 0x1021

uint16_t crc16(uint8_t *buf, uint16_t length)
{
  uint16_t data, val;
  uint16_t i;

  uint16_t shift = 0xFFFF;

  for(i=0;i<length;i++) 
	{
    if((i % 8) == 0)
      data = (*buf++)<<8;
    val = shift ^ data;
    shift = shift<<1;
    data = data <<1;
    if(val&0x8000)
      shift = shift ^ CRC_POLY16;
  }
  return shift;
}

uint8_t bufferCmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint8_t BufferLength)
{
	BufferLength++;
  while (BufferLength--)
  {
    if ((*pBuffer1) != *pBuffer2)
    {
      return BufferLength;
    }
    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}


//由于ARM默认是小端模式，而串口和I2C传输的模式是大端模式，对于uint16_t，uint32_t，float等数据适用
//memcpy采用大端模式，对于uint16_t，uint32_t，float等数据不适用，适用于uint8_t和char等单字节数据
void *i2cdata_cpy( void *dest, const void *src, size_t size)
{
	if((dest == NULL) || (src == NULL))	
		return NULL;
	char *tempsrc = ((char *)src + size);
	char *tempdest = (char *)dest;	
	while(size -- > 0)	
		*tempdest++ = *--tempsrc ;
	return dest;
}

void i2cdata_init(I2C_DataType *I2C_Data)
{
	memset(I2C_Data->frame,0,I2C_FRAME_SIZE);
//	memset(I2C_Data->data_read,0,I2C_FRAME_SIZE);
//	memset(I2C_Data->data_send,0,I2C_FRAME_SIZE);

	I2C_Data->flag = 0;
	I2C_Data->addr_hard = 0;
	I2C_Data->addr_soft = 0;
	I2C_Data->size = 0;
	I2C_Data->cmd = 0;
	memset(I2C_Data->data,0,I2C_DATA_SIZE);
	I2C_Data->crc=0;
}

void i2cdata_update(Device_DataType *device, I2C_DataType *I2C_Data)
{
	i2cdata_init(I2C_Data);
	
	I2C_Data->addr_hard = device->type;		//地址只在发送时左移一位，其他过程和excel保持一致
	I2C_Data->addr_soft = device->address;
	I2C_Data->size = I2C_FRAME_SIZE;
	I2C_Data->cmd = device->command;
	I2C_Data->value = device->value;
	i2cdata_cpy(I2C_Data->data+I2C_DATA_SIZE-sizeof(device->value), &device->value, sizeof(device->value));

	i2cdata_cpy(I2C_Data->frame+I2C_HARD_INDEX, &I2C_Data->addr_hard, sizeof(I2C_Data->addr_hard));
	i2cdata_cpy(I2C_Data->frame+I2C_SOFT_INDEX, &I2C_Data->addr_soft, sizeof(I2C_Data->addr_soft));
	i2cdata_cpy(I2C_Data->frame+I2C_SIZE_INDEX, &I2C_Data->size, sizeof(I2C_Data->size));
	i2cdata_cpy(I2C_Data->frame+I2C_CMD_INDEX, &I2C_Data->cmd, sizeof(I2C_Data->cmd));
	memcpy(I2C_Data->frame+I2C_DATA_INDEX, I2C_Data->data, sizeof(I2C_Data->data));
	I2C_Data->crc = crc16(I2C_Data->frame, I2C_FRAME_SIZE - I2C_CRC_SIZE);
	i2cdata_cpy(I2C_Data->frame + I2C_CRC_INDEX, &I2C_Data->crc, sizeof(I2C_Data->crc));
	
	I2C_Data->flag = 1;
}

void i2cdata_parse(Device_DataType *device, I2C_DataType *I2C_Data)
{
	uint8_t *pframe = I2C_Data->frame;		//默认是：I2C_Data->frame,此外还有I2C_Data->data_read和I2C_Data->data_send

	I2C_Data->flag = 0;
	i2cdata_cpy(&I2C_Data->addr_hard, pframe + I2C_HARD_INDEX, sizeof(I2C_Data->addr_hard));
	i2cdata_cpy(&I2C_Data->addr_soft, pframe + I2C_SOFT_INDEX, sizeof(I2C_Data->addr_soft));
	I2C_Data->size = pframe[I2C_SIZE_INDEX];
	I2C_Data->cmd = pframe[I2C_CMD_INDEX];
	memcpy(&I2C_Data->data, pframe + I2C_DATA_INDEX, sizeof(I2C_Data->data));
	i2cdata_cpy(&I2C_Data->value, I2C_Data->data+I2C_DATA_SIZE-sizeof(I2C_Data->value), sizeof(I2C_Data->value));
	i2cdata_cpy(&I2C_Data->crc, pframe + I2C_CRC_INDEX, sizeof(I2C_Data->crc));

	device->type = I2C_Data->addr_hard;
	device->address = I2C_Data->addr_soft;
	I2C_Data->size = I2C_FRAME_SIZE;
	device->command = I2C_Data->cmd;
	device->value = I2C_Data->value;
	
	I2C_Data->flag = 1;
}

void i2cdata_write(I2C_DataType *I2C_Data, uint16_t device_type, uint32_t device_address, uint8_t device_command, float device_value)
{
	Device_DataType device;
	
	device.type = device_type;
	device.address = device_address;
	device.command = device_command;
	device.value = device_value;

	i2cdata_update(&device, I2C_Data);
}


uint16_t getI2CHardAddress(I2C_DataType *I2C_Data)
{
	return I2C_Data->addr_hard;
}

uint32_t getI2CSoftAddress(I2C_DataType *I2C_Data)
{
	return I2C_Data->addr_soft;
}

bool checkI2CHardAddress(I2C_DataType *I2C_Data)
{
	if(getI2CHardAddress(I2C_Data) == I2C_HARD_ADDR)
		return true;
	else 
		return false;
}

bool checkI2CSoftAddress(I2C_DataType *I2C_Data)
{
	if(getI2CSoftAddress(I2C_Data) == I2C_SOFT_ADDR)
		return true;
	else 
		return false;
}


uint16_t calculateCRC(uint8_t *pframe)
{
	return crc16(pframe, I2C_FRAME_SIZE - I2C_CRC_SIZE);
}

bool checkCRC(I2C_DataType *I2C_Data)
{
	uint8_t *pframe = I2C_Data->frame;
	if(I2C_Data->crc == calculateCRC(pframe))
		return true;
	else 
		return false;
}

void executeCommand(I2C_DataType *I2C_Data)
{
	switch(I2C_Data->cmd)
	{
		case 0x40:	
				break;
		case 0x41:	
				break;
		default:	
				break;
	}
}


