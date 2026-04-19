/* ********************************************
 * main.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Entry point
 *    Initializes all subsystems, configures LED pattern and RTC time,
 *    prints system information to console, sets static IP flag, clears
 *    and populates event log, then enters cyclic executive loop.
 *    Within the loop, updates LED FSM, log EEPROM writeback, config EEPROM
 *    writeback, and dumps EEPROM contents once when EEPROM is idle.
 */
#define sei() __asm__ volatile ("sei" ::: "memory") // SUPER IMPORTANT
// ^ needed to enable global interrupts from main

#include "uart.h"
#include "config.h"
#include "led.h"
#include "log.h"
#include "rtc.h"
#include "vpd.h"
#include "eeprom.h"
#include "util.h"
#include <avr/io.h>   // for UBRR0* registers

/* ----- Defined ----- */

// necessary helper function to override existing baud causing issue in uart output
void uart_set_baud(unsigned long baud) {
    unsigned int ubrr = (F_CPU / (16UL * baud)) - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B |= (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

/* ********************************************
 * main - program entry point
 *   args: none
 *   returns: never returns (infinite loop)
 *   behavior:
 *     1. Initialize uart, config, led, log, rtc, vpd in that order
 *     2. Set LED blink pattern to "--- -.-" (OK in Morse)
 *     3. Set RTC date/time to "01/01/2019 00:00:00"
 *     4. Enable global interrupts
 *     5. Print identification and VPD strings to console
 *     6. Set static IP flag and mark config as modified
 *     7. Clear event log and add three test records
 *     8. Enter infinite loop: update LED, log, config; dump EEPROM once
 */
int main(void) {
    // 1. Initialization stage (order matters)
    uart_init();        // Initialize UART console
    uart_set_baud(9600); // REQUIRED TO RUN
    
    // 2. Enable global interrupts after all peripherals are ready
    sei();
    
    uart_writestr("0: Start\r\n");

    uart_writestr("1: config_init\r\n");
    config_init();
    uart_writestr("2: led_init\r\n");
    led_init();
    uart_writestr("3: log_init\r\n");
    log_init();
    uart_writestr("4: rtc_init\r\n");
    rtc_init();
    uart_writestr("5: vpd_init\r\n");
    vpd_init();

    uart_writestr("6: LED blink set\r\n");
    led_set_blink("--- -.-");
    rtc_set_by_datestr("01/01/2019 00:00:00");

    uart_writestr("7: Printing VPD\r\n");
    uart_writestr("SER 486 Project 2 – Nathaniel Davis-Perez\r\n");
    uart_writestr(vpd.model);
    uart_writestr("\r\n");
    uart_writestr(vpd.manufacturer);
    uart_writestr("\r\n");
    uart_writestr(vpd.token);
    uart_writestr("\r\n");
    
    // 3. Configure LED blink pattern and RTC date/time
    led_set_blink("--- -.-");                 // OK in Morse code
    rtc_set_by_datestr("01/01/2019 00:00:00");

    // 4. Write system information to console
    uart_writestr("SER 486 Project 2 – Nathaniel Davis-Perez\r\n");
    uart_writestr(vpd.model);
    uart_writestr("\r\n");
    uart_writestr(vpd.manufacturer);
    uart_writestr("\r\n");
    uart_writestr(vpd.token);
    uart_writestr("\r\n");

    // 5. Pre-main-loop tasks
    config.use_static_ip = 1;
    config_set_modified();          // Indicate that config needs EEPROM writeback

    log_clear();
    log_add_record(0xaa);
    log_add_record(0xbb);
    log_add_record(0xcc);

    // 6. Cyclic executive loop
    unsigned char dumped = 0;       // Flag to ensure EEPROM dump occurs only once

    while (1) {
        // Update LED blink finite state machine
        led_update();

        // Update log – writes back any modified entries if EEPROM not busy
        log_update();

        // Update config – writes back config data if modified and EEPROM not busy
        config_update();

        // Dump EEPROM contents once when EEPROM is idle
        if ((!eeprom_isbusy()) && (!dumped)) {
            dump_eeprom(0, 0x100);
            dumped = 1;
        }
    }

    return 0;   // Never reached
}