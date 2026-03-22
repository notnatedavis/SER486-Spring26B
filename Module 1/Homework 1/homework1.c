/*
 * Nathaniel Davis-Perez
 * March 22 , 2026
 * SER 486 - Spring '26 B
 * Homework 1 
 */

/* ----- Imports ----- */

#include "uart.h"

/* ----- Helper Functions ----- */

/*
 * writestr - writes a null-terminated string to the serial console
 *
 *   str - pointer to the string to be written
 */
void writestr(const char *str) {
    while (*str != '\0') {
        uart_writechar(*str);
        str++;
    }
}

/*
 * bits_to_str - returns a string constant representing the given number of bits
 *
 *   bits - the number of bits
 */
const char *bits_to_str(unsigned int bits) {
    switch (bits) {
        case 8:  return "8";
        case 16: return "16";
        case 32: return "32";
        default: return "?";
    }
}

/* ----- Main Entry ----- */

int main(void) {
    // 1. initialize the serial port 
    uart_init();

    // 2. output the required strings using writestr
    writestr("Hello World from Atmega328\r\n");
    writestr("SER486 -- Homework Assignment 1\r\n");
    writestr("Nathaniel Davis-Perez\r\n");

    /* 3. output the sizes of (fundamental) data types
     *   use sizeof() to get size in bytes,  x8 = bits
     *   convert bit count to string via bits_to_str()
     */
    writestr("char size = ");
    writestr(bits_to_str(sizeof(char) * 8));
    writestr(" bits\r\n");

    writestr("int size = ");
    writestr(bits_to_str(sizeof(int) * 8));
    writestr(" bits\r\n");

    writestr("long size = ");
    writestr(bits_to_str(sizeof(long) * 8));
    writestr(" bits\r\n");

    // 4. loop forever
    while (1)
        ;

    return 0; // unreachable
}