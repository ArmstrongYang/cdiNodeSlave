/*****************************************************************

File			:	YD-Module\BLE_DA14580\BLE.h
Fuction		:	
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/

#ifndef _CI2C_H_
#define _CI2C_H_

#include "main.h"

#define I2C_ERROR_MAX					((uint8_t)5)
#define BUS_I2C  hi2c1

#define I2C_FRAME_SIZE			((uint8_t)16)
#define I2C_HARD_ADDR				((uint16_t)0x0020)
#define I2C_SOFT_ADDR				((uint32_t)0x01010101)

#define SLAVE_HARD_ADDR				((uint16_t)0x0008)
#define SLAVE_SOFT_ADDR				((uint32_t)0x01010101)

#define I2C_HARD_INDEX			((uint8_t)0)
#define I2C_SOFT_INDEX			((uint8_t)2)
#define I2C_SIZE_INDEX      ((uint8_t)6)
#define I2C_CMD_INDEX      	((uint8_t)7)
#define I2C_DATA_INDEX     	((uint8_t)8)
#define I2C_DATA_SIZE      	((uint8_t)6)
#define I2C_CRC_INDEX      	((uint8_t)14)
#define I2C_CRC_SIZE      	((uint8_t)2)

#define I2C_TX_TIMEOUT          		((uint16_t)1000)
#define I2C_RX_TIMEOUT          		((uint16_t)1000)


typedef struct i2c_state
{
	
	char state;

	bool init;
	char error;
	
}I2C_StateType;

typedef struct DataFormat
{
	uint8_t frame[I2C_FRAME_SIZE];
//	uint8_t data_read[I2C_FRAME_SIZE];
//	uint8_t data_send[I2C_FRAME_SIZE];
	
	char flag;
	
	uint16_t addr_hard;
	uint32_t addr_soft;
	uint8_t  size;
	uint8_t  cmd;
	uint8_t  data[I2C_DATA_SIZE];
	float  value;
	uint16_t crc;
}I2C_DataType;


typedef struct DeviceDataFormat
{
	uint16_t type;
	uint32_t address;
	uint8_t command;
	float value;
}Device_DataType;


#include "main.h"
#include "cI2C.h"


extern void i2cdata_init(I2C_DataType *I2C_Data);
extern void i2cdata_update(Device_DataType *device, I2C_DataType *I2C_Data);
extern void i2cdata_parse(Device_DataType *device, I2C_DataType *I2C_Data);
extern void i2cdata_write(I2C_DataType *I2C_Data, uint16_t device_type, uint32_t device_address, uint8_t device_command, float device_value);
extern bool checkI2CHardAddress(I2C_DataType *I2C_Data);
extern bool checkI2CSoftAddress(I2C_DataType *I2C_Data);
extern bool checkCRC(I2C_DataType *I2C_Data);

uint8_t bufferCmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint8_t BufferLength);
uint16_t crc16(uint8_t *buf, uint16_t length);

uint16_t getI2CHardAddress(I2C_DataType *I2C_Data);
uint32_t getI2CSoftAddress(I2C_DataType *I2C_Data);

void executeCommand(I2C_DataType *I2C_Data);

extern I2C_StateType i2c_State;

extern osMutexId cI2C_Handle;

extern uint8_t device_write(Device_DataType *device);
extern uint8_t device_read(Device_DataType *device);


#endif
