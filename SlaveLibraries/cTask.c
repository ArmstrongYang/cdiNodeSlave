

#include "cTask.h"

osMutexId WifiUart_MutexReadHandle;
osMutexId WifiUart_MutexWriteHandle;
uint32_t flashdestination = APPLICATION_ADDRESS;

uint32_t ramsource;
uint16_t start = 0;
//数据必须4字节的整数倍
uint16_t step = 256*2;
uint16_t end = 256*2 - 1;
	uint32_t gTime = 0;
//uint16_t end=9406;
//uint16_t start=9406-255;
uint16_t binLen = 19408;
void writeFlashTest(void);
char MasterAddress[]= "/HW001";

void WifiUart_MutexFun(void){
	
    osMutexDef(WifiUart_MutexRead);
    WifiUart_MutexReadHandle = osMutexCreate(osMutex(WifiUart_MutexRead));
    osMutexDef(WifiUart_MutexWrite);
    WifiUart_MutexWriteHandle = osMutexCreate(osMutex(WifiUart_MutexWrite));

}
uint16_t gLength = 0;			//16kbytes,6bytes/frame =2730.666

void cTask_Init(void) {

    cLEDs_Init();
    cWIFI_Init();
//		gTime = HAL_GetTick();

}
void cTask_List(void) {

    cLEDs_Task();
    cWIFI_Task();
}


osThreadId cLEDs_TaskHandle;

char cLEDs_status = 1;	
void cLEDs_TaskFun(void const * argument) {

    while(1) {
			
			if(cLEDs_status==0) {
					cLEDs_Write(R,ON);
					cLEDs_Write(G,OFF);
					cLEDs_Write(B,OFF);
			}
			else if(cLEDs_status==1) {
					cLEDs_Write(R,OFF);
					cLEDs_Toggle(G);
					cLEDs_Write(B,OFF);
			}
			else if(cLEDs_status==2) {
					cLEDs_Write(R,OFF);
					cLEDs_Write(G,OFF);
					cLEDs_Toggle(B);
			}
			else {
					cLEDs_Write(R,OFF);
					cLEDs_Write(G,OFF);
					cLEDs_Write(B,OFF);

			}
			osDelay(2000);
    }
}

void cLEDs_Task(void) {

    osThreadDef(cLEDs_Handle, cLEDs_TaskFun, osPriorityNormal, 0, 128);
    cLEDs_TaskHandle = osThreadCreate(osThread(cLEDs_Handle), NULL);

}


osThreadId cWIFI_TaskHandle;
void cWIFI_TaskFun(void const * argument)
{
	while(1)
	{
		//1.检查WIFI账号密码是否为空
		if(*WIFI_SSID_NAME=='\0')
		{
			printf("WIFI_SSID_NAME is null\r\n");
			wifiState.gotssid = 0;
		}
		
		if(wifiState.gotssid==0)
		{
			if(cWIFI_CIPSERVER("=1","OK",NULL))
			{
				printf("Waiting ssid and pwd...........\r\n");
				cWIFI_WaitAP(60000);
				printf("Stop wating ssid and pwd...........\r\n");
				wifiState.gotssid=1;
				osDelay(200);
			}
			else
			{
				osDelay(1000);
			}
		}

		if(wifiState.gotssid==1 && wifiState.cwjap==0){
			if(cWIFI_JoinAP()==true)
			{
				wifiState.cwjap=1;
				osDelay(300);
			}
			else
			{
				wifiState.gotssid=0;
			}
		}

		if(wifiState.cwjap==1 && wifiState.tcp==0){
			if(cWIFI_JoinTCP()==true){
				printf("TCP success \r\n");
				wifiState.tcp=1;
			}
			else{
				printf("TCP error\r\n");
				wifiState.tcp=0;
				if(cWIFI_CheckAP()==false)
				{
					wifiState.cwjap=0;
					osDelay(300);
				}
				wifiState.error+=1;
				if(wifiState.error>=ERROR_MAX)
				{
					sprintf(httpPacket.content, "%s\"%s\"", "AT+PING=",TCP_SERVER_ADDRESS);
					if(cWIFI_Cmd(httpPacket.content,"+","OK",RX_TIMEOUT)==false)
					{
						wifiState.cwjap=0;
						wifiState.tcp=0;
					}
					wifiState.error=0;
				}
			}
		}
		
//		if(wifiState.report==0 && wifiState.tcp==1)
//		{
//			binLen = cWIFI_ServerReport(MasterAddress,10000);
//			printf("binLen:%d\r\n",binLen);
//			gTime = HAL_GetTick();
//			if(binLen>0)
//			{	wifiState.report=1;	}
//		}
		if(wifiState.report==1 && binLen>0)
		{
			cWIFI_ServerFinish(MasterAddress,8000);
			wifiState.report=0;
			binLen=0;
		}
		if(wifiState.tcp==1 && binLen>0)
		{
			//bug1:如果网站没有该地址的程序的话，在已经通过了cWIFI_ServerReport（）函数，会导致此处的计数不会增加
			//bug2:多连接状态下，AT+CIPSEND=75需要id
			if(cWIFI_ServerDownload(MasterAddress,start,end,1000)==true)
			{
				printf("Download OK\r\n");
				printf("Flash Start:%d-%d\r\n",start,end);
				//RAM数据从uint8_t 转换成对齐的 uint32_t							
				ramsource = (uint32_t) & wifiData.pData[0];

				//写数据到Flash，并判断正确与否
				//bug1 2016-12-07 00:13:51: 有时候程序未擦除的话，可能会flash烧录失败
				if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, step/4) == FLASHIF_OK)                   
				{
					flashdestination += step;
					printf("Flash OK:%d-%d\r\n",start,end);
					//发送请求下一帧
					if(binLen-1-end>=step)
					{
						start+=step;
						end +=step;
					}
					else if((binLen-1-end<step)&&(binLen-1-end>0))
					{
						start+=step;
						end = binLen-1;
					}
					else if((binLen-1)==end)
					{
						//bug 2016-12-07 00:28:34：发送接收成功指令，返回的是false，然后导致循环到download会出现索引错误
						if(cWIFI_ServerFinish(MasterAddress,8000)==true)
						{
							printf( "JumpToAPP\r\n");
							writeFlashTest();
							printf( "Total Time:%d s\r\n", (HAL_GetTick() - gTime)/1000 );
							NVIC_SystemReset();
							Boot2APP();
						}
						else
						{
							printf( "cWIFI_ServerFinish error\r\n");
						}
					}
				}
				else{
					printf("Flash error\r\n");
				}
			}
			else
			{
				printf("Download error\r\n");
				if(cWIFI_CheckTCP()==false)
				{
					printf("TCP error\r\n");
					wifiState.tcp=0;
				}
			}
		}
		osDelay(500);
	}
	
	
	
    while(1) {
			if(wifiState.init==true){
				printf("init ok\r\n");
			}		
			if(wifiState.cwjap==0){
				if(cWIFI_JoinAP()==true)
				{
					wifiState.cwjap=1;
					osDelay(300);
				}
			}
			
			if(wifiState.tcp==0 && wifiState.cwjap==1){
				if(cWIFI_JoinTCP()==true){
					printf("AP success \r\n");
					
					wifiState.tcp=1;
				}
				else{
					printf("AP error\r\n");
					wifiState.tcp=0;
					if(cWIFI_CheckAP()==false)
					{
						wifiState.cwjap=0;
						osDelay(300);
					}
				}
			}
			if(wifiState.report==0 && wifiState.tcp==1)
			{
				binLen = cWIFI_ServerReport(MasterAddress,10000);
				gTime = HAL_GetTick();
				if(binLen>0)
				{	wifiState.report=1;	}
			}
			printf("cWIFI_Fun\r\n");

//			cWIFI_JoinTCP();
//			if((wifiState.tcp==1)&&(wifiState.stateStart==1))
//			cWIFI_ServerReport(addr,10000);
			if(wifiState.tcp==1 && binLen>0)
			{
				//bug1:如果网站没有该地址的程序的话，在已经通过了cWIFI_ServerReport（）函数，会导致此处的计数不会增加
				if(cWIFI_ServerDownload(MasterAddress,start,end,2000)==true)
				{
					printf("Download OK\r\n");
//					HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)wifiData.pData, 256, 1000);
					printf("Flash Start:%d-%d\r\n",start,end);
//					if((wifiData.pData[0]!=0) || (wifiData.pData[1]!=0 ))
//					{
					//RAM数据从uint8_t 转换成对齐的 uint32_t							
					ramsource = (uint32_t) & wifiData.pData[0];

					//写数据到Flash，并判断正确与否
					if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, step/4) == FLASHIF_OK)                   
					{
						flashdestination += step;
						printf("Flash OK:%d-%d\r\n",start,end);
						//发送请求下一帧
						if(binLen-1-end>=step)
						{
							start+=step;
							end +=step;
						}
						else if((binLen-1-end<step)&&(binLen-1-end>0))
						{
							start+=step;
							end = binLen-1;
						}
						else if((binLen-1)==end)
						{
							HAL_UART_Transmit(&DEBUG_UART, (uint8_t*)"JumpToAPP\r\n", 11, 100);
							/* Suspend Tick increment */
//							HAL_SuspendTick();
//							HAL_NVIC_DisableIRQ(SysTick_IRQn);
							writeFlashTest();
							//Total Time:32 s
							//2016-12-01 16:55:21Total Time:127 s
							//2016-12-01 16:57:16 Total Time:12 s
							//2016-12-01 17:20:49 Total Time:11 s
							printf( "Total Time:%d s\r\n", (HAL_GetTick() - gTime)/1000 );
							NVIC_SystemReset();
							Boot2APP();
						}
					}
					else 
						printf("Flash error\r\n");
					
				}
				else
				{
					printf("Download error\r\n");
					if(cWIFI_CheckTCP()==false)
					{
						printf("TCP error\r\n");
						wifiState.tcp=0;
					}
				}
			}
			else
			{
				wifiState.report=0;
			}
//	printf("Report%d\r\n",cWIFI_ServerReport(addr,8000));

        
//	cWIFI_ServerFinish(addr,8000);
//
//	cWIFI_ServerStatus(addr,8000);
//        osDelay(100);
    }
}

void cWIFI_Task(void) {

    osThreadDef(cWIFI_Handle, cWIFI_TaskFun, osPriorityNormal, 0, 128);
    cWIFI_TaskHandle = osThreadCreate(osThread(cWIFI_Handle), NULL);

}


