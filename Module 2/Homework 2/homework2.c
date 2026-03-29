/* *********************
 * homework2.c
 *
 * SER486 - Assignment 2
 * Spring '26
 * Written By:  Nathaniel Davis-Perez (student)
 *
 * this file implements functions associated with SER486
 * homework assignment 2.  The procedures implemented
 * provide ...
 *
 * functions are :
 *    delay()  - busy-wait loop for 2 second delay
 *    main()   - initializes hardware and runs control loop
 */

#include <avr/io.h>

/* *********************
 * delay
 *
 * busy-wait delay loop calibrated for ~2 seconds at 16 MHz
 * uses a nested loop to burn CPU cycles w/o sleeping
 *
 * args : none
 * returns :   void
 * changes :   nothing (pure delay)
 */
void delay(void)
{
    volatile unsigned int i;
    volatile unsigned int j;

    for (i = 0; i < 640; i++) {
        for (j = 0; j < 3200; j++) {
            /* busy wait */
        }
    }
}

/* *********************
 * main
 *
 * entry point 
 * initializes Port B pin 1 as an output and
 * enters a control loop that toggles pin every (delay_const) sec
 *
 * arguments: none
 * returns:   int (never returns — infinite loop)
 * changes:   DDRB bit 1, PORTB bit 1
 */
int main(void)
{
    // initialize : set Port B pin 1 as output (rest untouched)
    DDRB |= (1 << DDB1);

    // control loop : toggle Port B pin 1 every (delay) sec
    while (1) {
        PORTB ^= (1 << PORTB1);
        delay();
    }

    return 0;
}