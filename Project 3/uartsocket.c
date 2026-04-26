/********************************************************
 * uartsocket.c
 *
 * SER486 – Project 3 (standalone)
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Polling UART‑based socket.  Ctrl‑C initiates connection,
 * characters are buffered until a line is complete.
 ********************************************************/
#include "uartsocket.h"
#include "uart.h"
#include <avr/io.h>
#include <string.h>

static unsigned char connected = 0;
static char rx_buf[128];
static unsigned char rx_idx = 0;

void uartsocket_init(void) {
    connected = 0;
    rx_idx = 0;
}

void uartsocket_writechar(char ch)   { uart_writechar(ch); }
void uartsocket_writestr(const char *str)  { uart_writestr(str); }
void uartsocket_writehex8(unsigned char num)  { uart_writehex8(num); }
void uartsocket_writehex16(unsigned int num)  { uart_writehex16(num); }
void uartsocket_writedec32(signed long num)   { uart_writedec32(num); }

unsigned char uartsocket_is_connected(void) {
    return connected;
}

unsigned char uartsocket_is_packet_available(void) {
    /* packet available if we have a complete line (ends with '\n') */
    return (rx_idx > 0 && rx_buf[rx_idx-1] == '\n');
}

unsigned char uartsocket_readpacket(char *buf, char *delimiters) {
    if (!uartsocket_is_packet_available()) return 0;
    strncpy(buf, rx_buf, rx_idx);
    buf[rx_idx] = '\0';
    rx_idx = 0;            /* reset buffer after copy */
    return 1;
}

/* must be called regularly from main() to process incoming characters */
void uartsocket_poll(void) {
    while (UCSR0A & (1 << RXC0)) {
        char ch = UDR0;
        if (ch == 0x03) {          /* Ctrl‑C */
            connected = 1;
            uart_writestr("Connection Established\r\n");
        } else if (connected) {
            if (rx_idx < sizeof(rx_buf)-1) {
                rx_buf[rx_idx++] = ch;
                /* line ending: \r or \n – we keep them */
                if (ch == '\n') {
                    /* packet complete, will be picked up by main loop */
                }
            }
        }
    }
}