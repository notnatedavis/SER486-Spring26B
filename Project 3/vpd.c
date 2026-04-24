/* ********************************************
 * vpd.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * implements Vital Product Data (VPD) management with EEPROM storage
 * factory defaults include token "SER", placeholder name strings,
 * and a computed MAC address (based on first/last name)
 *
 * Functions:
 *   vpd_init() - load VPD from EEPROM; write defaults if invalid
 *   is_data_valid() - private: validate token "SER" and checksum
 *   write_defaults() - private: write factory defaults to EEPROM
 */

/* ----- Imports ----- */
#include "vpd.h"
#include "eeprom.h"
#include "util.h"
#include <string.h>

/* ----- Defined ----- */
#define VPD_BASE_ADDR 0x0000

/* factory defaults */
static const vpd_struct defaults = {
    .token = "SER",
    .model = "FirstName", // im assuming I put this instead of my actual FirstName 'Nathaniel'
    .manufacturer = "LastName",
    .serial_number = "AnyString",
    .manufacture_date = 0,
    .mac_address = { 'F' & 0xFE, 'i', 'r', 'L', 'a', 's' },  /* from "FirstName LastName" */
    .country_of_origin = "USA",
    .checksum = 0
};

/* Public VPD instance */
vpd_struct vpd;

/* Private functions */
static int is_data_valid(void);
static void write_defaults(void);

/* ********************************************
 * vpd_init - initialises VPD from EEPROM; writes defaults if invalid
 *   args: none
 *   returns: nothing
 *   behavior: reads VPD structure from EEPROM address 0x0000
 *             if token not "SER" or checksum invalid, writes factory
 *             defaults to EEPROM and re-reads
 */
void vpd_init(void) {
    /* read current VPD from EEPROM */
    eeprom_readbuf(VPD_BASE_ADDR, (unsigned char*)&vpd, sizeof(vpd_struct));
    if (!is_data_valid()) {
        write_defaults();
        /* re-read after writing defaults */
        eeprom_readbuf(VPD_BASE_ADDR, (unsigned char*)&vpd, sizeof(vpd_struct));
    }
}

/* ********************************************
 * is_data_valid - validates token and checksum of current VPD
 *   args: none
 *   returns: 1 if token is "SER" and checksum is valid, else 0
 *   behavior: compares first three characters of vpd.token to "SER",
 *             then calls is_checksum_valid() on the whole structure
 */
static int is_data_valid(void) {
    if (strncmp(vpd.token, "SER", 3) != 0)
        return 0;
    if (!is_checksum_valid((unsigned char*)&vpd, sizeof(vpd_struct)))
        return 0;
    return 1;
}

/* ********************************************
 * write_defaults - writes factory defaults to EEPROM (blocking)
 *   args: none
 *   returns: nothing
 *   behavior: copies the static defaults structure, updates its checksum,
 *             writes it to EEPROM at address 0x0000, and waits for write
 *             completion by polling eeprom_isbusy()
 */
static void write_defaults(void) {
    vpd_struct def_copy = defaults;
    update_checksum((unsigned char*)&def_copy, sizeof(vpd_struct));
    eeprom_writebuf(VPD_BASE_ADDR, (unsigned char*)&def_copy, sizeof(vpd_struct));
    /* Wait for write to complete before returning */
    while (eeprom_isbusy());
}