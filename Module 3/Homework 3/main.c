/* ********************************************
 * main.c
 *
 * SER486 - Assignment 3
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * This file implements the main entry point for
 * Homework Assignment 3.
 *
 * functions are :
 *    main() - program entry point; handles initialization
 *             and the main control loop
 */

#include "hw4lib.h"
#include "output.h"

/* ********************************************
 * main()
 *
 * Program entry point
 * Initializes hardware, outputs
 * identification and test hex values to the UART serial
 * console, then enters an infinite loop blinking the
 * morse code pattern for "ok" on Port B pin 1 LED.
 *
 * args :
 *   none
 *
 * returns :
 *   void
 *
 * changes :
 *   DDRB  - sets pin 1 as output
 *   PORTB - toggled by blink_led() via output.c
 *   UART transmit buffer - written during init & output calls
 */
int main()
{
    // initialization stage

    // set Port B pin 1 (PB1) as output for user LED
    DDRB |= (1 << PB1);

    // initialize the UART subsystem before any serial output
    uart_init();

    writestr("SER486 HW3 - Nathaniel Davis-Perez\n\r");

    // output the 8-bit hex value 0x0A + CR+LF
    writehex8(0x0A);
    writestr("\n\r");

    // output the 16-bit hex value 0xC0DE + CR+LF
    writehex16(0xC0DE);
    writestr("\n\r");

    // control loop

    // loop forever, blinking orse code "ok" (--- -.-) on LED
    while (1) {
        blink_led("--- -.- ");
    }

    return 0;
}