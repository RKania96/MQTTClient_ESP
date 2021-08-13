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

#define TIMEOUT_MS 		15000
#define ONE_CYCLE_TIME 	0.0000000125

static unsigned long int ticks_t0;
static uint8_t cnt = 0;

/**
  * GetTime Init function
*/
void GetTime_Init()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	ITM->LAR = 0xC5ACCE55;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * Return time in ms
*/
long unsigned int GetTime_ticks(void) {

	return DWT->CYCCNT;
}

/**
  * Start timer
*/
static void GetTime_timeoutBegin(void)
{
	ticks_t0 = GetTime_ticks();
}

/**
  * Return True if timeout expired
*/
static bool GetTime_timeoutIsExpired(const uint16_t interval_ms)
{
	if(!cnt)
	{
		GetTime_timeoutBegin();
		cnt++;
	}

	long unsigned int ticks = (GetTime_ticks() - ticks_t0);
	long unsigned int time_ms = ticks * ONE_CYCLE_TIME;

	if(interval_ms < time_ms)
	{
		cnt--;
		return true;
	}

	return false;
}

/**
  * ESP init function
  *
*/
bool ESP_Init(char *SSID, char *PASSWD)
{
    char command[128];

	Init_RingBuffer();
	GetTime_Init();

	//Reset ESP
	RingBuffer_WriteString(ESP_SEND_AT_RESTORE);

	//Wait 5s to restore
	HAL_Delay(5000);

	RingBuffer_WriteString(ESP_SEND_AT);
	while(!(RingBuffer_WaitForGivenResponse(ESP_RESP_AT))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	RingBuffer_WriteString(ESP_SEND_AT_CWMODE);
	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CWMODE))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	sprintf (command, ESP_SEND_AT_CWJAP, SSID, PASSWD);
	RingBuffer_WriteString(command);
	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CWJAP))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	//HAL_Delay(1000);

	return true;
}

bool ESP_IsConnected(void)
{
	RingBuffer_WriteString(ESP_SEND_AT_CIPSTATUS);
	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CIPSTATUS))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	return true;
}

bool ESP_StartTCP(const char * host, const uint16_t port, const uint16_t keepAlive, const bool ssl)
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
	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_AT_CIPSTART))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	return true;
}

bool ESP_Send(unsigned char *data, const uint8_t dataLength)
{
    char command[128];

    RingBuffer_Clear();

	sprintf(command, ESP_SEND_AT_CIPSEND, dataLength);
	RingBuffer_WriteString(command);

	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_ENTER_DATA))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	RingBuffer_WriteLenghtString(data,dataLength);
	while (!(RingBuffer_WaitForGivenResponse(ESP_RESP_SEND_OK))) { if(GetTime_timeoutIsExpired( TIMEOUT_MS )) { return false; } }

	return true;
}

int ESP_Receive(const char * const data, const uint8_t dataLength)
{
	int result = 0;

	// Receive the available data.
	while (!(RingBuffer_Receive(ESP_RESP_IPD, dataLength, data, &result)));

	return result;
}
