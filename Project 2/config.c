/* ********************************************
 * config.c
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements configuration data management with EEPROM storage.
 * Provides factory defaults, checksum validation, and lazy writeback.
 *
 * Functions:
 *   config_init() - loads config from EEPROM; writes defaults if invalid
 *   config_update() - writes modified config to EEPROM if not busy
 *   config_set_modified() - marks config as modified for later update
 *   is_data_valid() - private: validates token and checksum
 *   write_defaults() - private: writes factory defaults to EEPROM
 */

/* ----- Imports ----- */
#include "config.h"
#include "eeprom.h"
#include "util.h"
#include <string.h>

/* ----- Defined ----- */
#define CONFIG_BASE_ADDR 0x0040
/* factory defaults */
static const config_struct defaults = {
    .token = "ASU",
    .hi_alarm = 0x03FF,
    .hi_warn  = 0x03FE,
    .lo_alarm = 0x0000,
    .lo_warn  = 0x0001,
    .use_static_ip = 0,
    .static_ip = {192, 168, 1, 100},
    .checksum = 0
};

/* public config instance */
config_struct config;
static unsigned char modified = 0;

/* private functions */
static int is_data_valid(void);
static void write_defaults(void);

/* ********************************************
 * config_init - initialises config from EEPROM; writes defaults if invalid
 *   args: none
 *   returns: nothing
 *   behavior: waits until EEPROM is not busy, then reads config structure
 *             from address 0x0040. If token is not "ASU" or checksum fails,
 *             writes factory defaults to EEPROM and re-reads. Clears modified flag.
 */
void config_init(void) {
    /* Wait until EEPROM is not busy */
    while (eeprom_isbusy());
    eeprom_readbuf(CONFIG_BASE_ADDR, (unsigned char*)&config, sizeof(config_struct));
    if (!is_data_valid()) {
        write_defaults();
        eeprom_readbuf(CONFIG_BASE_ADDR, (unsigned char*)&config, sizeof(config_struct));
        modified = 0;
    }
}

/* ********************************************
 * config_update - writes modified config to EEPROM if not busy
 *   args: none
 *   returns: nothing
 *   behavior: if EEPROM is busy or config not modified, does nothing.
 *             Otherwise updates checksum, writes entire config to EEPROM
 *             write buffer, and clears modified flag.
 */
void config_update(void) {
    if (eeprom_isbusy())
        return;
    if (!modified)
        return;
    /* Update checksum before writing */
    update_checksum((unsigned char*)&config, sizeof(config_struct));
    eeprom_writebuf(CONFIG_BASE_ADDR, (unsigned char*)&config, sizeof(config_struct));
    modified = 0;
}

/* ********************************************
 * config_set_modified - marks config as modified for later update
 *   args: none
 *   returns: nothing
 *   behavior: sets the internal modified flag to 1. Should be called
 *             whenever any field of the config structure is changed.
 */
void config_set_modified(void) {
    modified = 1;
}

/* ********************************************
 * is_data_valid - validates token and checksum of current config
 *   args: none
 *   returns: 1 if token is "ASU" and checksum is valid, else 0
 *   behavior: compares first three characters of config.token to "ASU",
 *             then calls is_checksum_valid() on the whole structure.
 */
static int is_data_valid(void) {
    if (strncmp(config.token, "ASU", 3) != 0)
        return 0;
    if (!is_checksum_valid((unsigned char*)&config, sizeof(config_struct)))
        return 0;
    return 1;
}

/* ********************************************
 * write_defaults - writes factory defaults to EEPROM (blocking)
 *   args: none
 *   returns: nothing
 *   behavior: copies the static defaults structure, updates its checksum,
 *             writes it to EEPROM at address 0x0040, and waits for write
 *             completion by polling eeprom_isbusy().
 */
static void write_defaults(void) {
    config_struct def_copy = defaults;
    update_checksum((unsigned char*)&def_copy, sizeof(config_struct));
    eeprom_writebuf(CONFIG_BASE_ADDR, (unsigned char*)&def_copy, sizeof(config_struct));
    while (eeprom_isbusy());
}