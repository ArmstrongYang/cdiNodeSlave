/*****************************************************************

File			:	YD-Module\BLE_DA14580\BLE.h
Fuction		:	
Author		:	@hiyangdong
Version		:	V1.0
Time			:	30 Nov. 2015

*****************************************************************/


#ifndef _SI2C_H_
#define _SI2C_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

#define BUS_I2C  hi2c1

#define I2C_BROADCAST_ADDR	((uint16_t)0x0000)
	 
#define I2C_FRAME_SIZE			((uint8_t)16)
#define I2C_HARD_INDEX			((uint8_t)0)
#define I2C_SOFT_INDEX			((uint8_t)2)
#define I2C_SIZE_INDEX      ((uint8_t)6)
#define I2C_CMD_INDEX      	((uint8_t)7)
#define I2C_DATA_INDEX     	((uint8_t)8)
#define I2C_DATA_SIZE      	((uint8_t)6)
#define I2C_CRC_INDEX      	((uint8_t)14)
#define I2C_CRC_SIZE      	((uint8_t)2)

#define I2C_ERROR_MAX					((uint8_t)5)
#define I2C_TX_TIMEOUT          		((uint16_t)1000)
#define I2C_RX_TIMEOUT          		((uint16_t)1000)

typedef struct
{
	bool init;	
	char state;
	char error;
	
}I2C_StateType;

typedef struct
{
	uint8_t frame[I2C_FRAME_SIZE];
	
	char 		 flag;
	uint16_t addr_hard;
	uint32_t addr_soft;
	uint8_t  size;
	uint8_t  cmd;
	uint8_t  data[I2C_DATA_SIZE];
	float    value;
	uint16_t crc;
	
}I2C_DataType;

typedef struct
{
	bool state;		// «∑Ò‘⁄œﬂ
	uint16_t type;
	uint32_t address;
	uint8_t  command;
	float    value;
	
}Device_DataType;


extern I2C_StateType i2c_State;
extern I2C_DataType i2c_Data;
extern Device_DataType deviceMe;

extern uint8_t device_write(Device_DataType *device);
extern uint8_t device_read(Device_DataType *device);

extern void i2cdata_init(I2C_DataType *I2C_Data);
extern void i2cdata_update(Device_DataType *device, I2C_DataType *I2C_Data);
extern void i2cdata_parse(Device_DataType *device, I2C_DataType *I2C_Data);

uint16_t getI2CHardAddress(I2C_DataType *I2C_Data);
uint32_t getI2CSoftAddress(I2C_DataType *I2C_Data);

extern bool checkI2CHardAddress(Device_DataType *device, I2C_DataType *I2C_Data);
extern bool checkI2CSoftAddress(Device_DataType *device, I2C_DataType *I2C_Data);
extern bool checkCRC(I2C_DataType *I2C_Data);

extern void i2cdata_write(I2C_DataType *I2C_Data, uint16_t device_type, uint32_t device_address, uint8_t device_command, float device_value);

uint16_t crc16(uint8_t *buf, uint16_t length);


#ifdef __cplusplus
}
#endif


#endif
