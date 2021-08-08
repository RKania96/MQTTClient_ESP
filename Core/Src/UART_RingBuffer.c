/*
 * UART_RingBuffer.c
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#include "UART_RingBuffer.h"
#include <string.h>

#define E_OK	1
#define E_ERROR 0
#define E_NOK   -1

//#define INCREMENT_TAIL(x) ((unsigned int)(x->tail + 1) % UART_BUFFER_SIZE)

/* definition of used UART  */

UART_HandleTypeDef huart1;
#define UART huart1

/* Initialization of RS and TX buffers  */

ring_buffer RX_Buffer = { { 0 }, 0, 0};
ring_buffer TX_Buffer = { { 0 }, 0, 0};

ring_buffer *pRX_Buffer;
ring_buffer *pTX_Buffer;

/* Functions declaration  */

bool RingBuffer_PutChar(unsigned char c, ring_buffer *buffer);

/**
  * USART1 Initialization Function
  * Initialize the ring buffer
  *
*/
void Init_RingBuffer(void)
{
	/* Initialize UART */
	UART.Instance = USART1;
	UART.Init.BaudRate = 115200;
	UART.Init.WordLength = UART_WORDLENGTH_8B;
	UART.Init.StopBits = UART_STOPBITS_1;
	UART.Init.Parity = UART_PARITY_NONE;
	UART.Init.Mode = UART_MODE_TX_RX;
	UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART.Init.OverSampling = UART_OVERSAMPLING_16;
	UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&UART) != HAL_OK)
	{
		  __disable_irq();
		  while (1)
		  {
		  }
	}

	/* Initialize pointers to RX and TX buffer */
	pRX_Buffer = &RX_Buffer;
	pTX_Buffer = &TX_Buffer;

    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&UART, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&UART, UART_IT_TXE);
}

/**
  * Put char to buffer
  *
*/
bool RingBuffer_PutChar(unsigned char c, ring_buffer *rbuf)
{
	bool result = E_ERROR;
	int i = (unsigned int)(rbuf->head + 1) % UART_BUFFER_SIZE;	// increment head and do modulo if is overflow

	if(i != rbuf->tail)
	{
		rbuf->buffer[rbuf->head] = c;
		rbuf->head = i;

		result =  E_OK;
    }

	return result;
}

/**
  * Read data in the RX_Buffer and increment tail
  *
*/
int RingBuffer_Read(void)
{
	// no characters to read
	if(pRX_Buffer->head == pRX_Buffer->tail)
	{
		return E_NOK;
	}
	else
	{
		unsigned char c = pRX_Buffer->buffer[pRX_Buffer->tail];
		pRX_Buffer->tail = (unsigned int)(pRX_Buffer->tail + 1) % UART_BUFFER_SIZE;
		return c;
	}
}

/**
  * Write data in the TX_Buffer and increment head
  *
*/
bool RingBuffer_Write(int c)
{
	bool result = E_ERROR;

	if (c>=0)
	{
		int i = (pTX_Buffer->head + 1) % UART_BUFFER_SIZE;

		// If the output buffer is full, there's nothing for it other than to
		// wait for the interrupt handler to empty it a bit
		// ???: return 0 here instead?
		while (i == pTX_Buffer->tail);

		pTX_Buffer->buffer[pTX_Buffer->head] = (uint8_t)c;
		pTX_Buffer->head = i;

		__HAL_UART_ENABLE_IT(&UART, UART_IT_TXE); // Enable UART transmission interrupt

		result = E_OK;
	}

	return result;
}

/**
  * Write string to the TX_Buffer
  *
*/

void RingBuffer_WriteString (const char *s)
{
	while(*s) RingBuffer_Write(*s++);
}

/**
  * Checks if the data is available to read in the RX_Buffer
  *
*/
int RingBuffer_isDataToRead(void)
{
	return (uint16_t)(UART_BUFFER_SIZE + pRX_Buffer->head - pRX_Buffer->tail) % UART_BUFFER_SIZE;
}

/**
  * Clear entire ring buffer, new data will start from position 0
  *
*/
void RingBuffer_Clear (void)
{
	memset(pRX_Buffer->buffer,'\0', UART_BUFFER_SIZE);
	pRX_Buffer->head = 0;
}

/**
  * See content in RX_Buffer -> it is read function without incrementing tail
  *
*/
int RingBuffer_SeeContent()
{
	if(pRX_Buffer->head == pRX_Buffer->tail)
	{
		return E_NOK;
	}
	else
	{
		return pRX_Buffer->buffer[pRX_Buffer->tail];
	}
}

/**
  * Wait for given response
  *
*/
int RingBuffer_WaitForGivenResponse (char *str)
{
	int str_idx = 0;
	int len = strlen(str);
	int result = E_NOK;

again:
	while(!RingBuffer_isDataToRead())
	{
		//wait for incoming data
	}

	while(RingBuffer_SeeContent() != str[str_idx])
	{
		pRX_Buffer->tail = (unsigned int)(pRX_Buffer->tail + 1) % UART_BUFFER_SIZE;
	}

	while(RingBuffer_SeeContent() == str [str_idx])
	{
		str_idx++;
		RingBuffer_Read();
		if (str_idx == len) return 1;
		while (!RingBuffer_isDataToRead()); // wait for incoming data
	}

	if (str_idx != len)
	{
		str_idx = 0;
		goto again; //RingBuffer_WaitForGivenResponse(str); // recursive search
	}

	if (str_idx == len) { result = E_OK; }

	return result;
}

/**
  * Find string in Ring Buffer
  *
*/
int RingBuffer_FindString (char *str, char *rbuf)
{
	int str_idx = 0;
	int rbuf_idx = 0;
	int result = E_NOK;

repeat:
	while (str[str_idx] != rbuf[rbuf_idx]) { rbuf_idx++; }

	if (str[str_idx] == rbuf[rbuf_idx])
	{
		while (str[str_idx] == rbuf[rbuf_idx])
		{
			str_idx++;
			rbuf_idx++;
		}
	}
	else
	{
		str_idx = 0;
		if (rbuf_idx >= strlen(rbuf)) return result;
		goto repeat;
	}

	if (str_idx == strlen(str)) { result = E_OK; }

	return result;
}

/**
  * ISR for the UART
  *
*/
void UART_IRQHandler ()
{
	uint32_t tmp_flag 	   = __HAL_UART_GET_FLAG(&UART, UART_FLAG_RXNE);
	uint32_t tmp_it_source = __HAL_UART_GET_IT_SOURCE(&UART, UART_IT_RXNE);

	if((tmp_flag != RESET) && (tmp_it_source != RESET))
	{
		unsigned char c = (USART1->RDR & (uint16_t)0x00FF);
		RingBuffer_PutChar (c, pRX_Buffer);

		__HAL_UART_CLEAR_PEFLAG(&UART);

		return;
	}

	if((__HAL_UART_GET_FLAG(&UART, UART_FLAG_TXE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&UART, UART_IT_TXE) != RESET))
	{
		if(TX_Buffer.head == TX_Buffer.tail)
		{
			__HAL_UART_DISABLE_IT(&UART, UART_IT_TXE);
		}
		else
		{
			unsigned char c = TX_Buffer.buffer[TX_Buffer.tail];
	    	TX_Buffer.tail = (TX_Buffer.tail + 1) % UART_BUFFER_SIZE;
			USART1->TDR = c;
		}

		return;
	}

	HAL_UART_IRQHandler(&UART);
}


int Copy_upto (char *string, char *buffertocopyinto)
{
	int str_idx = 0;
	int len = strlen(string);
	int idx = 0;

again:
	while (!RingBuffer_isDataToRead());
	while (RingBuffer_SeeContent() != string[str_idx])
		{
			buffertocopyinto[idx] = pRX_Buffer->buffer[pRX_Buffer->tail];
			pRX_Buffer->tail = (unsigned int)(pRX_Buffer->tail + 1) % UART_BUFFER_SIZE;
			idx++;
			while (!RingBuffer_isDataToRead());

		}
	while (RingBuffer_SeeContent() == string [str_idx])
	{
		str_idx++;
		buffertocopyinto[idx++] = RingBuffer_Read();
		if (str_idx == len) return 1;
		while (!RingBuffer_isDataToRead());
	}

	if (str_idx != len)
	{
		str_idx = 0;
		goto again;
	}

	if (str_idx == len) return 1;
	else return -1;
}

int Get_after (char *string, uint8_t numberofchars, char *buffertosave)
{

	while (RingBuffer_WaitForGivenResponse(string) != 1);
	for (int indx=0; indx<numberofchars; indx++)
	{
		while (!(RingBuffer_isDataToRead()));
		buffertosave[indx] = RingBuffer_Read();
	}
	return 1;
}


void GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto)
{
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int str_idx = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[str_idx] != buffertocopyfrom[indx]) indx++;
	if (startString[str_idx] == buffertocopyfrom[indx])
	{
		while (startString[str_idx] == buffertocopyfrom[indx])
		{
			str_idx++;
			indx++;
		}
	}

	if (str_idx == startStringLength) startposition = indx;
	else
	{
		str_idx =0;
		goto repeat1;
	}

	str_idx = 0;

repeat2:
	while (endString[str_idx] != buffertocopyfrom[indx]) indx++;
	if (endString[str_idx] == buffertocopyfrom[indx])
	{
		while (endString[str_idx] == buffertocopyfrom[indx])
		{
			str_idx++;
			indx++;
		}
	}

	if (str_idx == endStringLength) endposition = indx-endStringLength;
	else
	{
		str_idx =0;
		goto repeat2;
	}

	str_idx = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		buffertocopyinto[indx] = buffertocopyfrom[i];
		indx++;
	}
}
