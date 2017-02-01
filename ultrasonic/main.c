#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#define F_CPU 16000000
#define INSTR_PER_US 16    		// instructions per mircosecond
#define INSTR_PER_MS 16000		// instructions per millisecond
#define MAX_RESP_TIME_MS 200    // timeout - max time to wait for low voltage drop
#define TRIG_PIN PORTB0
#define ECHO_PIN PORTD3			//INT1

volatile long result_cm = 0;
volatile unsigned char echo_high = 0;
volatile unsigned char running = 0;
volatile uint32_t timer_counter = 0;

volatile uint8_t tx_writePos = 0;
volatile uint8_t tx_readPos = 0;
char txBuffer[BUFFER_SIZE];

void appendSerial(char c);
void serialWrite(char c[]);
void pulseTrigger(void);
void initTimer(void);
void initINT1(void);
void initUART(int baud);

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

// timer overflow interrupt, each time when timer value passes 255 value
ISR(TIMER0_OVF_vect)
{
	if (echo_high) //if the echo was already high (somewhere in the echo pulse)
	{
		timer_counter++; // count the number of overflows

		// dont wait too long for the sonar end response, stop if time for measuring the distance exceeded limits
		uint32_t ticks = timer_counter * 256 + TCNT0;
		uint32_t max_ticks = (uint32_t)MAX_RESP_TIME_MS * INSTR_PER_MS;

		if (ticks > max_ticks) // timeout if true
		{
			echo_high = 0;	// stop counting timer values
			running = 0; 	// ultrasound scan done
			result_cm = -1; // show that measurement failed with a timeout (could return max distance here if needed)
		}
	}
}

ISR(INT1_vect)
{
	if (running)
	{
		if (echo_high == 0) // echo wasnt on before so this is the start of the echo pulse
		{
			TCNT0 = 0; // reset timer counter
			timer_counter = 0;

			echo_high = 1;
		}
		else // echo was already on, therefore this is the falling edge of the echo pulse
		{
			// convert from time to distance
			// d(cm) = [ time_s * 340m/s ] / 2 = time_us/58
			result_cm = (timer_counter * 256 + TCNT0) / (58 * INSTR_PER_US) / 10;

			echo_high = 0;
			running = 0;
		}
	}
}

int main(void)
{
	//set UART, 8bit, 0 stop bits, 9600 baud rate
	initUART(9600);

	DDRB = (1 << TRIG_PIN) | (1 << PORTB5);
	PORTB = 0x00;

	//connect ECHO to INT1 and setup
	initINT1();

	//setup and start timer
	initTimer();

	sei();

	while(1)
	{
		if (running == 0)
		{
			_delay_ms(50);

			pulseTrigger();

			char value[4];
			sprintf(value,"%ld", result_cm);
			serialWrite(value);
			serialWrite("\n\r");
		}
	}
}

void initUART(int baud)
{
	int buadrate = ((F_CPU/16/baud) - 1);

	//set baudrate
	UBRR0H = (buadrate >> 8);
	UBRR0L =  buadrate;

	//enable TX and interrupts
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0);

	//using 8 bit data transmission
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// setup 8 bit timer
void initTimer()
{
	TCCR0B |= (1 << CS00); 	//No prescaling
	TCNT0 = 0;				//Reset timer
	TIMSK0 |= (1 << TOIE0); //enable interrupt for timer overflow
}

// turn on interrupts for INT1
void initINT1()
{
	EICRA |= (1 << ISC10); //Any logical chSange on INT1
	EIMSK |= (1 << INT1); //Enable INT1
}

void pulseTrigger()
{
	PORTB = 0x00; // clear to zero for 1 us
	_delay_us(1);
	PORTB = 0x01; // set high for 10us
	_delay_us(10);
	PORTB = 0x00; // clear

	running = 1;
}

//append char c to the serialBuffer array
void appendSerial(char c)
{
	txBuffer[tx_writePos] = c;
	tx_writePos++;

	if(tx_writePos >= BUFFER_SIZE)
		tx_writePos = 0;
}

//create the transmit buffer and then trigger INTR
void serialWrite(char c[])
{
	for(uint8_t i = 0; i < strlen(c); i++)
		appendSerial(c[i]);

	if(UCSR0A & (1 << UDRE0)) //when ready to transmit
		UDR0 = 0;
}