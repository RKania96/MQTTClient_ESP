/*
 * ESP.h
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#ifndef ESP_H_
#define ESP_H_


#include <stdlib.h>
#include <stdint.h>

int ESP_Init(char *SSID, char *PASSWD);
int ESP_IsConnected(void);
int ESP_StartTCP(const char * host, const uint16_t port, const uint16_t keepAlive, const uint8_t ssl);
int ESP_Send(unsigned char *data, const uint8_t dataLength);


#endif /* ESP_H_ */
