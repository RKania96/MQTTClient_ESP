/*
 * NetworkWraper.c
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#include <NetworkWrapper.h>
#include "ESP.h"
#include <string.h>
#include <stdbool.h>

static int NetworkSendState = 0;
static int NetworkRecvState = 0;


int NetworkSend(unsigned char *pMqttData, unsigned int dataLength)
{
	bool espResult = 0;

	// State Machine.
	switch(NetworkSendState) {
	case 0:
		//ESP Init -> go to next state
		if (ESP_Init(WIFI_SSID, WIFI_PASS) ) { NetworkSendState++; }
		break;
	case 1:
		// WiFi connection status -> go to next state
		if( ESP_IsConnected() ) { NetworkSendState++; }
		break;
	case 2:
		// Start TCP connection -> go to next state
		if( ESP_StartTCP(NETWORK_HOST, NETWORK_PORT, CONNECTION_KEEPALIVE_S, NETWORK_SSL) ) { NetworkSendState++; }
		break;
	case 3:
		// Send the data -> return the actual number of bytes. Stay in this state unless error occurs.
		if( ESP_Send(pMqttData, dataLength) ) { return dataLength; }
		break;
	default:
		// Reset the state machine.
		NetworkSendState = 0;
	}

	// Fall-back on error
	if(espResult == 0) { return -1; }

	// In progress.
	return 0;
}

int NetworkRecv(unsigned char *pMqttData, unsigned int maxBytes)
{
	bool espResult;
	static char receiveBuffer[128];

	espResult = ESP_Receive(receiveBuffer, 128);

	// Return the count.
	return espResult;
}
