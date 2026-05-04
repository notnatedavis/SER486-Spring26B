/********************************************************
 * uart.c
 *
 * SER486 – UART driver for ATmega328P
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements uart_init and the uart_write* family.
 * Uses USART0 with 8N1 at 57600 baud (adjust as
 * needed for your simulation).
 *
 * Functions:
 *   uart_init()        – configure USART0
 *   uart_writechar()   – transmit a single byte
 *   uart_writestr()    – transmit a null‑terminated string
 *   uart_writehex8()   – transmit an 8‑bit hex number
 *   uart_writehex16()  – transmit a 16‑bit hex number
 *   uart_writedec32()  – transmit a signed 32‑bit decimal
 *   uart_writeip()     – transmit an IPv4 address
 ********************************************************/

#include "uart.h"
#include <avr/io.h>

#define BAUD 57600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

/********************************************************
 * uart_init – configure USART0 for 8N1.
 ********************************************************/
void uart_init(void)
{
    UBRR0H = (unsigned char)(UBRR_VAL >> 8);
    UBRR0L = (unsigned char)(UBRR_VAL);
    UCSR0B = (1 << TXEN0);               /* enable transmitter */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8 data bits, 1 stop bit */
}

/********************************************************
 * uart_writechar – transmit one character (blocking).
 ********************************************************/
void uart_writechar(char ch)
{
    while (!(UCSR0A & (1 << UDRE0)));   /* wait for empty buffer */
    UDR0 = ch;
}

/********************************************************
 * uart_writestr – transmit a null‑terminated string.
 ********************************************************/
void uart_writestr(const char *str)
{
    while (*str) {
        uart_writechar(*str++);
    }
}

/********************************************************
 * uart_writehex8 – output an 8‑bit value in hex.
 ********************************************************/
void uart_writehex8(unsigned char num)
{
    static const char hex[] = "0123456789ABCDEF";
    uart_writechar(hex[(num >> 4) & 0x0F]);
    uart_writechar(hex[num & 0x0F]);
}

/********************************************************
 * uart_writehex16 – output a 16‑bit value in hex.
 ********************************************************/
void uart_writehex16(unsigned int num)
{
    uart_writehex8((unsigned char)(num >> 8));
    uart_writehex8((unsigned char)(num & 0xFF));
}

/********************************************************
 * uart_writedec32 – output a signed 32‑bit decimal.
 ********************************************************/
void uart_writedec32(signed long num)
{
    char buf[12];
    char *p = buf + sizeof(buf) - 1;
    unsigned char neg = 0;

    *p = '\0';
    if (num < 0) {
        neg = 1;
        num = -num;
    }
    do {
        *--p = (char)('0' + (num % 10));
        num /= 10;
    } while (num > 0);
    if (neg) *--p = '-';
    uart_writestr(p);
}

/********************************************************
 * uart_writeip – output an IP address in dotted‑decimal.
 ********************************************************/
void uart_writeip(unsigned char *ip)
{
    uart_writedec32(ip[0]); uart_writechar('.');
    uart_writedec32(ip[1]); uart_writechar('.');
    uart_writedec32(ip[2]); uart_writechar('.');
    uart_writedec32(ip[3]);
}