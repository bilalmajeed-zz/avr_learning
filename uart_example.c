#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

void uart_init(uint32_t baudrate, uint8_t double_speed) {
    uint16_t ubrr = 0;
    if (double_speed) {
        UCSR0A = _BV(U2X0);  //Enable 2x speed
        ubrr = (F_CPU / (8UL * baudrate)) - 1;
    } else {
        ubrr = (F_CPU / (16UL * baudrate)) - 1;
    }
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr;

    UCSR0C &= ~(_BV(UMSEL01) | _BV(UMSEL00)); // enable asynchronous USART
    UCSR0C &= ~(_BV(UPM01) | _BV(UPM00)); // disable parity mode
    UCSR0C &= ~_BV(USBS0); // set 1-bit stop
    UCSR0C &= ~_BV(UCSZ02); 
    UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00); // set 8-bit data

    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   // Enable RX and TX 
}


uint8_t uart_getchar() {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void uart_read_line(uint8_t *value, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        value[i] = uart_getchar();
        if (value[i] == '\r') {
            value[i] = '\0';
            break;
        }
    }
}

void uart_putchar(const uint8_t data) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = data;
}


void uart_print(const char *value) {
    while (*value != '\0') {
        uart_putchar(*value++);
    }
}


int main(void) 
{
    uart_init(38400, 1);
    sei();

    uart_print("AT+NAME=BLUES\r\n");
    _delay_ms(1000);

    uart_print("AT+PSWD=7515\r\n");
    _delay_ms(1000);

    uart_print("AT+UART=38400,1,0\r\n");
    _delay_ms(1000);


    char data[11];
    for (;;) {
        memset(data, 0, 11);
        uart_read_line(data, 10);
        uart_print(data)
    }
}