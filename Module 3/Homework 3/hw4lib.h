/*
    uart.h

    this header file includes function declarations for the SER486
    Homework assignment 2
 */

/*
 * Homework 3 - hw4lib.h Fix
 * compile errors for (DDRB, PORTB, PB1, F_CPU) should resolve undeclared identifier errors
 */
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ----- */

#ifndef _UART_H
#define _UART_H


/* initialize uart subsystem - this function must be called once before
   the serial port can be used.
*/
void uart_init(void);

/*
   This function will write a single character to the atmega uart.
   uart_init() must be called once before this function will work.
*/
void uart_writechar(char ch);

#endif  /* _UART_H */

