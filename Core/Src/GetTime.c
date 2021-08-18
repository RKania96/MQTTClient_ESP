/*
 * GetTime.c
 *
 *  Created on: 18 sie 2021
 *      Author: rk
 */

#include "GetTime.h"

#define TIMEOUT_TICKS 		800000000

static unsigned long int ticks_t0;

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
void GetTime_timeoutBegin(void)
{
	ticks_t0 = GetTime_ticks();
}

/**
  * Return True if timeout expired
*/
int GetTime_timeoutIsExpired()
{

	long unsigned int ticks = (GetTime_ticks() - ticks_t0);

	if(TIMEOUT_TICKS < ticks)
	{
		return 1;
	}

	return 0;
}

