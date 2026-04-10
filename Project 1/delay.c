/* ********************************************
 * delay.c
 *
 * SER486 - Project 1 Counter Timers
 * Spring '26
 * Written By:  Nathaniel Davis-Perez
 *
 * Implements two instances of a 1‑ms resolution delay timer using
 * Timer0 in CTC mode. The interrupt service routine increments
 * per‑instance counters up to their respective limits.
 *
 * Functions:
 *   delay_init() - configures Timer0
 *   delay_get() - returns current elapsed ms for instance
 *   delay_set() - sets limit and resets count for instance
 *   delay_isdone() - returns 1 if count reached limit
 *   __vector_14() - Timer0 Compare A ISR (1 ms tick)
 */

/* ----- hardcoded AVR register addresses (atmega328p) ----- */
#define SREG (*(volatile unsigned char*)0x5F)
#define TCCR0A (*(volatile unsigned char*)0x44)
#define TCCR0B (*(volatile unsigned char*)0x45)
#define OCR0A (*(volatile unsigned char*)0x47)
#define TIMSK0 (*(volatile unsigned char*)0x6E)

/* ----- bit values (prescaler 64, CTC mode, OCIE0A) ----- */
#define CTC_BIT 0x02 /* WGM01 = 1 */
#define PRESC_64 0x03 /* CS00=1, CS01=1 */
#define OC_COMP_VAL 249 /* 1 ms @ 16 MHz */
#define OCIE0A_BIT 0x02 /* Timer0 Compare A interrupt enable */

/* ----- inline assembly for cli/sei (atomic operations) ----- */
#define cli() __asm__ volatile ("cli" ::: "memory")
#define sei() __asm__ volatile ("sei" ::: "memory")

/* internal state – two delay instances (0 and 1) */
static volatile unsigned int count[2] = {0, 0};
static unsigned int limit[2] = {0, 0};
static unsigned char initialized = 0;

/* ********************************************
 * delay_init - configure Timer0 for 1 ms CTC interrupts
 *   args: none
 *   returns: nothing
 *   behavior: sets CTC mode, prescaler 64, compare value 249,
 *             enables Output Compare A interrupt. Called once.
 */
static void delay_init(void) {
    if (initialized) return;
    TCCR0A = CTC_BIT; /* CTC mode (WGM01=1) */
    TCCR0B = PRESC_64; /* prescaler = 64 */
    OCR0A  = OC_COMP_VAL; /* compare value for 1 ms */
    TIMSK0 = OCIE0A_BIT; /* enable interrupt */
    initialized = 1;
}

/* ********************************************
 * delay_get - return current count for a delay instance
 *   args: num - instance index (0 or 1)
 *   returns: current elapsed milliseconds for that instance
 *   behavior: atomic read (disables interrupts temporarily)
 */
unsigned delay_get(unsigned num) {
    unsigned char sreg = SREG;
    unsigned int val;
    cli();
    val = count[num];
    SREG = sreg;
    return val;
}

/* ********************************************
 * delay_set - set delay limit and reset count for an instance
 *   args: num  - instance index (0 or 1)
 *         msec - delay limit in milliseconds
 *   returns: nothing
 *   behavior: initializes Timer0 if needed, then atomically sets
 *             limit and clears count
 */
void delay_set(unsigned int num, unsigned int msec) {
    if (!initialized) delay_init();
    unsigned char sreg = SREG;
    cli();
    limit[num] = msec;
    count[num] = 0;
    SREG = sreg;
}

/* ********************************************
 * delay_isdone - check if a delay instance has finished
 *   args: num - instance index (0 or 1)
 *   returns: 1 if count >= limit, otherwise 0
 *   behavior: atomic read-compare
 */
unsigned delay_isdone(unsigned int num) {
    unsigned char sreg = SREG;
    unsigned char done;
    cli();
    done = (count[num] >= limit[num]);
    SREG = sreg;
    return done;
}

/* ********************************************
 * __vector_14 - Timer0 Compare A interrupt service routine
 *   args: none
 *   returns: nothing
 *   behavior: increments each delay instance counter up to its limit
 */
void __vector_14(void) __attribute__((signal));
void __vector_14(void) {
    unsigned char i;
    for (i = 0; i < 2; i++) {
        if (count[i] < limit[i]) {
            count[i]++;
        }
    }
}