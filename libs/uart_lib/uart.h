#ifndef UART_H_   /* Include guard */
#define UART_H_
#define BUFFER_SIZE  128

void appendSerial(char c);
void serialWrite(char  c[]);

char getChar(void);

#endif // UART_H_