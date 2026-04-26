/********************************************************
 * wdt.c
 *
 * SER486 – Project 3 Integration
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements the watchdog timer driver for ATmega328P.
 * The WDT is initialised for a 2 s timeout (interrupt
 * then reset). The ISR logs the event and attempts to
 * flush the log/config caches before the final reset.
 *
 * Functions :
 *   wdt_init() – setup WDT with 2 s period
 *   wdt_reset() – clear the WDT counter
 *   wdt_force_restart() – disable WDT interrupt, loop
 *   ISR(WDT_vect) – event logging + cache flush
 */

/* ----- Imports ----- */
#include "wdt.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include "led.h"
#include "log.h"
#include "config.h"
#include "alarm.h" // not used here  included for completeness 

/* ----- Watchdog timer interrupt vector ----- */
/* Force the watchdog ISR to be compiled with size optimization (-Os).
 * This ensures the smallest possible code for the critical cache flush sequence. */
#pragma GCC push_options
#pragma GCC optimize ("-Os")
ISR(WDT_vect) {
    /* turn on LED to signal watchdog event */
    led_on();

    /* add the watchdog event to the log (marks it modified) */
    log_add_record(EVENT_WDT);

    /* flush caches with interrupts disabled */
    unsigned char i;
    /* up to 16 calls may be needed to flush the entire log */
    for (i = 0; i < 16; i++) {
        log_update_noisr();
    }
    config_update_noisr();   /* first call flushes config if modified */

    /* The watchdog will issue a system reset after the next timeout.
       The RESET is automatic – no need to stay in the ISR */
}
#pragma GCC pop_options

/* ----- Public functions ----- */

/********************************************************
 * wdt_init – configure watchdog for 2 s interrupt+reset
 *   args: none
 *   returns: nothing
 *   behavior: sets WDTCSR to enable WDE, WDIE, and
 *             2 s prescaler (WDP2:0 = 0b111).
 */
void wdt_init(void) {
    cli();   /* disable global interrupts while configuring */
    WDTCSR = (1 << WDCE) | (1 << WDE);          /* enable change */
    WDTCSR = (1 << WDE) | (1 << WDIE)           /* interrupt + reset */
           | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);  /* 2 s */
    sei();   /* re-enable interrupts */
}

/********************************************************
 * wdt_reset – feed the watchdog
 *   args: none
 *   returns: nothing
 *   behavior: executes the WDR instruction
 */
void wdt_reset(void) {
    __builtin_avr_wdr();
}

/********************************************************
 * wdt_force_restart – force a system reset via WDT
 *   args: none
 *   returns: nothing (never returns)
 *   behavior: disables WDIE, then spins until reset
 */
void wdt_force_restart(void) {
    cli();
    WDTCSR = (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE)           /* reset enabled, no interrupt */
           | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);
    while (1) { /* wait for watchdog timeout */ }
}