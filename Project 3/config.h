/* ********************************************
 * config.h
 *
 * SER486 - Project 2 EEPROM
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Defines configuration structure and public interface for
 * managing system configuration stored in EEPROM at address 0x0040.
 *
 * Functions:
 *   config_init() - load configuration from EEPROM
 *   config_update() - write back modified configuration
 *   config_set_modified() - mark configuration as dirty
 */

/* ----- Defined ----- */
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* Config structure */
typedef struct {
    char token[4]; /* "ASU" */
    unsigned int hi_alarm; /* 0x3FF */
    unsigned int hi_warn; /* 0x3FE */
    unsigned int lo_alarm; /* 0x0000 */
    unsigned int lo_warn; /* 0x0001 */
    char use_static_ip; /* 0 = DHCP, 1 = static */
    unsigned char static_ip[4]; /* e.g. {192,168,1,100} */
    unsigned char checksum;
} config_struct;

/* Public config data (can be accessed directly) */
extern config_struct config;

/* ********************************************
 * config_init - initialises config from EEPROM; writes defaults if invalid
 *   args: none
 *   returns: nothing
 */
void config_init(void);

/* ********************************************
 * config_update - writes modified config to EEPROM if not busy
 *   args: none
 *   returns: nothing
 */
void config_update(void);

/* ********************************************
 * config_set_modified - marks config as modified for later update
 *   args: none
 *   returns: nothing
 */
void config_set_modified(void);

#endif /* CONFIG_H_INCLUDED */