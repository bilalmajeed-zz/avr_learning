#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart_lib/uart.h"

int main(void)
{   
	uartInit();
	
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