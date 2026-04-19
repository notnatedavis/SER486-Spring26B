/* ********************************************
 * lib_proj2.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Provides: UART, Timer1, RTC, Log, and basic LED control.
 * Delay and LED blink FSM are now in separate files (delay.c, led.c).
 */

/* ----- Imports ----- */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "uart.h"
#include "timer1.h"
#include "rtc.h"
#include "log.h"
#include "eeprom.h"
#include "util.h"
#include "led.h"      /* for led_init, led_on, led_off, led_is_on */

/* ============================== UART (polling) ============================== */
void uart_init(void) {
    unsigned int ubrr = (16000000UL / (16UL * 9600)) - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_writechar(char ch) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = ch;
}

void uart_writestr(char *str) {
    while (*str) uart_writechar(*str++);
}

void uart_writehex8(unsigned char num) {
    const char hex[] = "0123456789ABCDEF";
    uart_writechar(hex[(num >> 4) & 0x0F]);
    uart_writechar(hex[num & 0x0F]);
}

void uart_writehex16(unsigned int num) {
    uart_writehex8((unsigned char)(num >> 8));
    uart_writehex8((unsigned char)num);
}

void uart_writedec32(signed long num) {
    if (num < 0) {
        uart_writechar('-');
        num = -num;
    }
    char buf[12];
    char *p = buf + 11;
    *p = '\0';
    do {
        *--p = '0' + (num % 10);
        num /= 10;
    } while (num);
    uart_writestr(p);
}

/* ============================== Timer1 (1 second tick for RTC) ============================== */
static volatile unsigned long rtc_seconds = 0;

ISR(TIMER1_COMPA_vect) {
    rtc_seconds++;
}

void timer1_init(void) {
    TCCR1B = (1 << WGM12) | (1 << CS12);   /* CTC, prescaler 256 */
    OCR1A = 62500;                          /* 1 second @ 16 MHz */
    TIMSK1 |= (1 << OCIE1A);
    TCNT1 = 0;
}

unsigned long timer1_get(void) {
    return rtc_seconds;
}

void timer1_clear(void) {
    rtc_seconds = 0;
}

/* ============================== RTC ============================== */
void rtc_init(void) {
    timer1_init();
    sei();          /* global interrupts already enabled in main, but safe */
}

char *rtc_get_date_string(void) {
    static char buffer[20];
    unsigned long secs = rtc_seconds;
    unsigned long days = secs / 86400;
    unsigned long hours = (secs % 86400) / 3600;
    unsigned long minutes = (secs % 3600) / 60;
    unsigned long seconds = secs % 60;
    sprintf(buffer, "%3lu %02lu:%02lu:%02lu", days, hours, minutes, seconds);
    return buffer;
}

void rtc_set_by_datestr(char *datestr) {
    (void)datestr;
    rtc_seconds = 0;
}

unsigned long rtc_get_date(void) {
    return rtc_seconds;
}

char *rtc_num2datestr(unsigned long num) {
    static char buffer[20];
    unsigned long days = num / 86400;
    unsigned long hours = (num % 86400) / 3600;
    unsigned long minutes = (num % 3600) / 60;
    unsigned long seconds = num % 60;
    sprintf(buffer, "%3lu %02lu:%02lu:%02lu", days, hours, minutes, seconds);
    return buffer;
}

/* ============================== Log (circular buffer in EEPROM) ============================== */
#define LOG_EEPROM_ADDR 0x0200
#define LOG_MAX_ENTRIES 32

typedef struct {
    unsigned long timestamp;
    unsigned char event;
} log_entry_t;

static log_entry_t log_ram[LOG_MAX_ENTRIES];
static unsigned char log_head = 0;
static unsigned char log_tail = 0;
static unsigned char log_count = 0;
static unsigned char log_modified = 0;

void log_init(void) {
    eeprom_readbuf(LOG_EEPROM_ADDR, (unsigned char*)log_ram, sizeof(log_ram));
    log_head = 0;
    log_tail = 0;
    log_count = 0;
    log_modified = 0;
}

void log_update(void) {
    if (!log_modified) return;
    if (eeprom_isbusy()) return;
    eeprom_writebuf(LOG_EEPROM_ADDR, (unsigned char*)log_ram, sizeof(log_ram));
    log_modified = 0;
}

void log_clear(void) {
    memset(log_ram, 0, sizeof(log_ram));
    log_head = 0;
    log_tail = 0;
    log_count = 0;
    log_modified = 1;
}

void log_add_record(unsigned char eventnum) {
    if (log_count == LOG_MAX_ENTRIES) {
        log_tail = (log_tail + 1) % LOG_MAX_ENTRIES;
        log_count--;
    }
    log_entry_t *entry = &log_ram[log_head];
    entry->timestamp = rtc_get_date();
    entry->event = eventnum;
    log_head = (log_head + 1) % LOG_MAX_ENTRIES;
    log_count++;
    log_modified = 1;
}

int log_get_record(unsigned long index, unsigned long *time, unsigned char *eventnum) {
    if (index >= log_count) return 0;
    unsigned char pos = (log_tail + index) % LOG_MAX_ENTRIES;
    *time = log_ram[pos].timestamp;
    *eventnum = log_ram[pos].event;
    return 1;
}

unsigned char log_get_num_entries(void) {
    return log_count;
}

/* ============================== Basic LED control (hardware) ============================== */
#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_PIN  (1 << 5)   /* PB5 = Arduino pin 13 */

void led_init(void) {
    LED_DDR |= LED_PIN;
    led_off();
}

void led_on(void) {
    LED_PORT |= LED_PIN;
    uart_writechar('*');   // visual indication: LED on
}

void led_off(void) {
    LED_PORT &= ~LED_PIN;
    uart_writechar('.');   // visual indication: LED off
}

int led_is_on(void) {
    return (LED_PORT & LED_PIN) != 0;
}