/* ********************************************
 * temp.c
 *
 * SER486 - Assignment 4
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * This file implements the hardware device pattern
 * for the temperature sensor using the atmega328p ADC
 *
 * Functions:
 *    temp_init() - Initializes ADC for temperature measurement
 *    temp_is_data_ready() - Checks if ADC conversion is complete
 *    temp_start() - Starts an ADC conversion
 *    temp_get() - Reads ADC result and converts to Celsius
 */

/* no header files included */

/* atmega328p ADC register addresses */
#define ADMUX (*(volatile unsigned char*)0x7C) /* ADC multiplexer selection */
#define ADCSRA (*(volatile unsigned char*)0x7A) /* ADC control and status */
#define ADCL (*(volatile unsigned char*)0x78) /* ADC data low byte */
#define ADCH (*(volatile unsigned char*)0x79) /* ADC data high byte */

/* bit positions for ADCSRA */
#define ADEN 7 /* ADC enable */
#define ADSC 6 /* ADC start conversion */
#define ADIF 4 /* ADC interrupt flag */
#define ADPS2 2 /* ADC prescaler bit 2 */
#define ADPS1 1 /* ADC prescaler bit 1 */
#define ADPS0 0 /* ADC prescaler bit 0 */

/* bit positions for ADMUX */
#define REFS1 7 /* reference selection bit 1 */
#define REFS0 6 /* reference selection bit 0 */

/* ********************************************
 * temp_init()
 *
 * Initializes the atmega328p ADC to measure temperature sensor input 
 * channel (ADC0) with internal 1.1V reference
 *
 * Arguments:
 *   none
 *
 * Returns:
 *   void
 */
void temp_init(void)
{
    /* set internal 1.1V reference (REFS1=1, REFS0=1) + select ADC0 (MUX=0) */
    ADMUX = (1 << REFS1) | (1 << REFS0);

    /* enable ADC and set prescaler to 128 */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/* ********************************************
 * temp_is_data_ready()
 *
 * Reads the ADC status and determines if the most recent conversion 
 * has completed
 *
 * Arguments:
 *   none
 *
 * Returns:
 *   1 | if conversion completed, else 0
 */
int temp_is_data_ready(void)
{
    /* check ADIF bit; set when conversion completes */
    return (ADCSRA & (1 << ADIF)) ? 1 : 0;
}

/* ********************************************
 * temp_start()
 *
 * Starts a new ADC conversion by setting the ADSC bit
 *
 * Arguments:
 *   none
 *
 * Returns:
 *   void
 */
void temp_start(void)
{
    /* set the ADC start conversion bit */
    ADCSRA |= (1 << ADSC);
}

/* ********************************************
 * temp_get()
 *
 * Reads the most recent ADC conversion result,
 * converts it to degrees Celsius using the formula :
 *   (degrees = ((reading * 101) / 100) - 273)
 *
 * Arguments:
 *   none
 *
 * Returns:
 *   Temperature in degrees Celsius as signed integer
 */
int temp_get(void)
{
    unsigned char low, high;
    unsigned int reading;
    signed long degrees;

    /* read ADC result low byte first (locks high byte until ADCH is read) */
    low = ADCL;
    high = ADCH;
    reading = ((unsigned int)high << 8) | low;

    /* convert to Celsius using signed long to avoid overflow */
    degrees = ((signed long)reading * 101L) / 100L - 273L;

    /* return as integer */
    return (int)degrees;
}