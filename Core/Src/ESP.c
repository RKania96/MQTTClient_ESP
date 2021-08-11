/*
 * ESP.c
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#include "UART_RingBuffer.h"
#include "ESP.h"
#include "WiFi_Credentials.h"
#include "stdio.h"
#include "string.h"


char ip_addr[15];

//static unsigned long int (* ESP82_getTime_ms)(void);///< Used to hold handler for time provider.
//static unsigned long int ESP82_t0;///< Keeps entry time for timeout detection.
//
//
///*
// * @brief INTERNAL Timeout setup.
// */
//static void ESP82_timeoutBegin(void){
//	// Get entry time.
//	ESP82_t0 = ESP82_getTime_ms();
//}
//
///*
// * @brief INTERNAL Timeout checker.
// * @param interval_ms Interval time in ms.
// * @return True if timeout expired.
// */
//static bool ESP_timeoutIsExpired(const uint16_t interval_ms) {
//	// Check if the given interval is in the past.
//	return (interval_ms < (ESP82_getTime_ms() - ESP82_t0));
//}


/**
  * ESP init function
  *
*/
void ESP_Init()
{
    char data[128];


	Init_RingBuffer();
	//RingBuffer_WriteString("AT+RST\r\n");
	RingBuffer_WriteString("AT+RESTORE\r\n");

	for (int i=0; i<5; i++)
	{
		HAL_Delay(1000);
	}

	/********* AT **********/
	RingBuffer_WriteString("AT\r\n");
	while(!(RingBuffer_WaitForGivenResponse("AT\r\r\n\r\nOK\r\n")));

	/********* AT+CWMODE=1 **********/
	RingBuffer_WriteString("AT+CWMODE=1\r\n");
	while (!(RingBuffer_WaitForGivenResponse("AT+CWMODE=1\r\r\n\r\nOK\r\n")));

	/********* AT+CWJAP="SSID","PASSWD" **********/
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
	RingBuffer_WriteString(data);
	while (!(RingBuffer_WaitForGivenResponse("WIFI GOT IP\r\n\r\nOK\r\n")));

	HAL_Delay(1000);
}

bool ESP_IsConnected(void)
{
	RingBuffer_WriteString("AT+CIPSTATUS\r\n");
	while (!(RingBuffer_WaitForGivenResponse("STATUS:2\r\n\r\nOK\r\n")));

	return true;
}

bool ESP_StartTCP(const char * host, const uint16_t port, const uint16_t keepalive, const bool ssl)
{

	bool result;
    char data[128];

	/********* AT+CIPSTART=... **********/
    if(ssl)
    {
    	RingBuffer_WriteString("AT+CIPSSLSIZE=4096\r\n"); // ESP8266 module memory (2048 to 4096) reserved for SSL.
    	HAL_Delay(2000);
    }

	sprintf (data, "AT+CIPSTART=\"%s\",\"%s\",%i,%i\r\n", (ssl ? "SSL" : "TCP"), host, port, keepalive);
	RingBuffer_WriteString(data);
	while (!(RingBuffer_WaitForGivenResponse("CONNECT\r\n\r\nOK\r\n")));

}

bool ESP_Send(unsigned char *data, const uint8_t dataLength)
{
	bool result;
    char databuff[128];

	/********* AT+CIPSEND=... **********/
	sprintf(databuff, "AT+CIPSEND=%i\r\n", dataLength);
	RingBuffer_WriteString(databuff);

	// Wait for ">"
	while (!(RingBuffer_WaitForGivenResponse(">")));

	//RingBuffer_WriteString(data);
	RingBuffer_WriteStrings(data,dataLength);

	// Wait for "SEND OK"
	while (!(RingBuffer_WaitForGivenResponse("SEND OK")));

	RingBuffer_Clear();
	return true;
}


//CIPCLOSE trzeba?

int ESP_Receive(const char * const data, const uint8_t dataLength)
{
	int result = 0;

	// Receive the available data.
	while (!(RingBuffer_Receive("+IPD,", dataLength, data, &result)));

	return result;
}





//int Server_Send (char *str, int Link_ID)
//{
//	int len = strlen (str);
//	char data[80];
//	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);
//	RingBuffer_WriteString(data);
//	while (!(RingBuffer_WaitForGivenResponse(">")));
//	RingBuffer_WriteString (str);
//	while (!(RingBuffer_WaitForGivenResponse("SEND OK")));
//	sprintf (data, "AT+CIPCLOSE=5\r\n");
//	RingBuffer_WriteString(data);
//	while (!(RingBuffer_WaitForGivenResponse("OK\r\n")));
//	return 1;
//}
//
//void Server_Handle (char *str, int Link_ID)
//{
//	char datatosend[1024] = {0};
//	if (!(strcmp (str, "/ledon")))
//	{
//		sprintf (datatosend, Basic_inclusion);
//		strcat(datatosend, LED_ON);
//		strcat(datatosend, Terminate);
//		Server_Send(datatosend, Link_ID);
//	}
//
//	else if (!(strcmp (str, "/ledoff")))
//	{
//		sprintf (datatosend, Basic_inclusion);
//		strcat(datatosend, LED_OFF);
//		strcat(datatosend, Terminate);
//		Server_Send(datatosend, Link_ID);
//	}
//
//	else
//	{
//		sprintf (datatosend, Basic_inclusion);
//		strcat(datatosend, LED_OFF);
//		strcat(datatosend, Terminate);
//		Server_Send(datatosend, Link_ID);
//	}
//
//}
//
//void Server_Start (void)
//{
//	char buftocopyinto[64] = {0};
//	char Link_ID;
//	while (!(RingBuffer_Receive("+IPD,", 1, &Link_ID)));
//	Link_ID -= 48;
//	while (!(Copy_upto(" HTTP/1.1", buftocopyinto)));
//	if (RingBuffer_FindString("/ledon", buftocopyinto) == 1)
//	{
//		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
//		Server_Handle("/ledon",Link_ID);
//	}
//
//	else if (RingBuffer_FindString("/ledoff", buftocopyinto) == 1)
//	{
//		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
//		Server_Handle("/ledoff",Link_ID);
//	}
//
//	else if (RingBuffer_FindString("/favicon.ico", buftocopyinto) == 1);
//
//	else
//	{
//		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
//		Server_Handle("/ ", Link_ID);
//	}
//}
