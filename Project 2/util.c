/* ********************************************
 * util.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements checksum calculation and EEPROM dumping utility.
 *
 * Functions:
 *   update_checksum() - set last byte so sum of all bytes becomes zero
 *   is_checksum_valid() - return 1 if sum of all bytes is zero (mod 256)
 *   dump_eeprom() - display EEPROM contents in hex+ASCII (blocking)
 */

/* ----- Imports ----- */
#include "util.h"
#include "uart.h"
#include <avr/io.h>

/* ********************************************
 * update_checksum - set last byte so total sum becomes zero
 *   args: data - pointer to structure (including checksum byte)
 *         dsize - total size of structure in bytes
 *   returns: nothing
 *   behavior: sums all bytes except the last, then sets the last byte
 *             to the two's complement of that sum (mod 256), making the
 *             total sum of all bytes equal to 0 (mod 256).
 */
void update_checksum(unsigned char *data, unsigned int dsize) {
    unsigned int sum = 0;
    /* Sum all bytes except the last one */
    for (unsigned int i = 0; i < dsize - 1; i++) {
        sum += data[i];
    }
    /* Last byte = -sum (mod 256) */
    data[dsize - 1] = (unsigned char)(-sum);
}

/* ********************************************
 * is_checksum_valid - verify that total sum of bytes is zero
 *   args: data - pointer to structure (including checksum byte)
 *         dsize - total size of structure in bytes
 *   returns: 1 if sum of all bytes (mod 256) equals 0, else 0
 *   behavior: sums all bytes of the structure and returns true if the
 *             low byte of the sum is zero.
 */
int is_checksum_valid(unsigned char *data, unsigned int dsize) {
    unsigned int sum = 0;
    for (unsigned int i = 0; i < dsize; i++) {
        sum += data[i];
    }
    return (sum & 0xFF) == 0;
}

/* ********************************************
 * dump_eeprom - display EEPROM contents in hex and ASCII
 *   args: start_address - first address to display
 *         numbytes - number of bytes to dump
 *   returns: nothing
 *   behavior: reads EEPROM in 16‑byte lines, prints address,
 *             hexadecimal bytes (two hex digits each), and ASCII
 *             representation ('.' for non‑printable). Uses UART.
 */
void dump_eeprom(unsigned int start_address, unsigned int numbytes) {
    unsigned char buf[16];   /* 16 bytes per line */
    unsigned int addr = start_address;
    unsigned int remaining = numbytes;

    while (remaining > 0) {
        unsigned int bytes_this_line = (remaining < 16) ? remaining : 16;
        /* Read one line from EEPROM */
        for (unsigned int i = 0; i < bytes_this_line; i++) {
            /* Inline EEPROM read (polling) – reusing logic */
            while (EECR & (1 << EEPE));
            EEARL = (unsigned char)(addr & 0xFF);
            EEARH = (unsigned char)((addr >> 8) & 0xFF);
            EECR |= (1 << EERE);
            buf[i] = EEDR;
            addr++;
        }
        /* Print address */
        uart_writehex16((unsigned int)addr - bytes_this_line);
        uart_writechar(':');
        uart_writechar(' ');
        /* Print hex bytes */
        for (unsigned int i = 0; i < bytes_this_line; i++) {
            uart_writehex8(buf[i]);
            uart_writechar(' ');
        }
        /* Pad with spaces for alignment */
        for (unsigned int i = bytes_this_line; i < 16; i++) {
            uart_writestr("   ");
        }
        uart_writechar(' ');
        uart_writechar(' ');
        /* Print ASCII representation */
        for (unsigned int i = 0; i < bytes_this_line; i++) {
            unsigned char c = buf[i];
            if (c >= 0x20 && c <= 0x7E)
                uart_writechar(c);
            else
                uart_writechar('.');
        }
        uart_writestr("\r\n");
        remaining -= bytes_this_line;
    }
}