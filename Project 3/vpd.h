/* ********************************************
 * vpd.h
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * defines Vital Product Data (VPD) struct and public interface
 * VPD is stored in EEPROM at address 0x0000
 *
 * Functions:
 *   vpd_init() - load VPD from EEPROM, initialising with defaults if invalid
 */

/* ----- Defined ----- */
#ifndef VPD_H_INCLUDED
#define VPD_H_INCLUDED

/* VPD struct */
typedef struct {
    char token[4]; /* "SER" */
    char model[12]; /* First Name (max 11 chars + null) */
    char manufacturer[12]; /* Last Name (max 11 chars + null) */
    char serial_number[12]; /* any string */
    unsigned long manufacture_date;
    unsigned char mac_address[6];
    char country_of_origin[4]; /* "USA" */
    unsigned char checksum;
} vpd_struct;

/* Public VPD data (can be accessed directly) */
extern vpd_struct vpd;

/* ********************************************
 * vpd_init - initialises VPD from EEPROM; writes defaults if invalid
 *   args: none
 *   returns: nothing
 */
void vpd_init(void);

/* debug code */
void vpd_dump();

#endif /* VPD_H_INCLUDED */