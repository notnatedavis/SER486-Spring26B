/* ********************************************
 * temp.h
 *
 * SER486 - Assignment 4
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * This file provides function declarations for
 * the temperature sensor hardware device pattern
 *
 * Functions:
 *    temp_init() - Initializes ADC for temperature measurement
 *    temp_is_data_ready() - Checks if ADC conversion is complete
 *    temp_start() - Starts an ADC conversion
 *    temp_get() - Reads ADC result and converts to Celsius
 */

#ifndef TEMP_H_INCLUDED
#define TEMP_H_INCLUDED

/* initialize the temperature sensor hardware */
void temp_init(void);

/* returns 1 if most recent ADC conversion is complete, else 0 */
int temp_is_data_ready(void);

/* start a new temperature measurement (ADC conversion) */
void temp_start(void);

/* retrieve the latest temperature value in degrees Celsius */
int temp_get(void);

#endif /* TEMP_H_INCLUDED */ /* headerguardclosingmarker */