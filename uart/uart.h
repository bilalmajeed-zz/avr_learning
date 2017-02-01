#ifndef UART_H_   /* Include guard */
#define UART_H_

#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define BUFFER_SIZE  128

void uartInit();
void appendSerial(char c);
void serialWrite(char  c[]);
char getChar(void);

#endif // UART_H_