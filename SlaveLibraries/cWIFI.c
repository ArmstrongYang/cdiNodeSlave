/**
 ****************************************************************
 *
 * File	:	cWIFI.c
 * Author	:	@hiyangdong
 * Version	:	V1.0
 * date	:	2015-05-13
 * brief	:	code file
 *
 *****************************************************************
 */
#include "cWIFI.h"
#include "cTask.h"

//char	WIFI_SSID_NAME[32]	= "YD-Xiaoxin";
//char	WIFI_PASSWORD[32]		= "12341234";

char	WIFI_SSID_NAME[32]	= "\0";
char	WIFI_PASSWORD[32]		= "\0";

/* char TCP_SERVER_ADDRESS[] = "alexyan.xyz"; */

char TCP_SERVER_ADDRESS[32] = "114.215.125.161";
char TCP_SERVER_ID = 4;

char	TCP_SERVER_PORT[8]	= "4000";
char	TCP_SERVER_HARDAPI[32]	= "/api/hardware";

char	TCP_CILENT_APSSID[16]	= "SmartNoder";
char	TCP_CILENT_APPWD[16]	= "00000000";
char	TCP_CILENT_PORT[8]	= "8000";

HttpPacketType	httpPacket;
WIFI_StateType	wifi_State;
WIFI_DataType	wifiData;

HAL_StatusTypeDef		HAL_Status;
extern CRC_HandleTypeDef	hcrc;

extern osMutexId WifiUart_MutexReadHandle;
extern osMutexId WifiUart_MutexWriteHandle;
bool cWIFI_Setting( char *cmd, char *reply, uint32_t msTimeout );

void WIFI_UART_Init( uint32_t Baud )
{
	huart1.Instance			= USART1;
	huart1.Init.BaudRate		= Baud;
	huart1.Init.WordLength		= UART_WORDLENGTH_8B;
	huart1.Init.StopBits		= UART_STOPBITS_1;
	huart1.Init.Parity		= UART_PARITY_NONE;
	huart1.Init.Mode		= UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl		= UART_HWCONTROL_NONE;
	huart1.Init.OverSampling	= UART_OVERSAMPLING_16;
	if ( HAL_UART_Init( &huart1 ) != HAL_OK )
	{
		Error_Handler();
	}
}


/**
 ****************************************************************
 *
 * Function:			cWIFI_Init
 * Author	:			@hiyangdong
 * Version	:			V1.0
 * date		:			2015-05-13
 * brief		:			initialize the WIFI module
 *
 *****************************************************************
 */
void cWIFI_Init( void )
{
	wifi_State.baud	= 0;
	wifi_State.init	= 0;

	wifi_State.cwjap = 0;
	wifi_State.tcp	= 0;
	wifi_State.cwmode = 0; 
	wifi_State.gotssid=0;
//	printf("cWIFI_BaudCheck");

	/*	波特率检查	*/	
	cWIFI_BaudCheck();
	
	/*	AP 兼 Station 模式：CWMODE 设置后不需重启	*/	
	if(cWIFI_Setting("AT+CWMODE?","+CWMODE:3",WIFI_RX_TIMEOUT)==false)
	{
		if(cWIFI_Setting("AT+CWMODE=3","OK",WIFI_RX_TIMEOUT)==true)
		{
			cWIFI_Setting("AT+RST","ready",WIFI_RX_TIMEOUT);
			wifi_State.cwmode = 3; 
		}
	}
	
	/*	非透传模式	*/
	if(cWIFI_Setting("AT+CIPMODE?","+CIPMODE:0",WIFI_RX_TIMEOUT)==false)
	{
		cWIFI_Setting("AT+CIPMODE=0","OK",WIFI_RX_TIMEOUT);
	}
	
	/*	开启多路连接：AT+CIPMUX=1，此时才能开启 TCP服务器	*/
	if(cWIFI_Setting("AT+CIPMUX?","+CIPMUX:1",WIFI_RX_TIMEOUT)==false)
	{
		cWIFI_Setting("AT+CIPMUX=1","OK",WIFI_RX_TIMEOUT);
	}
	
//	HAL_Delay(8000);
//	cWIFI_CWSAP(TCP_CILENT_APSSID, TCP_CILENT_APPWD, 1, 0);
//	printf("after cWIFI_BaudCheck");
//	HAL_Delay(3000);

//	cWIFI_CIPMUX("=0");

/*
 * cWIFI_ATE("1");
 * cWIFI_JoinAP();
 *
 * if(FLASH_If_Erase(APPLICATION_ADDRESS)==FLASHIF_OK)
 *      printf("FLASH_Erase OK\r\n");
 */
//	HAL_Delay( 12000 );
	printf( "End Init\r\n" );
}


bool cWIFI_BaudCheck( void )
{
	uint32_t	WIFI_BAUD_LIST[15] = { 115200, 9600, 19200, 57600, 38400, 4800, 14400, 28800, 2400, 110, 300, 1200, 230400, 460800, 921600 };
	uint32_t	msTimeStart;
	uint32_t	msTimeout	= 500;
	uint8_t		rData		= 0;
	uint8_t		index		= 0;
	uint8_t		pos		= 0;
	uint8_t		len		= 5;
	char		reply[]		= "ready";

	/* GPIO015：下拉工作模式（无其他功能）*/
	HAL_GPIO_WritePin( WIFI_WorkMode_GPIO_Port, WIFI_WorkMode_Pin, GPIO_PIN_RESET );

	for ( index = 0; index < 15; index++ )
	{
		msTimeStart = HAL_GetTick();
		WIFI_UART_Init( WIFI_BAUD_LIST[index] );

		/* CH_PD：1）高电平工作；2）低电平模块供电关掉	*/
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_RESET );
		HAL_Delay( 1 );
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_SET );

		/* 接收数据直到“ready”，平均时间：360ms	*/
		/* 获取当前时刻 */

		while ( 1 )
		{
			if ( HAL_UART_Receive( &WIFI_UART, &rData, 1, msTimeout ) != HAL_TIMEOUT )
			{
				if ( rData == reply[pos] )
				{
					pos++;
					if ( pos == len )
					{
						wifi_State.init	= 1;
						wifi_State.baud	= WIFI_BAUD_LIST[index];
						return(true);
					}
				}else {
					pos = 0;
				}
			}
			if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
			{
				printf( "cWIFI_OneCmd Timeout\r\n" );   /* 将接收到数据打印到调试串口1 */
				return(false);                          /* 超时 */
			}
		}
	}
	return(false);
}


bool cWIFI_Setting( char *cmd, char *reply, uint32_t msTimeout )
{
	uint32_t	msTimeStart;
	uint8_t		rData	= 0;
	uint8_t		pos	= 0;
	uint8_t		len	= 0;

	msTimeStart = HAL_GetTick();
	len = strlen( reply );

	if ( cmd )
	{
		HAL_Status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
		HAL_Status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) "\r\n", 2, WIFI_TX_TIMEOUT );
	}
	if ( !reply )
	{	return(true);	}

	while ( 1 )
	{
		if ( HAL_UART_Receive( &WIFI_UART, &rData, 1, msTimeout ) == HAL_OK )
		{
			if ( rData == reply[pos] ){
				pos++;
				if ( pos == len ){
					return(true); 		/* 收到指定的应答数据，返回成功 */
				}
			}else {
				pos = 0;
			}
		}else {
			HAL_Delay( 10 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			return(false); 				/* 超时 */
		}
	}
}


bool cWIFI_WaitAP( uint32_t msTimeout )
{
	HAL_StatusTypeDef	status = HAL_OK;
	uint32_t		msTimeStart;
	char			*pHead	= NULL;
//	uint8_t			tmp	= 0;
	uint8_t strLen=0;
	char replyFirst[] = "IPD";
	char *ssid = "SSID:";
	char *pwd = "PWD:";
	char *pKey = 0;
	char cnt = 0;
	
	msTimeStart = HAL_GetTick();
	memset( wifiData.buffer, 0, BUFFER_MAX );
	WIFI_UART.State = HAL_UART_STATE_READY;
	status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( status != HAL_OK )
	{
		printf( "status4:%d\r\n", status );
		return(false);
	}

	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
//		printf( "SERVER AP:%s\r\n", pHead );
		if ( pHead )
		{
//			printf( "Command OK!!!!!!!!!!!!!!!!%s\r\n", pHead );
			pKey = ssid;
			cnt=0;
			if ( strstr( wifiData.buffer, pKey ) )
			{
				pHead = strstr( wifiData.buffer, pKey );
				strLen = 0;
				while((*pHead!=';'))
				{
					pHead++;
					strLen++;
				}
				pHead = strstr( wifiData.buffer, pKey );
				strncpy(WIFI_SSID_NAME, pHead+strlen(pKey), strLen-strlen(pKey));
				cnt++;
				printf("NEW WIFI_SSID_NAME:%s\r\n", WIFI_SSID_NAME);
			}
			pKey = pwd;
			if ( strstr( wifiData.buffer, pKey ) )
			{
				pHead = strstr( wifiData.buffer, pKey );
				strLen = 0;
				while((*pHead!=';'))
				{
					pHead++;
					strLen++;
				}
				pHead = strstr( wifiData.buffer, pKey );
				strncpy(WIFI_PASSWORD, pHead+strlen(pKey), strLen-strlen(pKey));
				cnt++;
				printf("NEW WIFI_PWD:%s\r\n", WIFI_PASSWORD);
			}
			if(cnt>=2) 
			{
				HAL_UART_DMAStop( &WIFI_UART ); 
				return true; 
			}
		}
		else 
		{
			printf("Delay 10000ms\r\n");
			osDelay( 10000 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


bool cWIFI_WaitAPold( uint32_t msTimeout )
{
	HAL_StatusTypeDef	status = HAL_OK;
	uint32_t		msTimeStart;
	char			*pHead	= NULL;
//	uint8_t			tmp	= 0;
	uint8_t strLen=0;
	char replyFirst[] = "IPD";
	
	msTimeStart = HAL_GetTick();
	memset( wifiData.buffer, 0, BUFFER_MAX );
	WIFI_UART.State = HAL_UART_STATE_READY;
	status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( status != HAL_OK )
	{
		printf( "status4:%d\r\n", status );
		return(false);
	}

	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
//		printf( "SERVER AP:%s\r\n", pHead );
		if ( pHead )
		{
//			printf( "Command OK!!!!!!!!!!!!!!!!%s\r\n", pHead );
			if ( strstr( wifiData.buffer, "SSID:" ) )
			{
				pHead = strstr( wifiData.buffer, "SSID:" );
				while((*pHead!='\r')&&(*(pHead+1)!='\n'))
				{
					printf("----------%c\r\n", pHead[0]);
					printf("----------%d\r\n", strLen);
					pHead++;
					strLen++;
				}
				printf("----------%d\r\n", pHead[0]);
				printf("----------%d\r\n", pHead[1]);
				
				pHead = strstr( wifiData.buffer, "SSID:" );
				strncpy(WIFI_SSID_NAME,pHead+5,strLen+2-5);
				printf("NEW WIFI_SSID_NAME:%s\r\n", WIFI_SSID_NAME);
				osDelay(1000);
//				return(true);   /* 收到指定的应答数据，返回成功 */
			}
			if ( strstr( wifiData.buffer, "PWD:" ) )
			{
				pHead = strstr( wifiData.buffer, "PWD:" );
				while(((*pHead!='\r')&&(*(pHead+1)!='\n'))||(*pHead!=','))
				{
					printf("----------%c\r\n", pHead[0]);
					printf("----------%d\r\n", strLen);
					pHead++;
					strLen++;
				}
				printf("----------%d\r\n", pHead[0]);
				printf("----------%d\r\n", pHead[1]);
				
				pHead = strstr( wifiData.buffer, "PWD:" );
				strncpy(WIFI_PASSWORD,pHead+4,strLen+2-4);
				printf("NEW WIFI_PWD:%s\r\n", WIFI_PASSWORD);
				osDelay(1000);
//				return(true);   /* 收到指定的应答数据，返回成功 */
			}
		}
		else 
		{
//			printf( "Delay%d\r\n", tmp++ );
			osDelay( 1000 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}

/*
 *********************************************************************************************************
 *	函 数 名: ESP8266_WaitResponse
 *	功能说明: 等待ESP8266返回指定的应答字符串, 可以包含任意字符。只要接收齐全即可返回。
 *	形    参: _pAckStr : 应答的字符串， 长度不得超过255
 *			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
 *	返 回 值: 1 表示成功  0 表示失败
 *********************************************************************************************************
 */
bool cWIFI_WaitResponse( char *reply, uint32_t msTimeout )
{
	uint32_t	msTimeStart;
	uint8_t		rData	= 0;
	uint8_t		pos	= 0;
	uint8_t		len	= 0;
	char		*busy	= "busy";
	char		index	= 0;
	/* 获取当前时刻 */
	msTimeStart = HAL_GetTick();

	len = strlen( reply );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	while ( 1 )
	{
		if ( HAL_UART_Receive( &WIFI_UART, &rData, 1, msTimeout ) != HAL_TIMEOUT )
		{
			if ( rData == reply[pos] )
			{
				pos++;
				if ( pos == len )
				{
					osMutexRelease( WifiUart_MutexReadHandle );
					if ( index >= 4 )
					{
						osDelay( 500 );
					}
					return(true); /* 收到指定的应答数据，返回成功 */
				}
			}else {
				pos = 0;
			}
			if ( rData == busy[index] )
			{
				index++;
			}
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			osMutexRelease( WifiUart_MutexReadHandle );
			return(false); /* 超时 */
		}
	}
}

//bool cWIFI_WaitTwoResponse( char *reply1, char *reply2, uint32_t msTimeout )
//{
//	uint32_t	msTimeStart;
//	uint8_t		rData	= 0;
//	uint8_t		pos1	= 0;
//	uint8_t		pos2	= 0;
//	uint8_t		len1	= 0;
//	uint8_t		len2	= 0;
//	char		*busy	= "busy";
//	char		index	= 0;
//	
//	/* 获取当前时刻 */
//	msTimeStart = HAL_GetTick();

//	len1 = strlen( reply1 );
//	len2 = strlen( reply2 );

//	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
//	while ( 1 )
//	{
//		if ( HAL_UART_Receive( &WIFI_UART, &rData, 1, msTimeout ) != HAL_TIMEOUT )
//		{
//			if ( rData == reply1[pos1] )
//			{
//				pos1++;
//				if ( pos1 == len1 )
//				{
//					osMutexRelease( WifiUart_MutexReadHandle );
//					if ( index >= 4 )
//					{
//						osDelay( 500 );
//					}
//					return(true); /* 收到指定的应答数据，返回成功 */
//				}
//			}else {
//				pos = 0;
//			}
//			if ( rData == busy[index] )
//			{
//				index++;
//			}
//		}
//		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
//		{
//			osMutexRelease( WifiUart_MutexReadHandle );
//			return(false); /* 超时 */
//		}
//	}
//}



bool cWIFI_CmdSimple( char *cmd, char *reply, uint32_t msTimeout )
{
	uint32_t	msTimeStart;
	uint8_t		rData	= 0;
	uint8_t		pos	= 0;
	uint8_t		len	= 0;

	/* 获取当前时刻 */
	msTimeStart = HAL_GetTick();

	len = strlen( reply );

	/* 发送数据 */
	if ( cmd )
	{
		osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
		HAL_Status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
		if ( HAL_Status != HAL_OK )
		{
			printf( "status1:%d\r\n", HAL_Status );
			return(false);
		}
		HAL_Status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) "\r\n", 2, WIFI_TX_TIMEOUT );
		if ( HAL_Status != HAL_OK )
		{
			printf( "status2:%d\r\n", HAL_Status );
			return(false);
		}
		osMutexRelease( WifiUart_MutexWriteHandle );
	}
	if ( !reply )
	{
		return(true);
	}
	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	while ( 1 )
	{
		if ( HAL_UART_Receive( &WIFI_UART, &rData, 1, msTimeout ) != HAL_TIMEOUT )
		{
			if ( rData == reply[pos] )
			{
				pos++;
				if ( pos == len )
				{
					osMutexRelease( WifiUart_MutexReadHandle );
					return(true); /* 收到指定的应答数据，返回成功 */
				}
			}else {
				pos = 0;
			}
		}else {
			osDelay( 10 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			osMutexRelease( WifiUart_MutexReadHandle );
			return(false); /* 超时 */
		}
	}
}


/*
 * 函数名：cWIFI_Cmd
 * 描述  ：对WF-ESP8266模块发送AT指令
 * 输入  ：cmd，待发送的指令
 *         reply1，reply2，期待的响应，为NULL表不需响应，两者为或逻辑关系
 *         waittime，等待响应的时间
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */

/* reply 有顺序 */


/*
 * 问题：memset(wifiData.buffer, 0, BUFFER_MAX);运行时间太长了
 * 问题：有时候接收到错误的？
 */
bool cWIFI_Cmd( char * cmd, char * replyFirst, char * replySecond, uint32_t msTimeout )
{
	HAL_StatusTypeDef	status = HAL_OK;
	uint32_t		msTimeStart;
	char			*pHead	= NULL;
	uint8_t			tmp	= 0;
	memset( wifiData.buffer, 0, BUFFER_MAX );

	/* 发送数据 */
	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
	if ( status != HAL_OK )
	{
		printf( "status1:%d\r\n", status );
		osMutexRelease( WifiUart_MutexWriteHandle );
		return(false);
	}
	status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) "\r\n", 2, WIFI_TX_TIMEOUT );
	if ( status != HAL_OK )
	{
		printf( "status2:%d\r\n", status );
		osMutexRelease( WifiUart_MutexWriteHandle );
		return(false);
	}
	osMutexRelease( WifiUart_MutexWriteHandle );

	/* 不需要接收数据 */
	if ( (replyFirst == 0) && (replySecond == 0) )
	{
		return(true);
	}

	msTimeStart = HAL_GetTick();


/*
 * while(HAL_UART_GetState(&WIFI_UART)!=HAL_UART_STATE_READY)
 * {
 *      printf("status3:%d\r\n",HAL_UART_GetState(&WIFI_UART));HAL_Delay(500);
 *      printf("error:%d\r\n",HAL_UART_GetError(&WIFI_UART));HAL_Delay(500);
 *
 * }
 */
	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( HAL_Status != HAL_OK )
	{
//		HAL_UART_DMAStop( &WIFI_UART );
		osMutexRelease( WifiUart_MutexReadHandle );
		return(false);
	}

	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
//		printf( "pHead:%s\r\n", pHead );
		if ( pHead )
		{
//			printf( "Command OK!!!!!!!!!!!!!!!!%s\r\n", pHead );
			if ( replySecond == NULL )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				osMutexRelease( WifiUart_MutexReadHandle );
				return(true);   /* 收到指定的应答数据，返回成功 */
			}else if ( (replySecond != NULL) && strstr( pHead, replySecond ) )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				osMutexRelease( WifiUart_MutexReadHandle );
				return(true);   /* 收到指定的应答数据，返回成功 */
			}
		}else {
			printf( "Delay%d\r\n", tmp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


void cWIFI_Init2( void )
{
	uint32_t	WIFI_BAUD_LIST[15]	= { 115200, 9600, 19200, 57600, 38400, 4800, 14400, 28800, 2400, 110, 300, 1200, 230400, 460800, 921600 };
	uint8_t		index			= 0;

	/* GPIO015：下拉工作模式（无其他功能）	*/
	HAL_GPIO_WritePin( WIFI_WorkMode_GPIO_Port, WIFI_WorkMode_Pin, GPIO_PIN_RESET );

	for ( index = 0; index < 15; index++ )
	{
		WIFI_UART_Init( WIFI_BAUD_LIST[index] );

		/* CH_PD：1）高电平工作；2）低电平模块供电关掉	*/
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_RESET );
		HAL_Delay( 1 );
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_SET );

		/* 接收数据直到“ready”，平均时间：360ms	*/
		if ( cWIFI_CmdSimple( NULL, "ready", 500 ) )
		{
			wifi_State.init = 1;
			break;
		}
	}

	printf( "End Init\r\n" );
	HAL_Delay( 30000 );
}


/* !必须要考虑波特率 */

bool cWIFI_Rst( void )
{
	wifi_State.state = 0;
	if ( cWIFI_CmdSimple( "AT+RST", "ready", 3600 ) == true )
	{
		wifi_State.state = 1;
		return(true);
	}else {
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_RESET );
		osDelay( 100 );
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_SET );
		osDelay( 5000 );
		if ( cWIFI_AT() == true )
		{
			wifi_State.state = 1;
			return(true);
		}else {
			wifi_State.state = 0;
			return(true);
		}
	}
}


bool cWIFI_AT( void )
{
	wifi_State.error = 0;
	while ( cWIFI_CmdSimple( "AT", "OK", 2500 ) == false )
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			return(false);
		}
	}
	return(true);
}


void cWIFI_DeInit( void )
{
	HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_RESET );
	HAL_UART_MspDeInit( &WIFI_UART );
}


/*
 * 函数名：cWIFI_PD
 * 描述  ：使能/禁用WF-ESP8266模块
 * 输入  ：enumChoose = ON，使能模块
 *         enumChoose = OFF，禁用模块
 * 返回  : 无
 * 调用  ：被外部调用
 */

void cWIFI_PD( uint8_t mode )
{
	/* CH_PD：1）高电平工作；2）低电平模块供电关掉	*/
	if ( mode == 1 )
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_SET );
	else
		HAL_GPIO_WritePin( WIFI_PD_GPIO_Port, WIFI_PD_Pin, GPIO_PIN_RESET );
}


HAL_StatusTypeDef cWIFI_strWrite( char *pString )
{
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) pString, strlen( pString ), WIFI_TX_TIMEOUT );

	return(status);
}


HAL_StatusTypeDef cWIFI_strRead( char *pString, uint32_t timeout )
{
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_UART_Receive( &WIFI_UART, (uint8_t *) pString, BUFFER_MAX, timeout );

	printf( "%s", wifiData.buffer );

	return(status);
}


bool cWIFI_ATE( char *para )
{
	char buf[16] = "ATE";

	strcat( buf, para );
	wifi_State.error = 0;
	while ( cWIFI_Cmd( buf, "OK", NULL, 500 ) == false ) /* ATE0:关闭回显功能;ATE1:打开回显功能 */
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			return(cWIFI_AT() );
		}
	}
	wifi_State.state = 2;
	return(true);
}


/*
 * 函数名：cWIFI_Net_Mode_Choose
 * 描述  ：选择WF-ESP8266模块的工作模式
 * 输入  ：enumMode，工作模式
 * 返回  : 1，选择成功
 *         0，选择失败
 * 调用  ：被外部调用
 */
bool cWIFI_CWMODE( char *para , char *reply1, char *reply2)
{
	char buf[16] = "AT+CWMODE";

	strcat( buf, para );
	return cWIFI_Cmd( buf, reply1, reply2, 500 );
}


/*
发送命令：AT+CWSAP="SmartNode","00000000",1,0(设置指令)	
指令：AT+ CWSAP= <ssid>,<pwd>,<chl>, <ecn> 
说明：指令只有在AP模式开启后有效
<ssid>:字符串参数，接入点名称
<pwd>:字符串参数，密码最长64字节，ASCII
<chl>:通道号
< ecn >:0-OPEN，1-WEP，2-WPA_PSK，3-WPA2_PSK，4-WPA_WPA2_PSK 
响应：OK
*/

bool cWIFI_CWSAP( char *ssid , char *pwr, char chl, char ecn)
{
	char *buf = "AT+CWSAP";
	char cmd[32] = "\0";
//		HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
//	AT+CWSAP="SmartNode","00000000",1,0
	sprintf(cmd,"%s=\"%s\",\"%s\",%d,%d\r\n", buf, ssid, pwr, chl, ecn);
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
	return true;
//	return cWIFI_Cmd( buf, "OK", NULL, 500 );
}

bool cWIFI_CIPSERVER( char *para , char *reply1, char *reply2)
{
	char *buf = "AT+CIPSERVER";
	char cmd[32] = "\0";

	sprintf(cmd,"%s%s,%s", buf, para, TCP_CILENT_PORT);
	return cWIFI_Cmd( cmd, reply1, reply2, 500 );
}

bool cWIFI_CIPMODE( char *para )
{
	char buf[16] = "AT+CIPMODE";

	strcat( buf, para );
	wifi_State.error = 0;
	while ( cWIFI_Cmd( buf, "OK", NULL, 500 ) == false ) /*  */
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			return(cWIFI_AT() );
		}
	}
	wifi_State.state = 2;
	return(true);
}


bool cWIFI_CIPMUX( char *para )
{
	char cmd[16] = "AT+CIPMUX";

	strcat( cmd, para );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
	return(cWIFI_WaitResponse( "OK", 500 ) );
}


/*
 * 函数名：cWIFI_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool cWIFI_JoinAPFull( char *pSSID, char *pPassword, uint32_t msTimeout )
{

	uint16_t	strLen;
	uint32_t	msTimeStart;
	
	char temp = 0;
	char		*replyFirst	= "WIFI CONNECTED";
	char		*replySecond = "WIFI GOT IP";

	msTimeStart = HAL_GetTick();
	memset( httpPacket.content, 0x00, CMDLEN);
	memset( wifiData.buffer, 0x00, BUFFER_MAX );

	sprintf( httpPacket.content,"AT+CWJAP=\"%s\",\"%s\"\r\n", pSSID, pPassword );
	strLen = strlen(httpPacket.content);
	
	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, strLen, 2000 );
	osMutexRelease( WifiUart_MutexWriteHandle );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( HAL_Status != HAL_OK )
	{
		return false;
	}
	while ( 1 )
	{
		if ( strstr( wifiData.buffer, replyFirst) )
		{
			if ( strstr( wifiData.buffer, replySecond) )
			{
				if ( strstr( wifiData.buffer, "OK") )
				{
					printf( "JoinAP OK\r\n");
					HAL_UART_DMAStop( &WIFI_UART );
					osMutexRelease( WifiUart_MutexReadHandle );
					return true;
				}
				else
				{
					msTimeStart+=200;
				}
			}
			else
			{
				msTimeStart+=100;
			}
		}
		if ( strstr( wifiData.buffer, "FAIL") || strstr( wifiData.buffer, "ERROR" ))
		{
			printf( "JoinAP FAIL\r\n");
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			return false;
		}
		if ( strstr( wifiData.buffer, "busy" ) )
		{
			printf( "JoinAP Delay2000ms:%d\r\n", temp++ );
			osDelay( 2000 );
		}
		else 
		{
			printf( "JoinAP Delay1000ms:%d\r\n", temp++ );
			osDelay( 1000 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}



bool cWIFI_CWJAP( char *pSSID, char *pPassword, uint32_t msTimeout )
{
	char cmd[64] = "AT+CWJAP";

/*    strcat(buf,"?"); */
	sprintf( cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", pSSID, pPassword );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
	return(cWIFI_WaitResponse( "WIFI GOT IP", msTimeout ) );


/*
 *    if(cWIFI_CmdSimple(buf,"+CWJAP:", (uint32_t)(msTimeout/2))==true)		//测试连接，时间可以很短
 *    {
 *        printf("WIFI already connect successfullly\r\n");
 *        wifi_State.cwjap = 1;
 *        return true;
 *    }
 *    else
 *    {
 *        sprintf (buf, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassword);
 *        if(cWIFI_Cmd(buf,"WIFI CONNECTED", "WIFI GOT IP", msTimeout)==true)
 *        {
 *            printf("WIFI connect successfullly\r\n");
 *            wifi_State.cwjap = 1;
 *            return true;
 *        }
 *        else
 *        {
 *            wifi_State.cwjap = 0;
 *            return false;
 *        }
 *    }
 */
}

/*bug1: 路由器未连接成功竟然会跳转到JoinTCP*/
bool cWIFI_JoinAP( void )
{
	uint32_t time = 20000;
	return cWIFI_JoinAPFull( WIFI_SSID_NAME, WIFI_PASSWORD, time );
//	wifi_State.error = 0;
//	while ( cWIFI_CWJAP( WIFI_SSID_NAME, WIFI_PASSWORD, time ) == false )
//	{
//		if ( wifi_State.error == 0 )
//		{
//			time = time * 2;
//		}
//		wifi_State.error++;
//		if ( wifi_State.error > ERROR_MAX )
//		{
//			return(cWIFI_AT() );
//		}
//	}
//	return(true);
}
bool cWIFI_CheckAP( void )
{
	uint32_t time = 2000;
	char buf[32] = "+CWJAP:";
	sprintf( buf, "+CWJAP:\"%s\"",WIFI_SSID_NAME);
//	memset( httpPacket.content, 0x00, CMDLEN);
//	sprintf( httpPacket.content, "AT+CWJAP?");
	return cWIFI_CmdSimple( "AT+CWJAP?", buf, time);  //测试连接，时间可以很短

 }
/*
 * cWIFI_JoinTCP
 * 描述  ：WF-ESP8266模块连接WiFi后连接tcp server
 * 输入  ：ip，tcp server地址
 *       ：port，tcp server 端口
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */

bool cWIFI_JoinTCPFull( char *pIP, char *pPort, uint32_t msTimeout )
{

	uint16_t	strLen;
	uint32_t	msTimeStart;
	
	char		temp		= 0;

	msTimeStart = HAL_GetTick();
	memset( httpPacket.content, 0x00, CMDLEN);
	memset( wifiData.buffer, 0x00, BUFFER_MAX );
	
	sprintf( httpPacket.content, "AT+CIPSTART=%d,\"TCP\",\"%s\",%s\r\n",TCP_SERVER_ID, pIP, pPort );
	strLen = strlen(httpPacket.content);
	
	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, strLen, 2000 );
	osMutexRelease( WifiUart_MutexWriteHandle );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( HAL_Status != HAL_OK )
	{
		osMutexRelease( WifiUart_MutexReadHandle );
		return(false);
	}
	while ( 1 )
	{
		if ( strstr( wifiData.buffer, "CONNECT\r\n\r\nOK" ) )
		{
			printf( "JoinTCP CONNECT OK\r\n");
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			return true;
		}
		if ( strstr( wifiData.buffer, "ALREADY CONNECT" ) )
		{
			printf( "JoinTCP ALREADY CONNECT\r\n");
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			return true;
		}
		if ( strstr( wifiData.buffer, "FAIL" )||strstr(wifiData.buffer, "ERROR")||strstr(wifiData.buffer,"CLOSED"))
		{
			printf( "JoinTCP CONNECT FAIL\r\n");
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			return false;
		}
		if ( strstr( wifiData.buffer, "busy" ) )
		{
			printf( "JoinTCP Delay1000ms:%d\r\n", temp++ );
			osDelay( 1000 );
		}
		else 
		{
			printf( "JoinTCP Delay100ms:%d\r\n", temp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}

bool cWIFI_CIPSTART( char *pIP, char *pPort, uint32_t msTimeout )
{
	char cmd[128];

	sprintf( cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", pIP, pPort );
//	cWIFI_Cmd(cmd,"CONNECT"
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );


/*
 * HAL_Delay(200);
 * return true;
 */
	return(cWIFI_WaitResponse( "CONNECT", msTimeout ) );


/*
 *    if(cWIFI_CmdSimple(cmd,"CONNECT", msTimeout)==true)		//测试TCP连接，如果连接则时间可以很短
 *    {
 * //		printf("TCP connect successfullly\r\n");
 *        wifi_State.tcp = 1;
 *        return true;
 *    }
 * //    else if(cWIFI_Cmd(buf,"ALREADY CONNECTED", "ERROR", 3000)==true)
 * //    {
 * ////			printf("TCP already connect successfullly\r\n");
 * //        wifi_State.tcp = 1;
 * //        return true;
 * //    }
 *    else
 *    {
 *        wifi_State.tcp = 0;
 *        return false;
 *    }
 */
}

/*bug1 2016-12-01 18:57:14：
				连接TCP失败，出现连续的ERROR和CLOESD
*/

bool cWIFI_JoinTCP( void )
{
	uint32_t time = 5000;
	return cWIFI_JoinTCPFull( TCP_SERVER_ADDRESS, TCP_SERVER_PORT, time );
//	uint32_t time = 2000;
//	wifi_State.error = 0;
//	return(cWIFI_CIPSTART( TCP_SERVER_ADDRESS, TCP_SERVER_PORT, time ) );


/*
 *    while(cWIFI_CIPSTART(TCP_SERVER_ADDRESS, TCP_SERVER_PORT, time)==false)
 *    {
 *        if(wifi_State.error==0)
 *        {
 *            time = time * 2;
 *        }
 *        wifi_State.error++;
 *        if(wifi_State.error > ERROR_MAX)
 *        {
 *            return cWIFI_AT();
 *        }
 *    }
 *    return true;
 */
}
bool cWIFI_CheckTCP( void )
{
	uint32_t time = 2000;
	memset( httpPacket.content, 0x00, CMDLEN);
	sprintf( httpPacket.content, "AT+CIPSTART=%d,\"TCP\",\"%s\",%s\r\n", TCP_SERVER_ID, TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	return cWIFI_CmdSimple(httpPacket.content, "ALREADY CONNECT", time);  //测试连接，时间可以很短

 }

uint8_t cWIFI_TCPSend( uint32_t _msTimeOut )
{
	uint32_t	len;
	uint32_t	ulStart;
	char		cmd[32];
	char		buf[64]		= "\0";
	char		content[256]	= "\0";
	uint16_t	temp		= 0;

	ulStart = HAL_GetTick();
	printf( "Start making content\r\n" );
	sprintf( buf, "GET %s\r\n", "/api/hardware/status HTTP/1.1" );


/*
 * osDelay(100); printf("sprintf_len:%d\r\n",temp);
 * osDelay(100); printf("buf1:%s\r\n",buf);
 * memset(content, 0x00, 1);
 */
	strcat( content, buf );
	osDelay( 100 );
	printf( "content1:%s\r\n", content );
	sprintf( buf, "Host: %s\r\n\r\n", "alexyan.xyz:4000" );
	strcat( content, buf );

	len = strlen( content );
	osDelay( 100 );
	printf( "content:%s\r\n", content );
	osDelay( 100 );
	printf( "length:%d\r\n", len );
	sprintf( cmd, "AT+CIPSEND=%d", len );
	osDelay( 100 );
	printf( "Start sending length\r\n" );
	cWIFI_Cmd( cmd, "OK\r\n>", NULL, 2500 );
	osDelay( 2000 );

	printf( "Start sending content\r\n" );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) content, len, 2500 );
	memset( wifiData.buffer, 0x00, BUFFER_MAX );
	HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );

	/* _usTimeOut == 0 表示无限等待 */

	while ( 1 )
	{
		if ( ( (HAL_GetTick() - ulStart) > _msTimeOut) )
		{
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口1 */
			return(false);                  /* 超时 */
		}else if ( strstr( wifiData.buffer, "+IPD,62" ) )
		{
			osDelay( 10 );
			printf( "Receive OK\r\n" );
			printf( "%s", wifiData.buffer );
			return(true);
		}else {
			osDelay( 100 );
			printf( "%d\r\n", temp++ );
		}
	}
}


void HttpPacket_Init( void )
{
	memset( httpPacket.content, 0, CMDLEN );
	memset( httpPacket.url, 0, 32 );

	printf( "hardApi:::%s\r\n", TCP_SERVER_HARDAPI );
	printf( "host:::%s\r\n", httpPacket.host );
}


void HttpPacket_Url( char* key, char* addr )
{
	printf( "key:::%s\r\n", key );
	sprintf( httpPacket.url, "%s%s%s", TCP_SERVER_HARDAPI, key, addr );
	printf( "url:::%s\r\n", httpPacket.url );
}


void HttpPacket_Host( void )
{
	sprintf( httpPacket.host, "%s:%s", TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	printf( "host:::%s\r\n", httpPacket.host );
}


/*	cWIFI_Packet(GET, "/report","/HW001",TCP_SERVER_PORT,NULL); */

bool cWIFI_ServerAPI( API_TYPE apiType )
{
	char	key[]	= "/report";
	char	addr[]	= "/HW001";
	switch ( apiType )
	{
	case REPORT:

		return(cWIFI_Packet( GET, key, addr, NULL ) );
	default:
		break;


/*
 *    DOWNLOAD=1,
 *    FINISH=2,
 *    IMAGE=3,
 *      STATUS = 4
 */
	}
	return(false);
}


bool cWIFI_Packet( OPS_TYPE operType, char* key, char* addr, char* range )
{
	HttpPacket_Init();
	HttpPacket_Url( key, addr );
	HttpPacket_Host();

	char buf[64];

	memset( buf, 0, sizeof(buf) );
	switch ( operType )
	{
	case GET:
		sprintf( buf, "GET %s HTTP/1.1\r\n", httpPacket.url );
		strcat( httpPacket.content, buf );
		sprintf( buf, "Host: %s\r\n", httpPacket.host );
		strcat( httpPacket.content, buf );
		if ( range )
		{
			strcat( httpPacket.content, range );
			strcat( httpPacket.content, "\r\n" );
		}
		strcat( httpPacket.content, "\r\n" );
		break;

	case POST:
		sprintf( buf, "POST %s HTTP/1.1\r\n", httpPacket.url );
		strcat( httpPacket.content, buf );
		sprintf( buf, "Host: %s\r\n\r\n", httpPacket.host );
		strcat( httpPacket.content, buf );
		break;

	default:
		break;
	}
	httpPacket.length = strlen( httpPacket.content );
	printf( "%s\r\n", httpPacket.content );
	if ( httpPacket.length )
	{
		return(true);
	}else {
		return(false);
	}
}


uint16_t cWIFI_IPDLength( char *str )
{
	char	subStr[6]	= "\0";
	uint8_t i		= 0, j = 0;
	while ( str[i] != ':' )
	{
		if ( isdigit( str[i] ) )
		{
			subStr[j] = str[i];
			j++;
		}
		i++;
	}
	return(atoi( subStr ) );
}


uint16_t cWIFI_ParseNumber( char *str )
{
	uint16_t	num		= 0;
	char		subStr[6]	= "\0";
	uint8_t		i		= 0, j = 0;
/*	printf("cWIFI_ParseNumbe\r\n%s\r\n", str); */
	while ( str[i] != '\0' )
	{
		if ( isdigit( str[i] ) )
		{
			subStr[j] = str[i];
			j++;
		}else {
			memset( subStr, 0, 6 );
			j = 0;
		}
		i++;
	}

	num = atoi( subStr );
	printf( "cWIFI_ParseNumberxxxxxxxxxxxxx%d\r\n", num );
	return(num);
}


uint16_t cWIFI_ParseNum( char *str )
{
	uint16_t	num		= 0;
	char		subStr[6]	= "\0";
	char		*pHead;
	uint8_t		i = 0, j = 0;

/*	printf("============xxxxxxxxx====\r\n%s\r\n", str); */

	pHead = strstr( str, "+IPD" );
	if ( pHead )
	{
		while ( str[i] != ':' )
		{
			if ( isdigit( str[i] ) )
			{
				subStr[j] = str[i];
				j++;
			}
			i++;
		}
	}else {
		while ( str[i] != '\0' )
		{
			if ( isdigit( str[i] ) )
			{
				subStr[j] = str[i];
				j++;
			}
			i++;
		}
	}

	printf( "sssssssssssssssss%s\r\n", subStr );
	num = atoi( subStr );
	printf( "sssssssssssssssss%d\r\n", num );
	return(num);
}


uint16_t cWIFI_ServerReport( char *addr, uint32_t msTimeout )
{
	char		key[] = "/report";
	uint32_t	len;
	uint32_t	msTimeStart;
	char		cmd[32];
	char		buf[64]		= "\0";
	char		temp		= 0;
	char		*pHead		= NULL;
	char		*replyFirst	= "+IPD";
/*    char *replySecond = "Connection: keep-alive"; */
	uint16_t jsonLen = 0;

	msTimeStart = HAL_GetTick();
	
	memset( httpPacket.content, 0x00, CMDLEN );
	memset( wifiData.buffer, 0x00, BUFFER_MAX );
	
	sprintf( buf, "GET %s%s%s HTTP/1.1\r\n", TCP_SERVER_HARDAPI, key, addr );
	strcat( httpPacket.content, buf );
	sprintf( buf, "Host: %s:%s\r\n\r\n", TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	strcat( httpPacket.content, buf );

	len = strlen( httpPacket.content );
	sprintf( cmd, "AT+CIPSEND=%d,%d", TCP_SERVER_ID,len );
	printf( "yyyyyyyyyyyyyyyyyyyyyyyyyy\r\n" );
	printf( "%s\r\n", cmd );
	printf( "%s\r\n", httpPacket.content );

	cWIFI_CmdSimple( cmd, ">", WIFI_TX_TIMEOUT );

	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, len, WIFI_TX_TIMEOUT );
	osMutexRelease( WifiUart_MutexWriteHandle );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( HAL_Status != HAL_OK )
	{
		osMutexRelease( WifiUart_MutexReadHandle );
		return false;
	}
	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );

		if ( pHead )
		{
			pHead = strstr( pHead, "\r\n\r\n" );
			if ( !isdigit( pHead[strlen( pHead ) - 1] ) )
				osDelay( 300 );
			jsonLen = cWIFI_ParseNumber( pHead );
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			return(jsonLen);
		}else {
			printf( "ReportDelay%d\r\n", temp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


#define CRC32_POLYNOMIAL3 ( (uint32_t) 0xEDB88320)
uint32_t revbit( uint32_t uData )
{
	uint32_t uRevData = 0, uIndex = 0;
	uRevData |= ( (uData >> uIndex) & 0x01);
	for ( uIndex = 1; uIndex < 32; uIndex++ )
	{
		uRevData	<<= 1;
		uRevData	|= ( (uData >> uIndex) & 0x01);
	}
	return(uRevData);
}


/*==================================================================
 * Function  : CRC32_ForBytes
 * Description   : CRC32输入为8bits buffer的指针及长度
 * Input Para    :
 * Output Para   :
 * Return Value:
 *  ==================================================================*/
uint32_t crc32_Bytes( uint8_t *pData, uint32_t uLen )
{
	uint32_t uIndex = 0, uData = 0, i;
	uIndex = uLen >> 2;


	/* Reset CRC generator */
	CRC->CR = CRC_CR_RESET;

	while ( uIndex-- )
	{
#ifdef USED_BIG_ENDIAN
		uData = __REV( (uint32_t *) pData );
#else
		memcpy( (uint8_t *) &uData, pData, 4 );
#endif
		pData	+= 4;
		uData	= revbit( uData );
		CRC->DR = uData;
	}
	uData	= revbit( CRC->DR );
	uIndex	= uLen & 0x03;
	while ( uIndex-- )
	{
		uData ^= (uint32_t) *pData++;
		for ( i = 0; i < 8; i++ )
			if ( uData & 0x1 )
				uData = (uData >> 1) ^ CRC32_POLYNOMIAL3;
			else
				uData >>= 1;
	}
	return(uData ^ 0xFFFFFFFF);
}

	/* 发送数据 */


/*
 * osMutexWait(WifiUart_MutexWriteHandle, osWaitForever);
 *    HAL_Status = HAL_UART_Transmit(&WIFI_UART, (uint8_t*)cmd, strlen(cmd), WIFI_TX_TIMEOUT);
 *    if(HAL_Status!=HAL_OK) {
 *        printf("status1:%d\r\n",HAL_Status);
 *      osMutexRelease(WifiUart_MutexWriteHandle);
 *        return false;
 *    }
 * osMutexRelease(WifiUart_MutexWriteHandle);
 */
bool cWIFI_ServerDownload( char *addr, uint16_t start, uint16_t end, uint32_t msTimeout )
{
	char		key[]		= "/download";
	
	uint16_t	bytesLen = end - start + 1;
	
	uint32_t	crc32Result;
	uint32_t	crc32Receive;
	uint32_t  ReplyEnd = 0xA5A5A5A5;
	uint32_t  replyEnd = 0;
	
	uint16_t	strLen;
	
	uint32_t	msTimeStart;
	
	char		cmd[32];
	char		buf[64]		= "\0";
	char		temp		= 0;
	char		*pHead		= NULL;
	char		*replyFirst	= "+IPD";
	char		replySecond[10] = "\0";


	sprintf(replySecond, "%d\r\n\r\n", (bytesLen + 8));

	msTimeStart = HAL_GetTick();
	memset( httpPacket.content, 0x00, CMDLEN);
	memset( wifiData.buffer, 0x00, BUFFER_MAX );

	sprintf( buf, "GET %s%s%s HTTP/1.1\r\n", TCP_SERVER_HARDAPI, key, addr );
	strcat( httpPacket.content, buf );
	sprintf( buf, "Host: %s:%s\r\n", TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	strcat( httpPacket.content, buf );
	sprintf( buf, "Range: bytes=%d-%d\r\n\r\n", start, end );
	strcat( httpPacket.content, buf );

	strLen = strlen( httpPacket.content );
//	sprintf( cmd, "AT+CIPSEND=%d", strLen );
	sprintf( cmd, "AT+CIPSEND=%d,%d", TCP_SERVER_ID,strLen );
	cWIFI_CmdSimple( cmd, ">", 1000 );

	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, strLen, 2000 );
	osMutexRelease( WifiUart_MutexWriteHandle );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if ( HAL_Status != HAL_OK )
	{
		osMutexRelease( WifiUart_MutexReadHandle );
		return false;
	}
	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
		if ( pHead )
		{
			pHead = strstr( pHead, replySecond );
			if ( pHead )
			{
				wifiData.pData	= (uint8_t *) (pHead + strlen( replySecond ) );
				crc32Result = crc32_Bytes( wifiData.pData, bytesLen);
				printf( "CRC32_ForBytes3:%#20x\r\n", crc32Result );

				crc32Receive = 0;
				for ( char i = 0; i < 4; i++ )
				{crc32Receive = (crc32Receive << 8) + wifiData.pData[i + bytesLen];}
				for ( char i = 4; i < 8; i++ )
				{replyEnd = (replyEnd << 8) + wifiData.pData[i + bytesLen];}
				printf( "Server number:%#20x\r\n", crc32Receive );

				if(replyEnd == ReplyEnd)
				{
					HAL_UART_DMAStop( &WIFI_UART );
					osMutexRelease( WifiUart_MutexReadHandle );
					if ( crc32Receive == crc32Result )
					{
						printf( "-------------Time:%dms\r\n", (HAL_GetTick() - msTimeStart) );
						return(true);
					}
					else {
					return(false); /* 收到指定的应答数据，返回成功 */
					}
				}

			}
		}else {
			printf( "DownDelay:%d\r\n", temp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


bool cWIFI_ServerFinish( char *addr, uint32_t msTimeout )
{
	char		key[] = "/finish";
	uint32_t	len;
	uint32_t	msTimeStart;
	char		cmd[32];
	char		buf[64]		= "\0";
	char		temp		= 0;
	char		*pHead		= NULL;
	char		*replyFirst	= "+IPD";
	char		*replySecond	= "200 OK";

	msTimeStart = HAL_GetTick();
	memset( httpPacket.content, 0x00, CMDLEN);
	memset( wifiData.buffer, 0x00, BUFFER_MAX );

	sprintf( buf, "GET %s%s%s HTTP/1.1\r\n", TCP_SERVER_HARDAPI, key, addr );
	strcat( httpPacket.content, buf );
	sprintf( buf, "Host: %s:%s\r\n\r\n", TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	strcat( httpPacket.content, buf );

	len = strlen( httpPacket.content );
//	sprintf( cmd, "AT+CIPSEND=%d", len );
	sprintf( cmd, "AT+CIPSEND=%d,%d", TCP_SERVER_ID,len );
	cWIFI_Cmd( cmd, "OK\r\n>", NULL, 3600 );

	printf( "yyyyyyyyyyyyyyyyyyyyyyyyyy\r\n" );
	printf( "xxx%s\r\n", httpPacket.content );

	osMutexWait( WifiUart_MutexWriteHandle, osWaitForever );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, len, WIFI_TX_TIMEOUT );
	osMutexRelease( WifiUart_MutexWriteHandle );

	osMutexWait( WifiUart_MutexReadHandle, osWaitForever );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if(HAL_Status!=HAL_OK)
	{
		printf("Hello");
		HAL_UART_DMAStop( &WIFI_UART );
		osMutexRelease( WifiUart_MutexReadHandle );
		return false;
	}
	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
		if ( pHead )
		{
			pHead = strstr( pHead, replySecond );
			if ( pHead )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				osMutexRelease( WifiUart_MutexReadHandle );
				printf( "::::::%s\r\n", pHead );
				return(true); /* 收到指定的应答数据，返回成功 */
			}
		}else {
			printf( "%d\r\n", temp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			HAL_UART_DMAStop( &WIFI_UART );
			osMutexRelease( WifiUart_MutexReadHandle );
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


uint8_t cWIFI_ServerStatus( char *addr, uint32_t msTimeout )
{
	char		key[] = "/status";
	uint32_t	len;
	uint32_t	msTimeStart;
	char		cmd[32];
	char		buf[64]		= "\0";
	char		temp		= 0;
	char		*pHead		= NULL;
	char		*replyFirst	= "\r\n\r\n+IPD";
	char		*replySecond	= ":{";
	uint16_t	jsonLen		= 0;
	char		jsonLenStr[6]	= "\0";

	msTimeStart = HAL_GetTick();

	sprintf( buf, "GET %s%s HTTP/1.1\r\n", TCP_SERVER_HARDAPI, key );
	strcat( httpPacket.content, buf );
	sprintf( buf, "Host: %s:%s\r\n\r\n", TCP_SERVER_ADDRESS, TCP_SERVER_PORT );
	strcat( httpPacket.content, buf );

	len = strlen( httpPacket.content );
	sprintf( cmd, "AT+CIPSEND=%d", len );
	cWIFI_Cmd( cmd, "OK\r\n>", NULL, 3600 );

	printf( "yyyyyyyyyyyyyyyyyyyyyyyyyy\r\n" );
	printf( "xxx%s\r\n", httpPacket.content );

	memset( wifiData.buffer, 0x00, BUFFER_MAX );
	HAL_UART_Transmit( &WIFI_UART, (uint8_t *) httpPacket.content, len, 2500 );
	WIFI_UART.State = HAL_UART_STATE_READY;
	HAL_Status = HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );
	if(HAL_Status!=HAL_OK)
	{return false;}

	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
		if ( pHead )
		{
			printf( "............%s\r\n", pHead );
			pHead = strchr( pHead, ',' );
			printf( "-----.%s\r\n", pHead );
			uint8_t i = 0, j = 0;
			while ( pHead[i] != ':' )
			{
				if ( isdigit( pHead[i] ) )
				{
					jsonLenStr[j] = pHead[i];
					j++;
				}
				i++;
			}
			printf( "sssssssssssssssss%s\r\n", jsonLenStr );
			jsonLen = atoi( jsonLenStr );
			printf( "sssssssssssssssss%d\r\n", jsonLen );
			pHead = strstr( pHead, replySecond );
			if ( pHead )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				printf( "::::::%s\r\n", pHead );
				return(true); /* 收到指定的应答数据，返回成功 */
			}
		}else {
			printf( "RStatusDelay:%d\r\n", temp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


bool cWIFI_Cmd2( char * cmd, char * replyFirst, char * replySecond, uint32_t msTimeout )
{
	HAL_StatusTypeDef	status = HAL_OK;
	uint32_t		msTimeStart;
	char			*pHead	= NULL;
	uint8_t			tmp	= 0;

	/* 发送数据 */
	status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) cmd, strlen( cmd ), WIFI_TX_TIMEOUT );
	if ( status != HAL_OK )
	{
		return(false);
	}
	status = HAL_UART_Transmit( &WIFI_UART, (uint8_t *) "\r\n", 2, WIFI_TX_TIMEOUT );
	if ( status != HAL_OK )
	{
		return(false);
	}

	/* 不需要接收数据 */
	if ( (replyFirst == 0) && (replySecond == 0) )
	{
		return(true);
	}

	msTimeStart = HAL_GetTick();
	memset( wifiData.buffer, 0, BUFFER_MAX );
	HAL_UART_Receive_DMA( &WIFI_UART, (uint8_t *) wifiData.buffer, BUFFER_MAX );

	while ( 1 )
	{
		pHead = strstr( wifiData.buffer, replyFirst );
		if ( pHead )
		{
			if ( replySecond == NULL )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				printf( "Command OK================\r\n" );
				return(true);   /* 收到指定的应答数据，返回成功 */
			}else if ( (replySecond != NULL) && strstr( pHead, replySecond ) )
			{
				HAL_UART_DMAStop( &WIFI_UART );
				printf( "Command OK----------------\r\n" );
				return(true);   /* 收到指定的应答数据，返回成功 */
			}
		}else {
			printf( "%d\r\n", tmp++ );
			osDelay( 100 );
		}
		if ( ( (HAL_GetTick() - msTimeStart) > msTimeout) )
		{
			printf( "Timeout\r\n" );        /* 将接收到数据打印到调试串口 */
			return(false);                  /* 超时 */
		}
	}
}


void cWIFI_CIPCLOSE( void )
{
	wifi_State.error = 0;
	while ( cWIFI_Cmd( "AT+CIPCLOSE", "CLOSE", "OK", 500 ) == 0 )
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			cWIFI_AT();
			wifi_State.state = 1;
			break;
		}
	}
	wifi_State.state = 2;
}


void cWIFI_CIPSEND( void )
{
	char cCmd [20];
	wifi_State.error = 0;

	sprintf( cCmd, "AT+CIPSEND=%d\r\n", httpPacket.length );
	while ( cWIFI_Cmd( cCmd, "> ", 0, 3000 ) == 0 )
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			cWIFI_AT();
			break;
		}
	}
}


bool cWIFI_StrSend( char * pStr )
{
	wifi_State.error = 0;

	while ( cWIFI_Cmd( pStr, "SEND OK", "success", 1000 ) == 0 )
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			cWIFI_AT();
			break;
		}
	}

	return( (bool) wifi_State.error);
}


void cWIFI_State( uint8_t state )
{
	switch ( state )
	{
	case 0:
		cWIFI_Rst();
		state = 1;
		break;
	case 1:
		cWIFI_Cmd( "AT", "OK", NULL, 250 );
	}
	while ( !cWIFI_Cmd( "AT", "OK", NULL, 250 ) )
	{
		cWIFI_Rst();
	}
}


bool cWIFI_SendString( char * pStr )
{
	char		cCmd [20];
	bool		bRet = false;
	uint16_t	ulStrLength;

	ulStrLength = strlen( pStr );

	sprintf( cCmd, "AT+CIPSEND=%d", ulStrLength );

	cWIFI_Cmd( cCmd, "> ", 0, 2000 );

	bRet = cWIFI_Cmd( pStr, "SEND OK", 0, 1000 );


	return(bRet);
}


void cWIFI_CWLAP( void )
{
	wifi_State.error = 0;
	while ( cWIFI_Cmd( "AT+CWLAP", "OK", NULL, 200 ) == 0 )
	{
		wifi_State.error++;
		if ( wifi_State.error > WIFI_ERROR_MAX )
		{
			cWIFI_AT();
			wifi_State.state = 1;
			break;
		}
	}
	wifi_State.state = 2;
}


void cWIFI_StateCheck( char *cmd, uint8_t state )
{
	char buf[16];

	*buf = *cmd;
	strcat( buf, "?" );
	if ( state == 0 ) /*不透明传输:AT+CIPMODE=0 */
	{
		if ( cWIFI_Cmd( "AT+CIPMODE?", "+CIPMODE:0", NULL, 2000 ) == true )
		{
			printf( "AT+CIPMODE?=0" );
			wifi_State.cipmode = 0;
		}else {
			cWIFI_Cmd( "AT+CIPMODE=0", "+CIPMODE:0", NULL, 2000 );
			printf( "AT+CIPMODE=0" );
			wifi_State.cipmode = 0;
		}
	}else if ( state == 1 ) /* 透明传输:AT+CIPMODE=1 */
	{
		if ( cWIFI_Cmd( "AT+CIPMODE?", "+CIPMODE:1", NULL, 2000 ) == true )
		{
			printf( "AT+CIPMODE?=1" );
			wifi_State.cipmode = 1;
		}else {
			cWIFI_Cmd( "AT+CIPMODE=1", "+CIPMODE:1", NULL, 2000 );
			printf( "AT+CIPMODE=1" );
			wifi_State.cipmode = 1;
		}
	}
}


