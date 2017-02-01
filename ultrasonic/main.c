#define F_CPU   16000000

#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define BUFFER_SIZE  128

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <stdlib.h>
#define INSTR_PER_US 16                   // instructions per microsecond (depends on MCU clock, 12MHz current)
#define INSTR_PER_MS 16000                // instructions per millisecond (depends on MCU clock, 12MHz current)
#define MAX_RESP_TIME_MS 200      // timeout - max time to wait for low voltage drop (higher value increases measuring distance at the price of slower sampling)
#define DELAY_BETWEEN_TESTS_MS 500 // echo cancelling time between sampling
volatile long result = 0;
volatile unsigned char up = 0;
volatile unsigned char running = 0;
volatile uint32_t timerCounter = 0;
char txBuffer[BUFFER_SIZE];
uint8_t tx_readPos = 0;
uint8_t tx_writePos = 0;
void appendSerial(char c);
void serialWrite(char c[]);

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
	if (up) {       // voltage rise was detected previously
		timerCounter++; // count the number of overflows
		// dont wait too long for the sonar end response, stop if time for measuring the distance exceeded limits
		uint32_t ticks = timerCounter * 256 + TCNT0;
		uint32_t max_ticks = (uint32_t)MAX_RESP_TIME_MS * INSTR_PER_MS; // this could be replaced with a value instead of multiplying
		if (ticks > max_ticks) {
			// timeout
			up = 0;          // stop counting timer values
			running = 0; // ultrasound scan done
			result = -1; // show that measurement failed with a timeout (could return max distance here if needed)
		}
	}
}

ISR(INT1_vect)
{
	if (running) { //accept interrupts only when sonar was started
		if (up == 0) // voltage rise, start time measurement
		{ 
			up = 1;
			timerCounter = 0;
			TCNT0 = 0; // reset timer counter
		} 
		else 
		{
			// voltage drop, stop time measurement
			up = 0;
			// convert from time to distance(millimeters): d = [ time_s * 340m/s ] / 2 = time_us/58
			result = (timerCounter * 256 + TCNT0) / 58;
			running = 0;
		}
	}
}

void sonar() 
{
	PORTB = 0x00; // clear to zero for 1 us
	_delay_us(1);
	PORTB = 0x01; // set high for 10us
	running = 1;  // sonar launched
	_delay_us(10);
	PORTB = 0x00; // clear
}

int main(void)
{
	UBRR0H = (BRC >> 8);
	UBRR0L =  BRC;
	
	//enable TX/RX and interrupts
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0) | (1 << RXEN0)  | (1 << RXCIE0);
	//using 8 bit data transmission
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	// ------------------- ultrasonic init code --------------------
	DDRB = (1 << PORTB0) | (1 << PORTB5); // PB0 output - connected to Trig
	PORTB = 0x00; // clear
	// turn on interrupts for INT1, connect Echo to INT1
	
	EICRA |= (1 << ISC10); //Any logical chSange on INT1
	EIMSK |= (1 << INT1); //Enable INT1
	// setup 8 bit timer & enable interrupts, timer increments to 255 and interrupts on overflow
	TCCR0B |= (1 << CS00); //No prescaling
	TCNT0 = 0;			//Reset timer
	TIMSK0 |= (1 << TOIE0); //Timer overflow interrupt enable
	
	sei(); // enable all(global) interrupts
	
	while(1){  /* main program loop */
		
		if (running == 0) { // launch only when next iteration can happen
			
			// create a delay between tests, to compensate for old echoes
			_delay_ms(DELAY_BETWEEN_TESTS_MS);
			sonar(); // launch measurement!
			if(result != 0)
			{
				char value[4];
				sprintf(value,"%ld", result);
				serialWrite(value);
				serialWrite("\n\r");
			}
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