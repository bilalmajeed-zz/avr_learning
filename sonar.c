#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define BUFFER_SIZE  128

char tx_serialBuffer[BUFFER_SIZE];
uint8_t tx_readPos = 0;
uint8_t tx_writePos = 0;

volatile uint16_t duration = 0;
volatile unsigned char running = 0;
volatile unsigned char rising_edge = 1;

void appendSerial(char c);
void serialWrite(char c[]);
void pulseTrig(void);
void startTimer(void);

ISR(TIMER1_CAPT_vect)
{
	if(running) //only accept intr when sonar has started
	{
		if (rising_edge == 1)
		{
			TCCR1B& = ~(1<<ICES1); //change capture to falling edge
			rising_edge = 0;
			TCNT1 = 0; //clear counter to restart counting
		}
		else
		{
			duration = ICR1; //save timestap on falling edge
			TCCR1B |= (1 << ICES1); //change capture to rising edge
			rising_edge = 1;
			running = 0;
		}
		
	}
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

int main(void)
{
	/* UART SETUP */
	UBRR0H = (BRC >> 8); //baudrate
	UBRR0L =  BRC; 
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0); //enable TX and INTR
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	/* PB0 - echo pin, PB1 - trigger pin */
	DRRB = (1 << PORTB1) | ~(1 << PORTB0);

	startTimer();

	sei();

	while(running = 0)
	{
		_delay_ms(50);

		pulseTrig();	
		
		char val[2];
		uint16_t distance_cm = duration/58;
		itoa(distance_cm, val, 10);
		serialWrite(val);	
		serialWrite("\n\r");
	}
}

void startTimer()
{
	TCNT1 = 0; //clearing timer
	TIFR1 = (1 << ICF1); //clear pending interrupts
	TCCR1B = (1 << ICES1) | (1 << CS10); //input capture on rising edge, no prescaler
	TIMKS1 = (1 << ICIE1) | (1 << TOIE1); //enabling input capture
}

void pulseTrig()
{
	PORTB = 0x00;
	_delay_us(1);
	PORTB = 0x01;
	_delay_us(10);
	PORTB = 0x00;
}

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