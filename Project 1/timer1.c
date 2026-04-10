/* ********************************************
 * timer1.c
 *
 * SER486 - Project 1 Counter Timers
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * Implements a free‑running 32‑bit counter with 1‑second resolution
 * using Timer1 in CTC mode. The ISR increments the counter each second.
 *
 * Functions:
 *   timer1_init()   - configures Timer1 for 1 Hz interrupts
 *   timer1_get()    - returns current tick count (atomic)
 *   timer1_clear()  - resets tick count to 0 (atomic)
 *   __vector_11()   - Timer1 Compare A ISR
 */

/* ----- hardcoded AVR register addresses (atmega328p) ----- */
#define SREG (*(volatile unsigned char*)0x5F)
#define TCCR1B (*(volatile unsigned char*)0x81)
#define OCR1AL (*(volatile unsigned char*)0x88) /* low byte */
#define OCR1AH (*(volatile unsigned char*)0x89) /* high byte */
#define TIMSK1 (*(volatile unsigned char*)0x6F)
#define TIFR1 (*(volatile unsigned char*)0x36)
/* helper to write 16-bit OCR1A */
#define OCR1A       (*(volatile unsigned int*)0x88)

/* ----- bit values (prescaler 1024, CTC mode, OCIE1A) ----- */
#define CTC_MODE_BIT 0x08 /* WGM12 = 1 */
#define PRESC_1024 0x05 /* CS12=1, CS10=1 */
#define TCCR1B_VAL (CTC_MODE_BIT | PRESC_1024) /* 0x0D */
#define OC_COMP_VAL 15624 /* 1 Hz @ 16 MHz */
#define OCIE1A_BIT 0x02 /* Timer1 Compare A interrupt enable */
#define OCF1A_BIT 0x02 /* Output Compare A flag */

/* ----- inline assembly for cli/sei (atomic operations) ----- */
#define cli() __asm__ volatile ("cli" ::: "memory")
#define sei() __asm__ volatile ("sei" ::: "memory")

static volatile unsigned long count = 0;   /* seconds since last clear */

/* ********************************************
 * timer1_init - initialize Timer1 for 1 second CTC interrupts
 *   args: none
 *   returns: nothing
 *   behavior: sets CTC mode, prescaler 1024, compare value 15624,
 *             enables Output Compare A interrupt, clears pending flag
 */
void timer1_init(void) {
    TCCR1B = TCCR1B_VAL;           /* CTC mode, prescaler 1024 */
    OCR1A  = OC_COMP_VAL;           /* 1 second compare value */
    TIMSK1 = OCIE1A_BIT;            /* enable interrupt */
    TIFR1  = OCF1A_BIT;             /* clear any pending flag */
}

/* ********************************************
 * timer1_get - return current 1‑second tick count
 *   args: none
 *   returns: number of seconds elapsed since last clear or power‑up
 *   behavior: atomic read (disables interrupts)
 */
unsigned long timer1_get(void) {
    unsigned char sreg = SREG;
    unsigned long val;
    cli();
    val = count;
    SREG = sreg;
    return val;
}

/* ********************************************
 * timer1_clear - reset the tick counter to zero
 *   args: none
 *   returns: nothing
 *   behavior: atomic write (disables interrupts)
 */
void timer1_clear(void) {
    unsigned char sreg = SREG;
    cli();
    count = 0;
    SREG = sreg;
}

/* ********************************************
 * __vector_11 - Timer1 Compare A interrupt service routine
 *   args: none
 *   returns: nothing
 *   behavior: increments the 32‑bit second counter
 */
void __vector_11(void) __attribute__((signal));
void __vector_11(void) {
    count++;
}