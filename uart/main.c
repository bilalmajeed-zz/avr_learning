#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../libs/uart_lib/uart.h"

#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)

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
