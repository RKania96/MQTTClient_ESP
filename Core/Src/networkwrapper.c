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

#define ESP_OK 1
#define ESP_ERROR -1

static int NetworkSendState = 0;
static int NetworkRecvState = 0;


int NetworkSend(unsigned char *pMqttData, unsigned int dataLength)
{
	int espResult;

	// State Machine.
	switch(NetworkSendState) {
	case 0:
		//ESP Init -> go to next state
		espResult = ESP_Init(WIFI_SSID, WIFI_PASS);
		if (ESP_OK == espResult) { NetworkSendState++; }
		break;
	case 1:
		// WiFi connection status -> go to next state
		espResult = ESP_IsConnected();
		if (ESP_OK == espResult) { NetworkSendState++; }
		break;
	case 2:
		// Start TCP connection -> go to next state
		espResult = ESP_StartTCP(NETWORK_HOST, NETWORK_PORT, CONNECTION_KEEPALIVE_S, NETWORK_SSL);
		if (ESP_OK == espResult) { NetworkSendState++; }
		break;
	case 3:
		// Send the data -> return the actual number of bytes. Stay in this state unless error occurs.
		espResult = ESP_Send(pMqttData, dataLength);
		if(ESP_OK == espResult){ return dataLength; }
		break;
	default:
		// Reset the state machine.
		NetworkSendState = 0;
	}

	// Fall-back on error
	if(espResult == ESP_ERROR) { return ESP_ERROR; }

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
