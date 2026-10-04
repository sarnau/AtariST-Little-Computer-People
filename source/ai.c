/*
 * ai.c -- AI decision engine and event dispatcher.
 * chk_actT: ~1 Hz priority ladder -> doAct(g_trac).
 * execEv: dispatch deferred events from FIFO.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "actions.h"
#include "ahouse.h"
#include "ai.h"
#include "airandom.h"
#include "delivery.h"
#include "events.h"
#include "globals.h"
#include "parser.h"
#include "random.h"

/* execEv: dispatch a single deferred event to its handler.
   in_evrt guards recursion; sleeper is forced out of bed first.
   Food-delivery drops silently if the 3-bit food-count is already 4. */

void
execEv(event)
short   event;
{
        in_evrt = YES;

        if (lcp.is_sleeping != NO)
                a_gioob();

        /* The arms are in the original's source order (BOOK_DELIVERY
           first, DOG_FOOD last), which decides the code layout; keep it.
           The phone handler is passed 0. */
        switch (event) {
        case ACTION_EVENT_BOOK_DELIVERY:
                er_bood();
                break;
        case ACTION_EVENT_RECORD_DELIVERY:
                er_recd();
                break;
        case ACTION_EVENT_FOOD_DELIVERY:
                if (((lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) == FOOD_PACKS_MAX)
                        break;
                er_food();
                break;
        case ACTION_EVENT_PHONE_CALL:
                ev_ansPh(0);
                break;
        case ACTION_GET_DRESSED:
                a_getd();
                break;
        case ACTION_EVENT_DOG_FOOD:
                er_dogf();
                break;
        }

        in_evrt = NO;
}

/* chk_actT -> parts/chk_actT.c. */

/* prsCmd -> parts/prsCmd.c. */
