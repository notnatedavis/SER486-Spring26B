/********************************************************
 * util.c
 *
 * SER486 – Project 3 Integration
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements utility helpers and the command‑line
 * packet parser for the IOT endpoint
 *
 * Functions:
 *   update_checksum() – fill in checksum byte
 *   is_checksum_valid() – verify checksum
 *   dump_eeprom() – print EEPROM hex dump
 *   update_tcrit_hi() – validate & set hi crit
 *   update_twarn_hi() – validate & set hi warn
 *   update_tcrit_lo() – validate & set lo crit
 *   update_twarn_lo() – validate & set lo warn
 *   parse_and_send_response() – interpret command packet
 */

/* ----- Imports ----- */
#include "util.h"
#include "process_packet.h"
#include "config.h"
#include "vpd.h"
#include "log.h"
#include "rtc.h"
#include "uart.h" /* optional, for local debug */
#include "uartsocket.h"
#include "eeprom.h"
#include "tempfsm.h"
#include <string.h>  /* for strtok, strcmp, etc. */

/* access the global temperature from main.c */
extern int current_temperature;

/* ----- Utility functions (util.h) ----- */

/********************************************************
 * update_checksum – compute checksum byte
 *   args: data array, size in bytes
 *   returns: nothing
 *   behavior: sets the last byte so that sum of all
 *             bytes modulo 256 equals zero.
 */
void update_checksum(unsigned char *data, unsigned int dsize) {
    unsigned int sum = 0;
    unsigned int i;
    for (i = 0; i < dsize - 1; i++) {
        sum += data[i];
    }
    data[dsize - 1] = (unsigned char)(-sum);
}

/********************************************************
 * is_checksum_valid – test checksum
 *   args: data array, size
 *   returns: 1 if sum == 0, else 0
 */
int is_checksum_valid(unsigned char *data, unsigned int dsize) {
    unsigned int sum = 0;
    unsigned int i;
    for (i = 0; i < dsize; i++) {
        sum += data[i];
    }
    return (sum == 0) ? 1 : 0;
}

/********************************************************
 * dump_eeprom – hex dump a region of EEPROM
 *   args: start address, number of bytes
 *   returns: nothing
 *   behavior: prints 16‑byte lines with address, hex,
 *             and ASCII representation.
 */
void dump_eeprom(unsigned int start_address, unsigned int numbytes) {
    unsigned int addr;
    unsigned char byte;
    for (addr = start_address; addr < start_address + numbytes; addr += 16) {
        uart_writehex16(addr);
        uart_writestr(": ");
        unsigned int i;
        for (i = 0; i < 16; i++) {
            if (addr + i >= start_address + numbytes) break;
            eeprom_readbuf(addr + i, &byte, 1);
            uart_writehex8(byte);
            uart_writechar(' ');
        }
        uart_writestr("\r\n");
    }
}

/* ----- Packet Processing (process_packet.h) ----- */

/********************************************************
 * update_tcrit_hi – set high critical alarm limit
 *   args: value (int)
 *   returns: 0 on success, 1 on error
 *   behavior: validates value > twarn_hi && <= 0x3FF
 */
int update_tcrit_hi(int value) {
    if (value <= config.hi_warn || value > 0x3FF) return 1;
    config.hi_alarm = value;
    config_set_modified();
    return 0;
}

/********************************************************
 * update_twarn_hi – set high warning limit
 *   args: value
 *   returns: 0 on success, 1 on error
 *   behavior: validates twarn_lo < value < tcrit_hi
 */
int update_twarn_hi(int value) {
    if (value <= config.lo_warn || value >= config.hi_alarm) return 1;
    config.hi_warn = value;
    config_set_modified();
    return 0;
}

/********************************************************
 * update_tcrit_lo – set low critical alarm limit
 *   args: value
 *   returns: 0 on success, 1 on error
 *   behavior: validates value < twarn_lo
 */
int update_tcrit_lo(int value) {
    if (value >= config.lo_warn) return 1;
    config.lo_alarm = value;
    config_set_modified();
    return 0;
}

/********************************************************
 * update_twarn_lo – set low warning limit
 *   args: value
 *   returns: 0 on success, 1 on error
 *   behavior: validates tcrit_lo < value < twarn_hi
 */
int update_twarn_lo(int value) {
    if (value <= config.lo_alarm || value >= config.hi_warn) return 1;
    config.lo_warn = value;
    config_set_modified();
    return 0;
}

/********************************************************
 * parse_and_send_response – read and handle packet
 *   args: none
 *   returns: nothing
 *   behavior: reads a line from uartsocket, dispatches
 *             GET / PUT / DELETE commands, and sends
 *             a JSON response or status message.
 */
void parse_and_send_response(void) {
    char buf[128];
    /* read a complete line (delimited by \r\n) */
    if (!uartsocket_readpacket(buf, "\r\n")) {
        uartsocket_writestr("ERROR: packet read failed\r\n");
        return;
    }

    /* Tokenise the request line */
    char *method = strtok(buf, " ");
    char *path   = strtok(NULL, " ");
    /* ignore HTTP version if present */

    if (!method || !path) {
        uartsocket_writestr("ERROR: malformed request\r\n");
        return;
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/device") == 0) {
        
        /* Build JSON response */
        uartsocket_writestr("{\r\n");

        /* VPD block */
        uartsocket_writestr("  \"vpd\":{\r\n");
        uartsocket_writestr("    \"model\":\"");
        uartsocket_writestr(vpd.model);
        uartsocket_writestr("\",\r\n");

        uartsocket_writestr("    \"manufacturer\":\"");
        uartsocket_writestr(vpd.manufacturer);
        uartsocket_writestr("\",\r\n");

        uartsocket_writestr("    \"serial_number\":\"");
        uartsocket_writestr(vpd.serial_number);
        uartsocket_writestr("\",\r\n");

        uartsocket_writestr("    \"manufacture_date\":\"");
        uartsocket_writestr(rtc_num2datestr(vpd.manufacture_date));
        uartsocket_writestr("\",\r\n");

        uartsocket_writestr("    \"mac_address\":\"44:4F:55:53:41:4E\",\r\n");
        uartsocket_writestr("    \"country_code\":\"");
        uartsocket_writestr(vpd.country_of_origin);
        uartsocket_writestr("\"\r\n  },\r\n");

        /* Config limits */
        uartsocket_writestr("  \"tcrit_hi\":");
        uartsocket_writedec32(config.hi_alarm);
        uartsocket_writestr(",\r\n");

        uartsocket_writestr("  \"twarn_hi\":");
        uartsocket_writedec32(config.hi_warn);
        uartsocket_writestr(",\r\n");

        uartsocket_writestr("  \"tcrit_lo\":");
        uartsocket_writedec32(config.lo_alarm);
        uartsocket_writestr(",\r\n");

        uartsocket_writestr("  \"twarn_lo\":");
        uartsocket_writedec32(config.lo_warn);
        uartsocket_writestr(",\r\n");

        /* Current temperature and state */
        uartsocket_writestr("  \"temperature\":");
        uartsocket_writedec32(current_temperature);
        uartsocket_writestr(",\r\n");

        uartsocket_writestr("  \"state\":\"");
        uartsocket_writestr(tempfsm_get_state());
        uartsocket_writestr("\",\r\n");

        /* Log array */
        uartsocket_writestr("  \"log\":[\r\n");
        unsigned char num = log_get_num_entries();
        unsigned char i;
        for (i = 0; i < num; i++) {
            unsigned long time;
            unsigned char evt;
            if (log_get_record(i, &time, &evt)) {
                uartsocket_writestr("    {\"timestamp\":\"");
                uartsocket_writestr(rtc_num2datestr(time));
                uartsocket_writestr("\",\"eventnum\":");
                uartsocket_writedec32(evt);
                uartsocket_writestr("}");
                if (i < num - 1) uartsocket_writestr(",");
                uartsocket_writestr("\r\n");
            }
        }
        uartsocket_writestr("  ]\r\n}\r\n");
    }
    else if (strcmp(method, "PUT") == 0) {
        /* Handle PUT /device/config?param=value */
        char *param_name = strtok(path, "?=");
        char *param_value = strtok(NULL, "=");
        if (!param_name || !param_value) {
            uartsocket_writestr("ERROR: invalid PUT syntax\r\n");
            return;
        }
        int val = atoi(param_value);
        int err = 1;
        if (strcmp(param_name, "/device/config") == 0) {
            /* Actually the param is after the '?' */
            param_name = strtok(NULL, ""); // rest of line
        }
        /* Now determine which config variable */
        if (strncmp(param_name, "tcrit_hi", 8) == 0) {
            err = update_tcrit_hi(val);
        } else if (strncmp(param_name, "twarn_hi", 8) == 0) {
            err = update_twarn_hi(val);
        } else if (strncmp(param_name, "tcrit_lo", 8) == 0) {
            err = update_tcrit_lo(val);
        } else if (strncmp(param_name, "twarn_lo", 8) == 0) {
            err = update_twarn_lo(val);
        } else if (strcmp(param_name, "/device?reset=\"true\"") == 0) {
            uartsocket_writestr("Resetting...\r\n");
            wdt_force_restart();   /* never returns */
        } else {
            uartsocket_writestr("ERROR: unknown parameter\r\n");
            return;
        }
        if (err) {
            uartsocket_writestr("ERROR: value out of range\r\n");
        } else {
            uartsocket_writestr("OK\r\n");
        }
    }
    else if (strcmp(method, "DELETE") == 0 && strcmp(path, "/device/log") == 0) {
        log_clear();
        uartsocket_writestr("Log cleared\r\n");
    }
    else {
        uartsocket_writestr("ERROR: unsupported command\r\n");
    }
}