/*
 * txSerial.c
 *
 *  Author: HHD
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
 
#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define BUFFER_SIZE  128
 
#include <util/delay.h>
 
char serialBuffer[BUFFER_SIZE];
uint8_t readPos = 0;
uint8_t writePos = 0;
 
void appendSerial(char c);
void serialWrite(char  c[]);

char getChar(void);
char peekChar(void);

int main(void)
{   
	UBRR0H = (BRC >> 8);
	UBRR0L =  BRC;
	 
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0) | (1 << RXEN0)  | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	DDRB = (1 << PORTB5);
	
	sei();
		 
	while(1)
	{
		char c = getChar();
		
		if(c == '1')
		{
			PORTB = (1 << PORTB5);
			serialWrite("LED On\n\r");
		}
		else if(c == '0')
		{
			PORTB = ~(1 << PORTB5);
			serialWrite("LED Off\n\r");
		}
	}
}
 
void appendSerial(char c)
{
	serialBuffer[writePos] = c;
	writePos++;
	 
	if(writePos >= BUFFER_SIZE)
	{
		writePos = 0;
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
	if(readPos != writePos)
	{
		ret = serialBuffer[readPos];
		readPos++;
		
		if(readPos >= BUFFER_SIZE)
		{
			readPos = 0;
		}
	}
	
	return ret;
}

char peekChar(void)
{
	char ret = '\0';
	if(readPos != writePos)
	{
		ret = serialBuffer[readPos];
	}
	
	return ret;
}

ISR(USART_TX_vect)
{
	if(readPos != writePos)
	{
		UDR0 = serialBuffer[readPos];
		readPos++;
		 
		if(readPos >= BUFFER_SIZE)
		{
			readPos = 0;
		}
	}
}

ISR(USART_RX_vect)
{
	serialBuffer[writePos] = UDR0;
	writePos++;
	
	if(writePos >= BUFFER_SIZE)
	{
		writePos = 0;	
	}
}