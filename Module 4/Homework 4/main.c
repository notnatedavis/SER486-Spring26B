/* ********************************************
 * main.c
 *
 * SER486 - Assignment 4
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * This file implements the main entry point for
 * Homework Assignment 4
 *
 * functions are :
 *    main() - program entry point; handles initialization
 *             and the main control loop
 */

#include "simpledelay.h"
#include "simpleled.h"
#include "temp.h"
#include "uart.h"

 /* ********************************************
 * main()
 *
 * Program entry point
 * Initializes LED, UART, and temperature sensor
 * Outputs a banner message, then enters an infinite loop that
 * starts an ADC conversion, waits for completion, retrieves the temp
 * prints it and delays 1 second
 *
 * args :
 *   none
 *
 * returns :
 *   void
 */
 int main()
 {
   /* initialize all required hardware */
   led_init();
   uart_init();
   temp_init();
   
   /* output banner message w/ required newline and carriage return */
   uart_writestr("SER 486 HW4 – Nathaniel Davis-Perez\n\r");

   /* main control loop (infinite) */
   while (1) {
      /* start a new temperature conversion */
      temp_start();

      /* wait for conversion to complete */
      while (!temp_is_data_ready()) {
         /* busy-wait for ADC completion */
      }

      /* get the converted temperature value */
      int temperature = temp_get();

      /* output temperature as decimal via UART */
      uart_writedec32(temperature);
      uart_writestr("\n\r");

      /* wait 1 second before next reading */
      delay(1000);
   }

   /* never reached */
   return 0;
}