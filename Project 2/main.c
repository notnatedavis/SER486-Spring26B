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
 *    Within tyhe loop, updates LED FSM, log EEPROM writeback, config EEPROM
 *    writeback, and dumps EEPROM contents once when EEPROM is idle.
 */
#define sei() __asm__ volatile ("sei" ::: "memory")

#include "uart.h"
#include "delay.h"
#include "config.h"
#include "led.h"
#include "log.h"
#include "rtc.h"
#include "vpd.h"
#include "eeprom.h"
#include "util.h"
#include <avr/io.h>

void uart_set_baud(unsigned long baud) {
    unsigned int ubrr = (F_CPU / (16UL * baud)) - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B |= (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

int main(void) {
    uart_init();
    uart_set_baud(9600);
    delay_init();                /* now works because delay_init is public */

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

    /* Pre‑main‑loop tasks */
    config.use_static_ip = 1;
    config_set_modified();

    log_clear();
    log_add_record(0xaa);
    log_add_record(0xbb);
    log_add_record(0xcc);

    unsigned char dumped = 0;

    while (1) {
        led_update();          /* uses delay instance 0 */
        log_update();
        config_update();

        if ((!eeprom_isbusy()) && (!dumped)) {
            dump_eeprom(0, 0x100);
            dumped = 1;
        }
    }
    return 0;
}