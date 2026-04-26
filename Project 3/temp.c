/********************************************************
 * temp.c
 *
 * SER486 – Project 3 (standalone)
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Simulated temperature sensor. Returns a fixed value.
 * Change temperature to test FSM transitions.
 ********************************************************/
#include "temp.h"

static int temperature = 88;   // can be changed for testing

void temp_init(void) { }

int temp_is_data_ready(void) {
    return 1;
}

void temp_start(void) { }

int temp_get(void) {
    return temperature;
}