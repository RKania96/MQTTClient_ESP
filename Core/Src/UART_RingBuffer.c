/*
 * UART_RingBuffer.c
 *
 *  Created on: 01.08.2021
 *      Author: RK
 *
 */

#include "UART_RingBuffer.h"
#include "GetTime.h"
#include <string.h>

#define E_OK	1
#define E_ERROR -1
#define E_NOK   0

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

	GetTime_Init();
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
  * Write string to the TX_Buffer with given length
  *
*/
void RingBuffer_WriteLenghtString(const unsigned char *s, int dataLength)
{
	int i = 0;
	while(i < dataLength)
	{
		RingBuffer_Write(s[i]);
		i++;
	}
}

/**
  * Checks if the data is available to read in the RX_Buffer
  *
*/
int RingBuffer_isDataToRead(void)
{
	int rc;
	if(rc=GetTime_timeoutIsExpired()) { return -1; }
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
	if(GetTime_timeoutIsExpired()) { return E_ERROR; }

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
	int rc;

begin:
	str_idx = 0;
	len = strlen(str);

	GetTime_timeoutBegin();

again:
	while(!(rc=RingBuffer_isDataToRead()))
	{
		//wait for incoming data
		if(rc == E_ERROR) {return E_ERROR; }
	}

	while(1)
	{
		rc = RingBuffer_SeeContent();

		if(rc == E_ERROR) {return E_ERROR; }

		if(rc != str[str_idx])
		{
			pRX_Buffer->tail = (unsigned int)(pRX_Buffer->tail + 1) % UART_BUFFER_SIZE;
		}
		else {break;}
	}

	while(1)
	{
		rc = RingBuffer_SeeContent();

		if(rc == E_ERROR) {return E_ERROR; }

		if(rc == str[str_idx])
		{
			str_idx++;
			RingBuffer_Read();
			if (str_idx == len) return 1;
			while (!(rc=RingBuffer_isDataToRead()))
			{
				if(rc == E_ERROR) {return E_ERROR; }// wait for incoming data
			}
		}
		else {break;}


	}

	if (str_idx != len)
	{
		str_idx = 0;
		goto again; //RingBuffer_WaitForGivenResponse(str); // recursive search
	}

	if (str_idx == len) { return E_OK; }

	goto begin;
}

///**
//  * ISR for the UART
//  *
//*/
//int RingBuffer_Receive(char *string, uint8_t numberofchars, char *buffertosave, int *result)
//{
//	while(RingBuffer_WaitForGivenResponse(string) != 1);
//	for (int indx=0; indx<numberofchars; indx++)
//	{
//		while (!(RingBuffer_isDataToRead()));
//		buffertosave[indx] = RingBuffer_Read();
//		*result = indx+1;
//	}
//	return true;
//}

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
