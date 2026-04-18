/* ********************************************
 * main.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Entry point
 *    Initializes all subsystems, configures LED pattern &2 RTC time,
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

/* ----- Defined ----- */
// External references to VPD data (model, manufacturer, token)
extern char model[];
extern char manufacturer[];
extern char token[];

/* ********************************************
 * main - program entry point
 *   args: none
 *   returns: never returns (infinite loop)
 *   behavior:
 *     1. Initialize all subsystems in prescribed order
 *     2. Configure LED blink pattern and RTC date/time
 *     3. Print system identification and VPD strings to console
 *     4. Set static IP flag and mark config as modified
 *     5. Clear event log and add three test records
 *     6. Enter infinite loop: update LED, log, config; dump EEPROM once
 */
int main(void) {
    // 1. Initialization stage
    uart_init();        // Initialize UART console
    config_init();      // Initialize configuration (EEPROM backed)
    led_init();         // Initialize LED control
    log_init();         // Initialize event log (EEPROM backed)
    rtc_init();         // Initialize RTC (Timer1)
    vpd_init();         // Initialize Vital Product Data

    // Set LED blink pattern to "--- -.-" (OK in Morse code)
    led_set_pattern("--- -.-");

    // Set RTC date/time to January 1, 2019, 00:00:00
    rtc_set_datetime("01/01/2019 00:00:00");

    // Enable global interrupts after all peripherals are ready
    sei();

    // 2. Write system information to console
    uart_puts("SER 486 Project 2 – Nathaniel Davis-Perez\n\r");
    uart_puts(model);
    uart_puts("\n\r");
    uart_puts(manufacturer);
    uart_puts("\n\r");
    uart_puts(token);
    uart_puts("\n\r");

    // 3. Pre-main-loop tasks
    // Set static IP flag and mark config as modified
    config.use_static_ip = 1;
    config_set_modified();     // Indicate that config needs to be written to EEPROM

    // Clear event log and add three records
    log_clear();
    log_add(0xaa);
    log_add(0xbb);
    log_add(0xcc);

    // 4. Cyclic executive loop
    uint8_t dumped = 0;        // Flag to ensure EEPROM dump occurs only once

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