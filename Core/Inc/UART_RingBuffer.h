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
  unsigned char buffer[UART_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
} ring_buffer;


void Init_RingBuffer(void);
int RingBuffer_Read(void);
bool RingBuffer_Write(int c);
void RingBuffer_WriteString(const char *s);
int RingBuffer_isDataToRead(void);
void RingBuffer_Clear (void);
int RingBuffer_SeeContent();
int RingBuffer_WaitForGivenResponse (char *string);

int RingBuffer_FindString (char *str, char *rbuf);
void UART_IRQHandler ();


/* Copies the required data from a buffer
 * @startString: the string after which the data need to be copied
 * @endString: the string before which the data need to be copied
 * @USAGE:: GetDataFromBuffer ("name=", "&", buffertocopyfrom, buffertocopyinto);
 */
void GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto);

/* Copy the data from the Rx buffer into the bufferr, Upto and including the entered string 
* This copying will take place in the blocking mode, so you won't be able to perform any other operations
* Returns 1 on success and -1 otherwise
* USAGE: while (!(Copy_Upto ("some string", buffer)));
*/
int Copy_upto (char *string, char *buffertocopyinto);

/* Copies the entered number of characters (blocking mode) from the Rx buffer into the buffer, after some particular string is detected
* Returns 1 on success and -1 otherwise
* USAGE: while (!(Get_after ("some string", 6, buffer)));
*/
int Get_after (char *string, uint8_t numberofchars, char *buffertosave);


#endif /* UARTRINGBUFFER_H_ */
