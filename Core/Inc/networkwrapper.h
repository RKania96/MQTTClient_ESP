/*
 * NetworkWraper.h
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#ifndef _NETWORKWRAPPER_H
#define _NETWORKWRAPPER_H

#include "Settings.h"

#ifndef network_socket_t
#define network_socket_t void*
#endif

int NetworkSend(unsigned char *pMqttData, unsigned int dataLength);
int NetworkRecv(unsigned char *pMqttData, unsigned int maxBytes);

#endif
