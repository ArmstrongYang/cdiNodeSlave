/*****************************************************************

File			:	YD-Module\dataStruct\dataStruct.c
Fuction		:	Power management chip
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/
#include "sI2C.h"

I2C_StateType i2c_State;
I2C_DataType i2c_Data;
Device_DataType deviceMe;


uint8_t device_write(Device_DataType *device)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t timeout = 10;
	
	printf("-----------------------device_write\r\n");
	i2cdata_update(device, &i2c_Data);
	while(HAL_Status!=HAL_OK)
	{
		if(__HAL_I2C_GET_FLAG(&BUS_I2C, I2C_FLAG_BUSY)==SET || i2c_State.error==I2C_ERROR_MAX)
		{
			i2c_State.error++;
			HAL_I2C_DeInit(&hi2c1);  
			MX_I2C1_Init();  
			HAL_Delay(5);
		}
		
		if(i2c_State.error>=2*I2C_ERROR_MAX)
		{
			i2c_State.error=0;
			HAL_I2C_DeInit(&hi2c1);
			MX_I2C1_Init();
			break;
		}
		
		HAL_Status = HAL_I2C_Master_Transmit(&BUS_I2C,(i2c_Data.addr_hard<<1),i2c_Data.frame,i2c_Data.size, timeout);
		if(HAL_Status==HAL_OK)
		{
			i2c_State.error=0;
			return true;
		}
	}
	return false;
}

uint8_t device_read(Device_DataType *device)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t timeout = 10;
	
	//检查i2c硬件状态
	printf("-----------------------device_write\r\n");
	i2cdata_update(device, &i2c_Data);
	printf("i2c_State_updated\r\n");
	while(HAL_Status!=HAL_OK)
	{
		if(__HAL_I2C_GET_FLAG(&BUS_I2C, I2C_FLAG_BUSY)==SET || i2c_State.error==I2C_ERROR_MAX)
		{
			i2c_State.error++;
			HAL_I2C_DeInit(&hi2c1);
			MX_I2C1_Init();
			HAL_Delay(5);
		}
		
		if(i2c_State.error>=2*I2C_ERROR_MAX)
		{
			i2c_State.error=0;
			HAL_I2C_DeInit(&hi2c1);
			MX_I2C1_Init();
			printf("I2C re init fail------------\r\n");
			break;
		}
		
		HAL_Status = HAL_I2C_Master_Transmit(&BUS_I2C,(i2c_Data.addr_hard<<1),i2c_Data.frame,i2c_Data.size, timeout);
		if(HAL_Status==HAL_OK)
		{
			printf("-------------------HAL_OK 1\r\n");
			HAL_Delay(10);
			HAL_Status = HAL_I2C_Master_Receive(&hi2c1, (i2c_Data.addr_hard<<1), i2c_Data.frame, i2c_Data.size, timeout);
			i2cdata_parse(device, &i2c_Data);
			if(HAL_Status==HAL_OK)
			{
				printf("-------------------HAL_OK 2\r\n");
				i2c_State.error=0;
				return true;
			}
		}
	}
	return false;
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

	I2C_Data->flag = 0;
	I2C_Data->addr_hard = 0;
	I2C_Data->addr_soft = 0;
	I2C_Data->size = 0;
	I2C_Data->cmd = 0;
	I2C_Data->crc=0;
	memset(I2C_Data->data,0,I2C_DATA_SIZE);
	memset(I2C_Data->frame,0,I2C_FRAME_SIZE);
	
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
	uint8_t *pframe = I2C_Data->frame;

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

bool checkI2CHardAddress(Device_DataType *device, I2C_DataType *I2C_Data)
{
	if(getI2CHardAddress(I2C_Data) == device->type)
		return true;
	else 
		return false;
}

bool checkI2CSoftAddress(Device_DataType *device, I2C_DataType *I2C_Data)
{
	if(getI2CSoftAddress(I2C_Data) == device->address)
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


