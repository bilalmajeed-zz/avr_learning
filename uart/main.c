#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define BUFFER_SIZE  128

// TX/RX buffers
char txBuffer[BUFFER_SIZE];
char rxBuffer[BUFFER_SIZE];

// R/W positions for TX/RX
uint8_t rx_readPos = 0;
uint8_t rx_writePos = 0;
uint8_t tx_readPos = 0;
uint8_t tx_writePos = 0;

void appendSerial(char c);
void serialWrite(char c[]);
char getChar(void);

ISR(USART_TX_vect) //trigger when transmitting
{
	//read char in UDR0
	if(tx_readPos != tx_writePos)
	{
		UDR0 = txBuffer[tx_readPos];
		tx_readPos++;
		 
		if(tx_readPos >= BUFFER_SIZE)
		{
			tx_readPos = 0;
		}
	}
}

ISR(USART_RX_vect) //trigger when receiving something
{
	rxBuffer[rx_writePos] = UDR0;
	rx_writePos++;
	
	if(rx_writePos >= BUFFER_SIZE)
	{
		rx_writePos = 0;	
	}
}

int main(void)
{
	//setup baudrate
	UBRR0H = (BRC >> 8);
	UBRR0L =  BRC;
	
	//enable TX/RX and interrupts
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0) | (1 << RXEN0)  | (1 << RXCIE0);
	//using 8 bit data transmission
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	//set PB5 as output
	DDRB = (1 << PORTB5);
	
	//enable external interrupts
	sei();

	while(1)
	{
		//get char from serial RX
		char c = getChar();
		
		if(c == '1')
		{
			PORTB = (1 << PORTB5); //set HIGH
			serialWrite("LED On\n\r");
		}
		else if(c == '0')
		{
			PORTB = ~(1 << PORTB5); //set LOW
			serialWrite("LED Off\n\r");
		}
	}
}

//append char c to the serialBuffer array
void appendSerial(char c)
{
	txBuffer[tx_writePos] = c;
	tx_writePos++;
	 
	if(tx_writePos >= BUFFER_SIZE)
	{
		tx_writePos = 0;
	}
}

//create the transmit buffer and then trigger INTR
void serialWrite(char c[])
{
	for(uint8_t i = 0; i < strlen(c); i++)
	{
		appendSerial(c[i]);
	}
	 
	if(UCSR0A & (1 << UDRE0)) //when ready to transmit
	{
		UDR0 = 0;
	}
}

//get first char in the receiving buffer
char getChar(void)
{
	char ret = '\0';
	if(rx_readPos != rx_writePos)
	{
		ret = rxBuffer[rx_readPos];
		rx_readPos++;
		
		if(rx_readPos >= BUFFER_SIZE)
		{
			rx_readPos = 0;
		}
	}
	return ret;
}