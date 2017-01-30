#include "uart.h"  /* Include the header (not strictly necessary here) */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char tx_serialBuffer[BUFFER_SIZE];
char rx_serialBuffer[BUFFER_SIZE];

uint8_t tx_readPos = 0;
uint8_t tx_writePos = 0;
uint8_t rx_readPos = 0;
uint8_t rx_writePos = 0;

void appendSerial(char c)
{
	tx_serialBuffer[tx_writePos] = c;
	tx_writePos++;
	 
	if(tx_writePos >= BUFFER_SIZE)
	{
		tx_writePos = 0;
	}
}
 
void serialWrite(char c[])
{
	for(uint8_t i = 0; i < strlen(c); i++)
	{
		appendSerial(c[i]);
	}
	 
	if(UCSR0A & (1 << UDRE0))
	{
		UDR0 = 0;
	}
}
 
char getChar(void)
{
	char ret = '\0';
	if(rx_readPos != rx_writePos)
	{
		ret = rx_serialBuffer[rx_readPos];
		rx_readPos++;
		
		if(rx_readPos >= BUFFER_SIZE)
		{
			rx_readPos = 0;
		}
	}
	
	return ret;
}

ISR(USART_TX_vect)
{
	if(tx_readPos != tx_writePos)
	{
		UDR0 = tx_serialBuffer[tx_readPos];
		tx_readPos++;
		 
		if(tx_readPos >= BUFFER_SIZE)
		{
			tx_readPos = 0;
		}
	}
}

ISR(USART_RX_vect)
{
	rx_serialBuffer[rx_writePos] = UDR0;
	rx_writePos++;
	
	if(rx_writePos >= BUFFER_SIZE)
	{
		rx_writePos = 0;	
	}
}