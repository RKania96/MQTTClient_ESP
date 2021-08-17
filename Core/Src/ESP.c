/*
 * ESP.c
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#include "UART_RingBuffer.h"
#include "ESP.h"
#include "stdio.h"
#include "string.h"

// ESP Event strings
#define ESP_SEND_AT_RESTORE 	"AT+RESTORE\r\n"
#define ESP_SEND_AT				"AT\r\n"
#define ESP_RESP_AT				"AT\r\r\n\r\nOK\r\n"
#define ESP_SEND_AT_CWMODE 		"AT+CWMODE=1\r\n"
#define ESP_RESP_AT_CWMODE 		"AT+CWMODE=1\r\r\n\r\nOK\r\n"
#define ESP_SEND_AT_CWJAP 		"AT+CWJAP=\"%s\",\"%s\"\r\n"
#define ESP_RESP_AT_CWJAP		"WIFI GOT IP\r\n\r\nOK\r\n"
#define ESP_SEND_AT_CIPSTATUS	"AT+CIPSTATUS\r\n"
#define ESP_RESP_AT_CIPSTATUS	"STATUS:2\r\n\r\nOK\r\n"
#define ESP_SEND_AT_CIPSSLSIZE 	"AT+CIPSSLSIZE=4096\r\n"
#define ESP_SEND_AT_CIPSTART 	"AT+CIPSTART=\"%s\",\"%s\",%i,%i\r\n"
#define ESP_RESP_AT_CIPSTART	"CONNECT\r\n\r\nOK\r\n"
#define ESP_SEND_AT_CIPSEND		"AT+CIPSEND=%i\r\n"
#define ESP_RESP_ENTER_DATA		">"
#define ESP_RESP_SEND_OK		"SEND OK"
#define ESP_RESP_IPD			"+IPD,"

#define ESP_OK		1
#define ESP_ERROR	0

int ESP_WaitResult(int result)
{
	if( result < 0 )  { return ESP_ERROR; }
	if( result == 1 ) { return ESP_OK; }
}

/**
  * ESP init function
  *
*/
int ESP_Init(char *SSID, char *PASSWD)
{
    char command[128];

	Init_RingBuffer();

	//Reset ESP
	RingBuffer_WriteString(ESP_SEND_AT_RESTORE);

	//Wait 5s to restore
	HAL_Delay(5000);

	RingBuffer_WriteString(ESP_SEND_AT);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_AT)))) { return ESP_ERROR; }

	RingBuffer_WriteString(ESP_SEND_AT_CWMODE);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CWMODE)))) { return ESP_ERROR; }

	sprintf (command, ESP_SEND_AT_CWJAP, SSID, PASSWD);
	RingBuffer_WriteString(command);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CWJAP)))) { return ESP_ERROR; }

	return ESP_OK;
}

int ESP_IsConnected(void)
{
	RingBuffer_WriteString(ESP_SEND_AT_CIPSTATUS);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CIPSTATUS)))) { return ESP_ERROR; }

	return ESP_OK;
}

int ESP_StartTCP(const char * host, const uint16_t port, const uint16_t keepAlive, const uint8_t ssl)
{
    char command[128];

    if(ssl)
    {
    	// ESP8266 module memory (2048 to 4096) reserved for SSL
    	RingBuffer_WriteString(ESP_SEND_AT_CIPSSLSIZE);
    	HAL_Delay(2000);
    }

	sprintf (command, ESP_SEND_AT_CIPSTART, (ssl ? "SSL" : "TCP"), host, port, keepAlive);
	RingBuffer_WriteString(command);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CIPSTART)))) { return ESP_ERROR; }

	return ESP_OK;
}

int ESP_Send(unsigned char *data, const uint8_t dataLength)
{
    char command[128];

    RingBuffer_Clear();

	sprintf(command, ESP_SEND_AT_CIPSEND, dataLength);
	RingBuffer_WriteString(command);
	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_ENTER_DATA)))) { return ESP_ERROR; }

	RingBuffer_WriteLenghtString(data,dataLength);

	if(!(ESP_WaitResult(RingBuffer_WaitForGivenResponse(ESP_RESP_SEND_OK)))) { return ESP_ERROR; }


	return ESP_OK;
}

int ESP_Receive(const char * const data, const uint8_t dataLength)
{
	int result = 0;

	// Receive the available data.
	while (!(RingBuffer_Receive(ESP_RESP_IPD, dataLength, data, &result)));

	return result;
}
