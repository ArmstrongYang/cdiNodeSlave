/*****************************************************************

File			:	YD-Module\dataStruct\dataStruct.c
Fuction		:	Power management chip
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/
#include "dataStruct.h"

I2C_StateType i2c_State;
I2C_DataType i2c_Data;

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
	memset(I2C_Data->frame,0,I2C_FRAME_SIZE);
	memset(I2C_Data->data,0,I2C_DATA_SIZE);
	I2C_Data->flag = 0;
	
	I2C_Data->addr_hard = (uint16_t) (device->type<<1);
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

void i2cdata_parse(I2C_DataType *I2C_Data)
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


