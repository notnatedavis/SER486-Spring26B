/********************************************************
 * alarm.c
 *
 * SER486 – Project 3 (standalone)
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements alarm_init and alarm_send using UART.
 ********************************************************/
#include "alarm.h"
#include "uart.h"

void alarm_init(void) {
    /* nothing to do */
}

void alarm_send(unsigned eventnum) {
    uart_writestr("ALARM: ");
    uart_writehex8(eventnum);
    uart_writestr("\r\n");
}