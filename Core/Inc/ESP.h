/*
 * ESP.h
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#ifndef ESP_H_
#define ESP_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

void ESP_Init();
bool ESP_IsConnected(void);
bool ESP_StartTCP(const char * host, const uint16_t port, const uint16_t keepalive, const bool ssl);
bool ESP_Send(unsigned char *data, const uint8_t dataLength);
int ESP_Receive(const char * const data, const uint8_t dataLength);


//void Server_Start (void);


#endif /* ESP_H_ */
