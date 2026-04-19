/* ********************************************
 * eeprom.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements buffered, interrupt-driven EEPROM writes and blocking reads.
 * Uses EEPROM ready interrupt to send one byte at a time.
 *
 * Functions:
 *   eeprom_readbuf() - blocking read of 'size' bytes into buffer
 *   eeprom_writebuf() - start buffered write, enable interrupt, start first byte
 *   eeprom_isbusy() - returns write_busy flag
 *   __vector_22() - ISR that writes subsequent bytes
 */

/* ----- Imports ----- */
#include "eeprom.h"

/* ----- Defines ----- */
#define BUFSIZE 256

/* ----- Internal state ----- */
static unsigned char writebuf[BUFSIZE];
static unsigned char bufidx;
static unsigned char writesize;
static unsigned int  writeaddr;
static volatile unsigned char write_busy = 0;

/* ********************************************
 * eeprom_readbuf - blocking read of bytes from EEPROM
 *   args: addr - starting address in EEPROM
 *         buf - pointer to destination buffer
 *         size - number of bytes to read (0-255)
 *   returns: nothing
 *   behavior: for each byte, waits for any ongoing write to finish,
 *             sets address registers, starts read, and stores result.
 *             Address increments automatically.
 */
void eeprom_readbuf(unsigned int addr, unsigned char* buf, unsigned char size) {
    for (unsigned char i = 0; i < size; i++) {
        /* Wait for any ongoing write to finish */
        while (EECR & (1 << EEPE));
        
        /* Set address */
        EEARL = (unsigned char)(addr & 0xFF);
        EEARH = (unsigned char)((addr >> 8) & 0xFF);
        
        /* Read data */
        EECR |= (1 << EERE);
        buf[i] = EEDR;
        addr++;
    }
}

/* ********************************************
 * eeprom_writebuf - start buffered write to EEPROM
 *   args: addr - starting address in EEPROM
 *         buf - pointer to source buffer
 *         size - number of bytes to write (0-255)
 *   returns: nothing
 *   behavior: stores address, sets write_busy=1, copies data into internal
 *             write buffer, sets writesize, enables EEPROM ready interrupt,
 *             then starts the first byte write manually (ISR handles the rest).
 *             Caller must ensure eeprom_isbusy() returns 0 before calling.
 */
void eeprom_writebuf(unsigned int addr, unsigned char* buf, unsigned char size) {
    /* Cannot start a new write if already busy */
    if (write_busy) return;
    
    /* Prepare for the write sequence */
    writeaddr = addr;
    write_busy = 1;
    bufidx = 0;
    writesize = size;
    
    /* Copy data into internal buffer */
    for (unsigned char i = 0; i < size; i++) {
        writebuf[i] = buf[i];
    }
    
    /* Enable EEPROM ready interrupt */
    EECR |= (1 << EERIE);
    
    /* Start the first write manually (the ISR will continue with the rest) */
    while (EECR & (1 << EEPE));            /* wait for previous write (none) */
    EEARL = (unsigned char)(writeaddr & 0xFF);
    EEARH = (unsigned char)((writeaddr >> 8) & 0xFF);
    EEDR = writebuf[bufidx];
    EECR |= (1 << EEMPE);                  /* master enable */
    EECR |= (1 << EEPE);                   /* start write */
    /* Note: we do NOT increment bufidx or writeaddr here – the ISR will handle
       the first byte's completion and advance to the next byte. */
}

/* ********************************************
 * eeprom_isbusy - check if a write is in progress
 *   args: none
 *   returns: 1 if write_busy is 1, else 0
 *   behavior: returns current state of write_busy flag.
 */
unsigned char eeprom_isbusy(void) {
    return write_busy;
}

/* ********************************************
 * __vector_22 - EEPROM ready interrupt service routine
 *   args: none (ISR)
 *   returns: nothing
 *   behavior: if more bytes remain, writes next byte from writebuf to EEPROM,
 *             increments address and index. After last byte, disables EEPROM
 *             ready interrupt and clears write_busy flag.
 */
ISR(EE_READY_vect) {
    /* Move to next byte (the one we just finished writing) */
    bufidx++;
    writeaddr++;
    
    if (bufidx < writesize) {
        /* Write the next byte */
        while (EECR & (1 << EEPE));
        EEARL = (unsigned char)(writeaddr & 0xFF);
        EEARH = (unsigned char)((writeaddr >> 8) & 0xFF);
        EEDR = writebuf[bufidx];
        EECR |= (1 << EEMPE);
        EECR |= (1 << EEPE);
    } else {
        /* No more bytes: disable interrupts and clear busy flag */
        EECR &= ~(1 << EERIE);
        write_busy = 0;
    }
}