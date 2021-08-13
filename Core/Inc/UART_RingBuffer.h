/*
 * UART_RingBuffer.h
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#ifndef UARTRINGBUFFER_H_
#define UARTRINGBUFFER_H_

#include "stm32l4xx_hal.h"
#include <stdbool.h>

/* size of the buffer */
#define UART_BUFFER_SIZE 256

typedef struct
{
  unsigned char 		buffer[UART_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
} ring_buffer;


void Init_RingBuffer(void);
int RingBuffer_Read(void);
bool RingBuffer_Write(int c);
void RingBuffer_WriteString(const char *s);
void RingBuffer_WriteLenghtString(const unsigned char *s, int datalen);
int RingBuffer_isDataToRead(void);
void RingBuffer_Clear (void);
int RingBuffer_SeeContent();
int RingBuffer_WaitForGivenResponse (char *string);
int RingBuffer_Receive (char *string, uint8_t numberofchars, char *buffertosave, int *result);
void UART_IRQHandler ();


#endif /* UARTRINGBUFFER_H_ */
