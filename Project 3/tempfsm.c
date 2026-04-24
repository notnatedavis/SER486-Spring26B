/********************************************************
 * tempfsm.c
 *
 * SER486 – Project 3 Integration
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Implements a 9‑state temperature FSM that provides
 * hysteresis, generates alarms and sets the LED blink
 * pattern according to the current state.
 *
 * Functions :
 *   tempfsm_init() – set initial state NORM1
 *   tempfsm_reset() – jump back to NORM1
 *   tempfsm_update() – main FSM evaluation
 *   tempfsm_get_state() – return string like "NORM1"
 */

/* ----- Imports ----- */
#include "tempfsm.h"
#include "alarm.h"
#include "led.h"
#include "log.h" // for event macros

/* ----- Internal state ----- */
typedef enum {
    NORM1, NORM2, NORM3,
    WARNHI1, WARNHI2,
    CRITICALHI,
    WARNLO1, WARNLO2,
    CRITICALLO
} temp_state_t;

static temp_state_t state = NORM1;

/* ----- Public functions ----- */

/********************************************************
 * tempfsm_init – initialise FSM
 *   args: none
 *   returns: nothing
 *   behavior: sets state to NORM1, turns off LED blink
 */
void tempfsm_init(void) {
    state = NORM1;
    led_set_blink(" "); // no blink
}

/********************************************************
 * tempfsm_reset – force state to NORM1
 *   args: none
 *   returns: nothing
 */
void tempfsm_reset(void) {
    state = NORM1;
    led_set_blink(" ");
}

/********************************************************
 * tempfsm_update – evaluate temperature and transition
 *   args: current, hicrit, hiwarn, locrit, lowarn
 *   returns: nothing
 *   behavior: checks outgoing transitions from current
 *             state and moves to next state when
 *             condition is met, triggering alarms and
 *             LED pattern changes as defined.
 */
void tempfsm_update(int current, int hicrit, int hiwarn, int locrit, int lowarn) {
    switch (state) {
    case NORM1 :
        if (current >= hiwarn) {
            alarm_send(EVENT_HI_WARN);
            led_set_blink("-");
            state = WARNHI1;
        } else if (current <= lowarn) {
            alarm_send(EVENT_LO_WARN);
            led_set_blink("-");
            state = WARNLO1;
        }
        /* else stay NORM1, LED remains " " */
        break;

    case NORM2 :
        if (current >= hiwarn) {
            alarm_send(EVENT_HI_WARN);
            led_set_blink("-");
            state = WARNHI1;
        } else if (current <= lowarn) {
            /* no alarm on this transition */
            led_set_blink("-");
            state = WARNLO1;
        }
        break;

    case NORM3 :
        if (current <= lowarn) {
            alarm_send(EVENT_LO_WARN);
            led_set_blink("-");
            state = WARNLO1;
        } else if (current >= hiwarn) {
            /* no alarm here */
            led_set_blink("-");
            state = WARNHI1;
        }
        break;

    case WARNHI1 :
        if (current >= hicrit) {
            alarm_send(EVENT_HI_ALARM);
            led_set_blink(".");
            state = CRITICALHI;
        } else if (current < hiwarn) {
            led_set_blink(" ");
            state = NORM3;
        }
        break;

    case WARNHI2 :
        if (current < hiwarn) {
            led_set_blink(" ");
            state = NORM3;
        } else if (current >= hicrit) {
            led_set_blink(".");
            state = CRITICALHI;
            /* no alarm sent from WARNHI2 -> CRITICALHI */
        }
        break;

    case CRITICALHI :
        if (current < hicrit) {
            led_set_blink("-");
            state = WARNHI2;
        }
        break;

    case WARNLO1 :
        if (current > lowarn) {
            led_set_blink(" ");
            state = NORM2;
        } else if (current <= locrit) {
            alarm_send(EVENT_LO_ALARM);
            led_set_blink(".");
            state = CRITICALLO;
        }
        break;

    case WARNLO2 :
        if (current > lowarn) {
            led_set_blink(" ");
            state = NORM2;
        } else if (current <= locrit) {
            led_set_blink(".");
            state = CRITICALLO;
        }
        break;

    case CRITICALLO :
        if (current > locrit) {
            led_set_blink("-");
            state = WARNLO2;
        }
        break;

    default :
        /* safety fallback */
        state = NORM1;
        led_set_blink(" ");
        break;
    }
}

/********************************************************
 * tempfsm_get_state – return readable state name
 *   args: none
 *   returns: constant string for current state
 */
const char* tempfsm_get_state(void) {
    switch (state) {
    case NORM1 : return "NORM1";
    case NORM2 : return "NORM2";
    case NORM3 : return "NORM3";
    case WARNHI1 : return "WARN_HI";
    case WARNHI2 : return "WARN_HI";
    case CRITICALHI : return "CRIT_HI";
    case WARNLO1 : return "WARN_LO";
    case WARNLO2 : return "WARN_LO";
    case CRITICALLO : return "CRIT_LO";
    default : return "NORM1";
    }
}