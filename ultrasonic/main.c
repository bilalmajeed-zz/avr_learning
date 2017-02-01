/*
 * ultrasonic.c
 *
 * Created: 1/29/2017 1:52:59 PM
 * Author : bilal
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../libs/uart_lib/uart.h"

unsigned char working;
unsigned char rising_edge;
uint16_t timer_value;
int distance_cm;
uint8_t error;

ISR(INT0_vect)
{
	if(working == 1) //check if echo is high, start timer
	{
		if(rising_edge == 0)
		{
			rising_edge = 1;
			TCNT0 = 0;
			timer_value = 0;
		}
		else //check if echo turned low, calculate distance
		{
			rising_edge = 0;
			distance_cm = (timer_value*256 + TCNT0)/58;
			working = 0;
		}
	}
}

ISR(TIMER0_OVF_vect)
{
	if(rising_edge == 1) //check if there was echo
	{
		timer_value++;
		//check if isnt out of range
		if (timer_value > 91)
		{
			working = 0;
			rising_edge = 0;
			error = 1;
		}
	}	
}

int main(void)
{
	/* init external intr */
	EICRA |= (1 << ISC00); //any logical change on int1
	EIMSK |= (1 << INT0); //enable int0
	
	/* init timer0 */
	TCCR0B |= (1 << CS00); //no prescaling
	TCNT0 = 0; //reset counter
	TIMSK0 |= (1 <<	TOIE0); //enable timer overflow interrupt
	
	
	/* init serial com */
	uartInit();
	sei();
	serialWrite("testing\n\r");
	
	DDRD |= (1 << PORTD3);
	DDRD &= ~(1 << PORTD2);
		
    while (1) 
    {
		char value[2];
		itoa(distance_cm, value, 10);
		serialWrite(value);
		serialWrite("\n\r");
		
		if(error == 1)
		{
			serialWrite("ERROR");
		}
		
		//restarting for another conversation
		if(working == 0)
		{
			_delay_ms(50); //reset hc-sr04
			PORTD &= !(1 << PORTD4);
			_delay_us(1);
			PORTD |= (1 << PORTD4);
			_delay_us(10);
			PORTD &= ~(1 << PORTD4);
			working = 1;
			error = 0;
		}
    }
}

