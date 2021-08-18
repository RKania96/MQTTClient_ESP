/*
 * GetTime.h
 *
 *  Created on: 18 sie 2021
 *      Author: rk
 */

#ifndef INC_GETTIME_H_
#define INC_GETTIME_H_

#include "stm32l4xx_hal.h"

void GetTime_Init();
void GetTime_timeoutBegin(void);
int GetTime_timeoutIsExpired();

#endif /* INC_GETTIME_H_ */
