/********************************************************
 * timer1.h
 *
 * Declarations for the hardware timer1 interface
 * (used by rtc and log subsystems).
 *
 * Author:   Doug Sandy
 * Date:     3/1/2018
 * Revision: 1.0
 *
 * Copyright(C) 2018, Arizona State University
 * All rights reserved
 *
 */
#ifndef TIMER1_H_INCLUDED
#define TIMER1_H_INCLUDED

/* initialize timer1 to generate a 1-second interrupt */
void timer1_init(void);

/* return the current 32-bit tick count (seconds) */
unsigned long timer1_get(void);

/* reset the tick count to 0 */
void timer1_clear(void);

#endif // TIMER1_H_INCLUDED