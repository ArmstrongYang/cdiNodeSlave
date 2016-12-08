/**
****************************************************************

File	:	cWIFI.h
Author	:	@hiyangdong
Version	:	V1.0
date	:	2015-05-13
brief	:	header file

*****************************************************************
*/
/**
  ******************************************************************************
  * File Name          : USART.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _CWIFI_H_
#define _CWIFI_H_

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"	
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmsis_os.h"
#include "usart.h"
#include "httpPacket.h"
#include "main.h"
#include "flash_if.h"
#include "boot.h"

		
#define BUFFER_MAX     		((uint16_t)1024)           
#define WIFI_TX_TIMEOUT				((uint32_t)1000)
#define WIFI_RX_TIMEOUT 				((uint32_t)1000)
#define WIFI_ERROR_MAX					((uint8_t)5)

typedef struct WIFIFormat
{
	char buffer[BUFFER_MAX];
	uint8_t *pData;
	HAL_StatusTypeDef status;
	uint8_t flag;
	uint16_t addr_hard;
	uint32_t addr_soft;
	uint8_t  size;
	uint8_t  cmd;
	uint8_t  data[6];
	uint16_t crc[2];	
}WIFI_DataType;

typedef enum{
	WIFI_CLOSE = 0,
	WIFI_SLEEP = 1,
	WIFI_RESET = 2,
	WIFI_CWJAP = 3,
	WIFI_TCP = 4,
	WIFI_SEND = 5
} ENUM_WifiState;

typedef struct wifi_state
{
	
	uint8_t state;
	ENUM_WifiState stateStart;
	ENUM_WifiState stateCurrent;
	ENUM_WifiState stateExpect;
	
	bool init;
	
	bool at;

	bool cwjap;
	bool tcp;
	bool cipmode;
	
	bool gotssid;
	
	bool report;
	
	char cwmode;
	
	char error;
	uint32_t baud;
	
}WIFI_StateType;

extern HttpPacketType httpPacket;
extern WIFI_StateType  wifiState;
extern WIFI_DataType wifiData;

extern char WIFI_SSID_NAME[32];
extern char WIFI_PASSWORD[];
//extern char TCP_SERVER_IPADDRESS[];
extern char TCP_SERVER_ADDRESS[];
extern char TCP_SERVER_PORT[];
	 
extern char TCP_CILENT_IPADDRESS[];
extern char TCP_CILENT_PORT[];

#define WIFI_UART  huart1
#define DEBUG_UART  huart2
/* ???????? */
typedef enum{
	STA,
  AP,
  STA_AP  
} ENUM_Net_ModeTypeDef;


typedef enum{
	 enumTCP,
	 enumUDP,
} ENUM_NetPro_TypeDef;
	

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID     = 5,
} ENUM_ID_NO_TypeDef;
	

typedef enum{
	OPEN = 0,
	WEP = 1,
	WPA_PSK = 2,
	WPA2_PSK = 3,
	WPA_WPA2_PSK = 4,
} ENUM_AP_PsdMode_TypeDef;


void cWIFI_Init(void);
void cWIFI_DeInit(void);
void WIFI_UART_Init(uint32_t Baud);
bool cWIFI_BaudCheck(void);


bool cWIFI_Rst(void);
bool cWIFI_AT(void);
void cWIFI_PD(uint8_t mode);
bool cWIFI_ATE(char *para);
bool cWIFI_CWMODE( char *para , char *reply1, char *reply2);
bool cWIFI_CIPSERVER( char *para , char *reply1, char *reply2);
bool cWIFI_CWSAP( char *ssid , char *pwr, char chl, char ecn);
bool cWIFI_WaitAP( uint32_t msTimeout );
bool cWIFI_CIPMODE(char *para);
bool cWIFI_CIPMUX(char *para);

bool cWIFI_CWJAP(char *pSSID, char *pPassword, uint32_t msTimeout);
bool cWIFI_JoinAP(void);
bool cWIFI_CheckAP( void );
bool cWIFI_CIPSTART(char *pIP, char *pPort, uint32_t msTimeout);
bool cWIFI_JoinTCP(void);
bool cWIFI_CheckTCP( void );

HAL_StatusTypeDef cWIFI_strWrite(char *pString);
HAL_StatusTypeDef cWIFI_strRead(char *pString, uint32_t timeout);

bool cWIFI_Cmd(char * cmd, char * replyFirst, char * replySecond, uint32_t msTimeout);
bool cWIFI_CmdSimple(char *cmd, char *reply, uint32_t msTimeout);

bool cWIFI_Packet(OPS_TYPE operType, char* key, char* addr, char* range);
bool cWIFI_ServerAPI(API_TYPE apiType);

uint8_t cWIFI_TCPSend(uint32_t _msTimeOut);
uint16_t cWIFI_ServerReport(char *addr, uint32_t msTimeout);
bool cWIFI_ServerDownload(char *addr, uint16_t start, uint16_t end, uint32_t msTimeout);
bool cWIFI_ServerFinish(char *addr, uint32_t msTimeout);
uint8_t cWIFI_ServerStatus(char *addr, uint32_t msTimeout);

void cWIFI_CWLAP ( void );
bool cWIFI_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode );
bool cWIFI_STAJoinTCP ( char * ip, char * port );
void cWIFI_CIPCLOSE ( void );
void cWIFI_CIPSEND ( void );
bool cWIFI_StrSend (char * pStr);
bool cWIFI_CWJAP ( char * pSSID, char * pPassWord, uint32_t timeout );










//#define     ESP8266_Usart( fmt, ... )           USART2_printf ( USART2, fmt, ##__VA_ARGS__ ) 
//#define     PC_Usart( fmt, ... )                printf ( fmt, ##__VA_ARGS__ )

//#define     ESP8266_CH_HIGH_LEVEL()             GPIO_SetBits( GPIOA, GPIO_Pin_0 )
//#define     ESP8266_CH_LOW_LEVEL()              GPIO_ResetBits( GPIOA, GPIO_Pin_0 )

//#define     ESP8266_RST_HIGH_LEVEL()            GPIO_SetBits( GPIOA, GPIO_Pin_1 )
//#define     ESP8266_RST_LOW_LEVEL()             GPIO_ResetBits( GPIOA, GPIO_Pin_1 )


//void        ESP8266_Choose                      ( FunctionalState enumChoose );
//void        ESP8266_Rst                         ( void );
//void        ESP8266_AT_Test                     ( void );
//bool        ESP8266_Cmd                         ( char * cmd, char * reply1, char * reply2, u32 waittime );
//bool        ESP8266_Net_Mode_Choose             ( ENUM_Net_ModeTypeDef enumMode );
//bool        ESP8266_JoinAP                      ( char * pSSID, char * pPassWord );
//bool        ESP8266_BuildAP                     ( char * pSSID, char * pPassWord, char * enunPsdMode );
//bool        ESP8266_Enable_MultipleId           ( FunctionalState enumEnUnvarnishTx );
//bool        ESP8266_Link_Server                 ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id);
//bool        ESP8266_StartOrShutServer           ( FunctionalState enumMode, char * pPortNum, char * pTimeOver );
//bool        ESP8266_TransparentTransmission               ( void );
//bool        ESP8266_SendString                  ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId );
//char *      ESP8266_ReceiveString               ( FunctionalState enumEnUnvarnishTx );

//void        ESP8266_STA_TCP_Client              ( void );
//void        ESP8266_AP_TCP_Server               ( void );
//void        ESP8266_StaTcpClient_ApTcpServer    ( void );
//void        Linktcp_LED_ON                      (void);
//void        LED_all_OFF                         (void);

#define AT     	"AT"    		/*  DES:    restart the module          */
														/*  FDB:	SUC:\r\nOK\r\n 6bytes  ERR:         */

#define ATE0		"ATE0"    	/*  DES:    restart the module          */

#define RST     "AT+RST"    /*  DES:    restart the module          */

/* WIFI     */
#define MODE    "AT+CWMODE" /*  DES:    wifi mode
                                SET:    AT+CWMODE=<mode>
                                INQ:    AT+CWMODE?
                                TST:    AT+CWMODE=?
                                PAR:    1= Sta, 2= AP, 3=both
                                EXP:    -                           */

#define CIPMODE "AT+CIPMODE"/*  DES:    TCPIP应用模式
                                SET:    AT+CWMODE=<mode>	RETURN:OK
                                INQ:    AT+CIPMODE?				RETURN:+CIPMODE: <mode> \r\n\r\nOK

                                TST:    
                                PAR:    0= 非透明传输模式，缺省模式, 1= 透明传输模式
                                EXP:    -                           */

#define CWJAP   "AT+CWJAP"  /*  DES:    join the AP
                                SET:    AT+ CWJAP =<ssid>,< pwd >
                                INQ:    AT+ CWJAP?
                                TST:    
                                PAR:    ssid=ssid, pwd=password
                                EXP:    -                           */

#define CWLAP   "AT+CWLAP"  /*  DES:    list the AP
                                SET:    AT+CWLAP                    */

#define CWQAP   "AT+CWQAP"  /*  DES:    quit the AP
                                SET:    AT+CWQAP
                                INQ:    -
                                TST:    AT+CWQAP=?                  */

#define CWSAP   "AT+CWSAP"  /*  DES:    set the AP
                                SET:    AT+ CWSAP= <ssid>,<pwd>,<chl>, <ecn>
                                INQ:    AT+ CWSAP?
                                TST:    -
                                PAR:    ssid, pwd, chl = channel, ecn = encryption
                                EXP:    AT+CWJAP="YOURSSID","helloworld"    */

/* TCP/IP   */
#define CIPSTATUS  "AT+CIPSTATUS"   /*  DES:    get the connection status
                                        SET:    AT+ CIPSTATUS               */

#define CIPSTART   "AT+CIPSTART"    /*  DES:    set up TCP or UDP connection
                                        SET:    1)single connection (+CIPMUX=0):    AT+CIPSTART= <type>,<addr>,<port>
                                                2) multiple connection (+CIPMUX=1): AT+CIPSTART= <id><type>,<addr>, <port>
                                            INQ:    -
                                            TST:    AT+CIPSTART=?
                                            PAR:    id = 0-4, type = TCP/UDP, addr = IP address, port= port
                                            EXP:    Connect to another TCP server, set multiple connection first:
                                                    AT+CIPMUX=1; connect: AT+CIPSTART=4,"TCP","X1.X2.X3.X4",9999    */

#define CIPSEND    "AT+CIPSEND"    /*  DES:    send data
                                       SET:    1)single connection(+CIPMUX=0) AT+CIPSEND=<length>
                                               2) multiple connection (+CIPMUX=1) AT+CIPSEND= <id>,<length>
                                       INQ:    -
                                       TST:    AT+CIPSEND=?
                                       PAR:    -
                                       EXP:    send data: AT+CIPSEND=4,15 and then enter the data  */

#define CIPCLOSE   "AT+CIPCLOSE"   /*  DES:    close TCP or UDP connection
                                       SET:    AT+CIPCLOSE=<id> or AT+CIPCLOSE
                                       INQ:    -
                                       TST:    AT+CIPCLOSE=?     */

#define CIFSR      "AT+CIFSR"      /*  DES:    Get IP address
                                       SET:    AT+CIFSR
                                       TST:    AT+ CIFSR=?    */

#define CIPMUX     "AT+CIPMUX"     /*  DES:    set mutiple connection
                                       SET:    AT+ CIPMUX=<mode>
                                       INQ:    AT+ CIPMUX?
                                       PAR:    0 for single connection 1 for mutiple connection   */

#define CIPSERVER  "AT+CIPSERVER"  /*  DES:    set as server
                                       SET:    AT+CIPSERVER= <mode>[,<port> ]
                                       PAR:    mode 0 to close server mode, mode 1 to open; port = port
                                       EXP:    turn on as a TCP server: AT+CIPSERVER=1,8888
                                               check the self server IP address: AT+CIFSR=?    */

#define IPD        "+IPD"          /*  DES:    received data   */



#endif    /* __WIFI_CONFIG_H */


#ifdef __cplusplus
}
#endif 


