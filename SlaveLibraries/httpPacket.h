
#ifndef _HTTP_PACKET_H_
#define _HTTP_PACKET_H_

#include <stdint.h>

#define CMDLEN 256

typedef enum operation_type
{
    POST,
    PUT,
    GET,
    DELETE
}OPS_TYPE;

typedef enum api_type
{
    REPORT=0,
    DOWNLOAD=1,
    FINISH=2,
    IMAGE=3,
		STATUS = 4
}API_TYPE;

typedef struct http_packet
{
	char url[32];
	char host[32];
	char* contentType;
	char* contentLength;
	char  content[CMDLEN];
	uint16_t length;
	
}HttpPacketType;


#endif  //HTTP_PACKET_H_
