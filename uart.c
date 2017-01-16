#define FOSC 1843200 //clock speed
#define _BAUD 9600
#define _UBRR ((F_CPU)/(BAUD*16UL)-1)

void main(void)
{
	uart_init(_UBRR);
}

void uart_init(unsigned int ubrr)
{
	/* set baud rate */
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;

	/* enable receiver and transmitter */
	UCSR0B = (1<<RXEN0) | (1<<TxEN0);

	/* set frame format: 8data, no parity, 1stop
	UPM00/UPM01 - parity bits
	USBS0 - stop bits (0 for 1bits, and 1 for 2bits)
	UCSZ00/UCSZ01 - data setting */
	UCSR0C |= ~(1<<USBS0) | (3<<UCSZ00);
}

void uart_transmit (unsigned char data)
{
	while(!(UCSRA & (1<<UDRE))); //wait while register is free
	UDR = data; 				 //load data in the register
}

unsigned char uart_receive(void)
{
	while(!(UCSRA) & (1<<RXC));	//wait whie data is being received
	return UDR;					//return 8-bit data
}