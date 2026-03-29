/********************************************************
 * output.c
 *
 * SER486 Assignment 4
 * Spring 2018
 * Written By:  Doug Sandy (instructor)
 * Modified By:
 *
 * this file implements functions associated with SER486
 * homework assignment 4.  The procedures implemented
 * provide led and console output support for debugging
 * of future SER 486 assignments.
 *
 * functions are:
 *    writestr(char *)  - a function to write an ascii
 *                        null-terminated string to the
 *                        avr atmega 328P uart port.
 *                        (instructor provided)
 *
 *    writehex8(unsigned char) - a function to write the
 *                        hexadecimal representation of
 *                        an 8-bit unsigned integer to
 *                        avr atmega 328P uart port.
 *
 *    writehex16(unsigned int) - a function to write the
 *                        hexadecimal representation of
 *                        a 16-bit unsigned integer to
 *                        avr atmega 328P uart port.
 *
 *    blink_led(char *) - a function to send a specific
 *                        blink pattern to the development
 *                        board's user-programmable LED.
 *
 *    delay(unsigned int) - delay code execution for
 *                        the specified amount of milliseconds.
 *                        (instructor provided)
 */

 #include "hw4lib.h"

 /* macro definitions used by delay() */
 #define MSEC_PER_SEC     1000
 #define CLOCKS_PER_LOOP  16
 #define LOOPS_PER_MSEC   ((F_CPU/MSEC_PER_SEC)/CLOCKS_PER_LOOP)

/**********************************
 * delay(unsigned int msec)
 *
 * This code delays execution for for
 * the specified amount of milliseconds.
 *
 * arguments:
 *   msec - the amount of milliseconds to delay
 *
 * returns:
 *   nothing
 *
 * changes:
 *   nothing
 *
 * NOTE: this is not ideal coding practice since the
 * amount of time spent in this delay may change with
 * future hardware changes.  Ideally a timer should be
 * used instead of a set of for loops.  Timers will be
 * taught later in the semester.
 */
void delay(unsigned int msec)
{
    volatile unsigned i,j ;  /* volatile prevents loops from being optimized away */

    /* outer loop, loops for specified number of milliseconds */
    for (i=0; i<msec; i++) {
        /* inner loop, loops for 1 millisecond delay */
        for (j=0;j<LOOPS_PER_MSEC;j++) {}
    }
}


/**********************************
 * writestr(char * str)
 *
 * This code writes a null-terminated string
 * to the UART.
 *
 * arguments:
 *   str - a pointer to the beginning of the
 *         string to be printed.
 *
 * returns:
 *   nothing
 *
 * changes:
 *   the state of the uart transmit buffer will
 *   be changed by this function.
 *
 * NOTE: uart_init() should be called this function
 *   is invoked for the first time or unpredictable
 *   UART behavior may result.
 */
void writestr(char * str)
{
    unsigned int i;

    /* loop for each character in the string */
    for (i=0; str[i]!=0;i++) {
        /* output the character to the UART */
        uart_writechar(str[i]);
    }
}

/************************************************************
* STUDENT CODE BELOW THIS POINT
*************************************************************/

/* *********************************
 * output.c
 *
 * SER486 - Assignment 3
 * Spring '26
 * Written By:  Nathaniel Davis-Perez (student)
 *
 * this file implements functions associated with SER486
 * homework assignment 3.  The procedures implemented
 * provide ...
 *
 * functions are :
 *    writehex8(unsigned char) - writes hexadecimal ASCII
 *                               representation of 8-bit
 *                               unsigned int to UART
 *
 *    writehex16(unsigned int) - writes hexadecimal ASCII
 *                               representation of 16-bit
 *                               unsigned int to UART
 *
 *    blink_led(char *)        - blinks a morse code pattern on
 *                               the development board's user LED
 */

/* Student-provided function comments go here */
/* *********************************
 * writehex8(unsigned char num)
 *
 * outputs two-digit hexadecimal ASCII representation
 * of 8-bit unsigned value to UART
 *
 * args :
 *   num - 8-bit unsigned value to display in hex
 *
 * returns :
 *   nothing
 *
 * changes:
 *   state of the UART transmit buffer
 */
void writehex8(unsigned char num)
{
    /* student-provided implementation code goes here */
    
    unsigned char piece;  // holds one 4-bit piece
    unsigned char digit;   // ASCII character to send

    // output the high piece (upper 4 bits) 
    piece = (num >> 4) & 0x0F;
    // convert piece to ASCII hex character
    digit = (piece < 10) ? ('0' + piece) : ('A' + piece - 10);
    uart_writechar(digit);

    // output the low piece (lower 4 bits)
    piece = num & 0x0F;
    // convert piece to ASCII hex character
    digit = (piece < 10) ? ('0' + piece) : ('A' + piece - 10);
    uart_writechar(digit);
}

/* Student-provided function comments go here */
/* *********************************
 * writehex16(unsigned int num)
 *
 * outputs four-digit hexadecimal ASCII representation
 * of 16-bit unsigned value to UART
 *
 * args :
 *   num - 16-bit unsigned value to display in hex
 *
 * returns :
 *   nothing
 *
 * changes :
 *   state of the UART transmit buffer
 */
void writehex16(unsigned int num)
{
    /* student-provided implementation code goes here */
    
    // output the high byte 15-8 first
    writehex8((unsigned char)(num >> 8));

    // output the low byte 7-0 second
    writehex8((unsigned char)(num & 0xFF));
}

/* Student-provided function comments go here */
/* *********************************
 * blink_led(char *msg)
 *
 * blinks the user-programmable LED on Port B pin 1 according
 * to morse code pattern string
 * each character in the string is evaluated as :
 *   '-' LED on for 750ms, then off for 100ms (dash)
 *   '.' LED on for 250ms, then off for 100ms (dot)
 *   ' ' LED off for 1000ms (gap between char)
 * All other characters are ignored
 *
 * args :
 *   msg - pointer to a null-terminated morse code pattern string
 *
 * returns :
 *   nothing
 *
 * changes :
 *   PORTB - toggles PB1 to control the LED
 */
void blink_led(char *msg)
{
    /* student-provided implementation code goes here */

    // index for stepping through msg string
    unsigned int i;  

    // iterate over each character in the morse code string
    for (i=0; msg[i] != 0; i++) {

        if (msg[i] == '-') {
            // (dash) : turn LED on for 750ms, then off for 100ms 
            writestr("LED ON  - DASH 750ms\n\r");
            PORTB |=  (1 << PB1); // LED on
            delay(750);
            PORTB &= ~(1 << PB1); // LED off
            writestr("LED OFF - 100ms\n\r");
            delay(100);

        } else if (msg[i] == '.') {
            // (dot) : turn LED on for 250ms, then off for 100ms
            writestr("LED ON  - DOT 250ms\n\r");
            PORTB |=  (1 << PB1); // LED on 
            delay(250);
            PORTB &= ~(1 << PB1); // LED off
            writestr("LED OFF - 100ms\n\r");
            delay(100);

        } else if (msg[i] == ' ') {
            // (space) : LED stays off for 1000ms (gap between char)
            writestr("LED OFF - SPACE 1000ms\n\r");
            PORTB &= ~(1 << PB1); // ensure LED is off
            delay(1000);

        }

        // all other char ignored
    }
}
