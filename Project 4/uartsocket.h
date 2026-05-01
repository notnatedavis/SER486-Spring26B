/********************************************************
 * uartsocket.h
 *
 * this file provides function declarations for the
 * SER486 UART‑based socket (used for stand‑alone testing).
 *
 * Author:   Nathaniel Davis-Perez
 * Date:     4/18/2018
 * Revision: 1.0
 *
 * Copyright(C) 2018, Arizona State University
 * All rights reserved
 *
 */
#ifndef UARTSOCKET_H_INCLUDED
#define UARTSOCKET_H_INCLUDED

void          uartsocket_init(void);
unsigned char uartsocket_is_connected(void);
unsigned char uartsocket_is_packet_available(void);
unsigned char uartsocket_readpacket(char *buf, char *delimiters);
void          uartsocket_poll(void);

void uartsocket_writechar(char ch);
void uartsocket_writestr(const char *str);
void uartsocket_writehex8(unsigned char num);
void uartsocket_writehex16(unsigned int num);
void uartsocket_writedec32(signed long num);

#endif // UARTSOCKET_H_INCLUDED