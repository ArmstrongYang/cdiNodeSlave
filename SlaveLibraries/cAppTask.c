
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "cAppTask.h"
#include "cLEDs.h"
#include "cWIFI.h"


uint16_t gLength = 0;			//16kbytes,6bytes/frame =2730.666
void Task1(void);

osMutexId WifiUart_MutexReadHandle;
osMutexId WifiUart_MutexWriteHandle;
osMutexId Device_MutexReadHandle;
osMutexId Device_MutexWriteHandle;
osMutexId cI2C_Handle;

void WifiUart_MutexFun(void){
	
    osMutexDef(WifiUart_MutexRead);
    WifiUart_MutexReadHandle = osMutexCreate(osMutex(WifiUart_MutexRead));
    osMutexDef(WifiUart_MutexWrite);
    WifiUart_MutexWriteHandle = osMutexCreate(osMutex(WifiUart_MutexWrite));
	
    osMutexDef(Device_MutexReadHandle);
    WifiUart_MutexReadHandle = osMutexCreate(osMutex(Device_MutexReadHandle));
    osMutexDef(Device_MutexWriteHandle);
    WifiUart_MutexWriteHandle = osMutexCreate(osMutex(Device_MutexWriteHandle));

    osMutexDef(cI2C_Handle);
    WifiUart_MutexWriteHandle = osMutexCreate(osMutex(cI2C_Handle));

}

void cTask_Init(void) {

	cLEDs_Init();
	cWIFI_Init();
}

void cTask_List(void) {

	cLEDs_Task();
	cI2C_Task1Fun();
//	Task1();
}

osThreadId Task1_Handle;

void key_led(void const * argument)
{
	Device_DataType sKey1;
	sKey1.type = 0x11;
	sKey1.address = SLAVE_SOFT_ADDR;
	sKey1.command = 0x40;
	sKey1.value = 0;
	
	device_read(&sKey1);
	
	Device_DataType sRGB1;
	sRGB1.type = 0x10;
	sRGB1.address = SLAVE_SOFT_ADDR+1;
	sRGB1.command = 0x40;
	sRGB1.value = sKey1.value;
	device_write(&sRGB1);
	osDelay(100);
}
void Task1(void)
{
  osThreadDef(task_Handle, key_led, osPriorityNormal, 0, 128);
  Task1_Handle = osThreadCreate(osThread(task_Handle), NULL);
}

/*任务1： cLEDs  开始*/
osThreadId cLEDs_TaskHandle;
char cLEDs_status = 1;	//0：红色代表电源正常，1：绿色闪烁代表出错，2：蓝色代表需要重启或电源电压太低
void cLEDs_TaskFun(void const * argument) {

  while(1){
		if(cLEDs_status==0){
			cLEDs_Toggle(R);
			cLEDs_Write(G,OFF);
			cLEDs_Write(B,OFF);
//			printf("Red LED Toggle\r\n");
			osDelay(1000);
		}
		else if(cLEDs_status==1){
			cLEDs_Write(R,OFF);
			cLEDs_Toggle(G);
			cLEDs_Write(B,OFF);
//			printf("Green LED Toggle\r\n");
			osDelay(1000);
		}
		else if(cLEDs_status==2){
			cLEDs_Write(R,OFF);
			cLEDs_Write(G,OFF);
			cLEDs_Toggle(B);
//			printf("Blue LED Toggle\r\n");
			osDelay(1000);
		}
		else{
			cLEDs_Write(R,OFF);
			cLEDs_Write(G,OFF);
			cLEDs_Write(B,OFF);
			osDelay(1000);
		}
  }
}

void cLEDs_Task(void) {
 
  osThreadDef(cLEDs_Handle, cLEDs_TaskFun, osPriorityNormal, 0, 128);
  cLEDs_TaskHandle = osThreadCreate(osThread(cLEDs_Handle), NULL);

}
/*任务1： cLEDs  结束*/
/*任务2： I2C  开始*/
osThreadId cI2C_TaskHandle;


void cI2C_Task1(void const * argument)
{
	while(1){
	Device_DataType sKey1;
	sKey1.type = 0x10;
	sKey1.address = SLAVE_SOFT_ADDR;
	sKey1.command = 0x51;
	sKey1.value = 2565536.0;
	
//	Device_DataType sRGB1;
//	sRGB1.type = 0x10<<1;
//	sRGB1.address = SLAVE_SOFT_ADDR;
//	sRGB1.command = 0x51;
//	sRGB1.value = 0;
	
	printf("Hello\r\n");
	device_read(&sKey1);
	printf("sKey1.value:%f\r\n",sKey1.value);
//	sRGB1.value = sKey1.value;
//	device_write(&sRGB1);
	osDelay(1000);
	}
	
}

void cI2C_Task1Fun(void) {
	
  osThreadDef(cI2C_Handle, cI2C_Task1, osPriorityAboveNormal, 0, 128);
  cI2C_TaskHandle = osThreadCreate(osThread(cI2C_Handle), NULL);

}

/*任务2： I2C  结束*/


