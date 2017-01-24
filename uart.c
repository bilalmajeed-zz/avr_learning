#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define BAUD 9600
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

void main(void)
{
	unsigned char a;
	char buffer[10];

	uart_init(BAUDRATE);
	while(1)
	{
		a = uart_receive();                 // save the received data in a variable
        itoa(a,buffer,10);                  // convert numerals into string
        _delay_ms(100);
        printf("DID SHIT\n");
	}
}

void uart_init(unsigned int ubrr)
{
	/* set baud rate */
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;

	/* enable receiver and transmitter */
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	/* set frame format: 8data, no parity, 1stop
	UPM00/UPM01 - parity bits
	USBS0 - stop bits (0 for 1bits, and 1 for 2bits)
	UCSZ00/UCSZ01 - data setting */
	UCSR0C |= ~(1<<USBS0) | (3<<UCSZ00);
}

void uart_transmit (unsigned char data)
{
	while(!(UCSRA & (1<<UDRE0))); //wait while register is free
	UDR = data; 				 //load data in the register
}

unsigned char uart_receive(void)
{
	while(!(UCSRA) & (1<<RXC));	//wait whie data is being received
	return UDR;					//return 8-bit data
}