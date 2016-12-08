/**
****************************************************************

File		:	main.h
Author	:	@hiyangdong
Version	:	V1.0
date		:	2015-05-19 19:04:20
brief		:	main header

*****************************************************************
*/

#ifndef _MAIN_H_
#define _MAIN_H_


#include "main.h"
#include "usart.h"
#include "cLEDs.h"
#include "cWIFI.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include <string.h>

extern osMutexId WifiUart_MutexReadHandle;
extern osMutexId WifiUart_MutexWriteHandle;

extern void cTask_Init(void);
extern void cTask_List(void);
extern void cLEDs_Task(void);
extern void cWIFI_Task(void);

extern void WifiUart_MutexFun(void);


#endif

