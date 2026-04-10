/* ********************************************
 * main.c
 *
 * SER486 - Project 1 Counter Timers
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * Entry point. Measures performance overhead of led_update(),
 * initializes hardware, sets up OK blinking and RTC, then runs
 * the main control loop displaying the RTC date every 500 ms.
 */

#include "delay.h"
#include "led.h"
#include "timer1.h"
#include "rtc.h"
#include "uart.h"

/* ********************************************
 * main - program entry point
 *   args: none
 *   returns: never returns (infinite loop)
 *   behavior:
 *     1. Measures performance of led_update() vs empty loop
 *     2. Initializes LED, RTC (Timer1)
 *     3. Sets blink pattern to "--- -.-" (OK)
 *     4. Sets RTC to 01/01/2019 00:00:00
 *     5. Prints startup message
 *     6. In main loop: calls led_update(), displays RTC date every 500 ms
 */
int main(void) {
    signed long c1, c2;

    /* Must initialize UART first for any output */
    uart_init();

    /* ----- Measure performance overhead of led_update() ----- */
    c1 = 0;
    delay_set(1, 10000);
    while (!delay_isdone(1)) { c1++; }

    c2 = 0;
    delay_set(1, 10000);
    while (!delay_isdone(1)) { led_update(); c2++; }

    uart_writedec32(c1);
    uart_writestr(" ");
    uart_writedec32(c2);
    uart_writestr("\r\n");

    /* ----- Project requirements ----- */
    led_init();                       /* initialize LED hardware (lib) */
    rtc_init();                       /* initializes Timer1 and RTC */
    led_set_blink("--- -.-");         /* OK in Morse code */
    rtc_set_by_datestr("01/01/2019 00:00:00");
    uart_writestr("SER 486 Project 1 - Nathaniel Davis-Perez\n\r");

    /* ----- Main loop ----- */
    delay_set(1, 500);                /* start 500 ms display timer */

    while (1) {
        led_update();                 /* update LED blink FSM */

        if (delay_isdone(1)) {
            /* Display current RTC date string without scrolling */
            uart_writestr(rtc_get_date_string());
            uart_writechar('\r');     /* carriage return only */
            delay_set(1, 500);        /* re‑arm for next 500 ms */
        }
    }

    /* never reached */
    return 0;
}