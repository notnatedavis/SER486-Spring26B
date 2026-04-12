/* ********************************************
 * led.c
 *
 * SER486 - Project 1 Counter Timers
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements morse code blinking on the LED w/ state machine
 * timing is handled by delay instance 0
 *
 * Functions:
 *   led_set_blink() - set new Morse pattern and reset FSM
 *   led_update() - called in main loop; updates blink state
 *
 * led_init(), led_on(), led_off(), led_is_on() provided by lib_proj1.a
 */

#include "led.h"
#include "delay.h"

/* morse code timing : 1 unit = 100 ms (dot = 1 unit, dash = 3 units) */
#define UNIT_MS 100

static char* blink_msg   = 0; /* pointer to current blink pattern */
static unsigned char pos = 0; /* current position in blink_msg   */
static unsigned char waiting_off = 0; /* 0 = start new symbol, 1 = waiting OFF after ON */

/* ********************************************
 * led_set_blink - set a new morse code pattern and reset FSM
 *   args: msg - pointer to null‑terminated string of '.', '-', ' '
 *   returns: nothing
 *   behavior: stores pattern, resets position and state, turns LED off,
 *             and forces a short delay to start blinking immediately
 */
void led_set_blink(char* msg) {
    blink_msg = msg;
    pos = 0;
    waiting_off = 0;
    led_off();
    delay_set(0, 1); /* quick timeout to start FSM */
}

/* ********************************************
 * led_update - update the LED blink finite state machine
 *   args: none
 *   returns: nothing
 *   behavior: called from main loop. Uses delay(0) for timing.
 *             Implements dot (1 unit ON, 1 unit OFF), dash (3 units ON, 1 OFF),
 *             space (3 units OFF). Repeats pattern indefinitely
 */
void led_update(void) {
    char sym;

    if (!blink_msg) return;
    if (!delay_isdone(0)) return;

    /* current delay period finished */
    if (!waiting_off) {
        /* start a new symbol */
        sym = blink_msg[pos];
        if (sym == '\0') { /* end of string – wrap around */
            pos = 0;
            sym = blink_msg[0];
        }

        if (sym == '.') {
            led_on();
            delay_set(0, UNIT_MS); /* dot = 1 unit ON */
            waiting_off = 1;
        }
        else if (sym == '-') {
            led_on();
            delay_set(0, 3 * UNIT_MS); /* dash = 3 units ON */
            waiting_off = 1;
        }
        else if (sym == ' ') {
            /* space: OFF for 3 units, move to next symbol immediately */
            led_off();
            delay_set(0, 3 * UNIT_MS);
            pos++; /* consume the space */
            /* waiting_off remains 0 – next symbol after this OFF */
        }
    }
    else {
        /* Finished an ON period – do inter‑symbol OFF (1 unit) */
        led_off();
        delay_set(0, UNIT_MS);
        waiting_off = 0;
        pos++; /* move to next symbol */
    }
}