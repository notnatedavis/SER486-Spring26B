/* ********************************************
 * eeprom.h
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Provides interface to buffered, interrupt-driven EEPROM operations.
 *
 * Functions:
 *   eeprom_readbuf() - blocking read from EEPROM
 *   eeprom_writebuf() - non‑blocking buffered write (interrupt driven)
 *   eeprom_isbusy() - check if a write is in progress
 */

/* ----- Defined ----- */
#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

/* ----- Imports ----- */
#include <avr/io.h>
#include <avr/interrupt.h>

/* ********************************************
 * eeprom_readbuf - blocking read of bytes from EEPROM
 *   args: addr - starting address in EEPROM
 *         buf - pointer to destination buffer
 *         size - number of bytes to read (0-255)
 *   returns: nothing
 */
void eeprom_readbuf(unsigned int addr, unsigned char* buf, unsigned char size);

/* ********************************************
 * eeprom_writebuf - start buffered write to EEPROM
 *   args: addr - starting address in EEPROM
 *         buf - pointer to source buffer
 *         size - number of bytes to write (0-255)
 *   returns: nothing
 *   note: caller must ensure eeprom_isbusy() returns 0
 */
void eeprom_writebuf(unsigned int addr, unsigned char* buf, unsigned char size);

/* ********************************************
 * eeprom_isbusy - check if a write is in progress
 *   args: none
 *   returns: 1 if busy, else 0
 */
unsigned char eeprom_isbusy(void);

#endif /* EEPROM_H_INCLUDED */