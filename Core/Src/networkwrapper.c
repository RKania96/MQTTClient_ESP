/**
 * @file      networkwrapper.c
 * @author    Atakan S.
 * @date      01/01/2019
 * @version   1.0
 * @brief     Network wrapper for PAHO MQTT project based on ESP8266.
 *
 * @copyright Copyright (c) 2018 Atakan SARIOGLU ~ www.atakansarioglu.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

// Includes.
#include "networkwrapper.h"
#include "ESP.h"
#include <string.h>
#include <stdbool.h>

// Variables.
//static char network_host[32] = "broker.emqx.io";///< HostName i.e. "test.mosquitto.org"
static char network_host[32] = "192.168.0.111";///< HostName i.e. "test.mosquitto.org"
static unsigned short int network_port = 8883;///< Remote port number.
static unsigned short int network_keepalive = 60;///< Default keepalive time in seconds.
static bool network_ssl = true;///< SSL is disabled by default.
static int network_send_state = 0;///< Internal state of send.
static int network_recv_state = 0;///< Internal state of recv.


int network_send(unsigned char *address, unsigned int bytes){

	bool espResult;

	// State Machine.
	switch(network_send_state) {
	case 0:
		// Init ESP8266 driver.
		ESP_Init();

		espResult=1;

		// To the next state.
		network_send_state++;

		break;
	case 1:
		// Check the wifi connection status.
		espResult = ESP_IsConnected();
		if(espResult == 1){
			// To the next state.
			network_send_state++;
		}
		break;
	case 2:
		// Start TCP connection.
		ESP_StartTCP(network_host, network_port, network_keepalive, network_ssl);

		// To the next state.
		network_send_state++;
		break;
	case 3:
		// Send the data.
		espResult = ESP_Send(address, bytes);
		if(espResult == 1){
			// Return the actual number of bytes. Stay in this state unless error occurs.
			return bytes;
		}
		break;
	default:
		// Reset the state machine.
		network_send_state = 0;
	}

	// Fall-back on error.
	if(espResult == 0){
		if(network_send_state < 4){
			// If error occured before wifi connection, start over.
			network_send_state = 0;
		}else{
			// Check wifi connection and try to send again.
			network_send_state = 2;
		}

		// Error.
		return -1;
	}

	// In progress.
	return 0;
}

int network_recv(unsigned char *address, unsigned int maxbytes)
{
	bool espResult;
	static char receiveBuffer[128];

	espResult = ESP_Receive(receiveBuffer, 128);

	// Return the count.
	return espResult;
}
