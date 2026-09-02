/*
 * ai.c -- AI decision engine and event dispatcher.
 * chk_actT: ~1 Hz priority ladder -> doAct(g_trac).
 * execEv: dispatch deferred events from FIFO.
 * addr: chk_actT(), execEv()
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
   Food-delivery drops silently if the 3-bit food-count is already 4.
   addr: execEv() */

void
execEv(event)
short   event;
{
        in_evrt = YES;

        if (lcp.is_sleeping != NO)
                a_gioob();

        /* STX writes the arms in a different source order -- the
           jump-table targets (28->0x5fd6, 29->0x5fde, 30->0x5ffa,
           31->0x600c, 32->0x5fce, 35->0x6004) put BOOK_DELIVERY
           first and DOG_FOOD last -- and passes 0 to the phone
           handler. */
#ifdef FAITHFUL
        switch (event) {
        case ACTION_EVENT_RECORD_DELIVERY:
                er_recd();
                break;
        case ACTION_EVENT_FOOD_DELIVERY:
                if (((lcp.door_states_and_flags >> 9) & 7) != 4)
                        er_food();
                break;
        case ACTION_EVENT_PHONE_CALL:
                ev_ansPh();
                break;
        case ACTION_EVENT_DOG_FOOD:
                er_dogf();
                break;
        case ACTION_EVENT_BOOK_DELIVERY:
                er_bood();
                break;
        case ACTION_GET_DRESSED:
                a_getd();
                break;
        }
#else
        switch (event) {
        case ACTION_EVENT_BOOK_DELIVERY:        /* 0x5fce */
                er_bood();
                break;
        case ACTION_EVENT_RECORD_DELIVERY:      /* 0x5fd6 */
                er_recd();
                break;
        case ACTION_EVENT_FOOD_DELIVERY:        /* 0x5fde */
                if (((lcp.door_states_and_flags >> 9) & 7) == 4)
                        break;
                er_food();
                break;
        case ACTION_EVENT_PHONE_CALL:           /* 0x5ffa */
                ev_ansPh(0);
                break;
        case ACTION_GET_DRESSED:                /* 0x6004 */
                a_getd();
                break;
        case ACTION_EVENT_DOG_FOOD:             /* 0x600c */
                er_dogf();
                break;
        }
#endif

        in_evrt = NO;
}

/* chk_actT -> parts/chk_actT.c (STX: 0x5ce2, immediately after gameLoop). */
#ifdef FAITHFUL
#include "parts/chk_actT.c"
#endif

/* prsCmd -> parts/prsCmd.c (STX: 0x1721c, in the 0x148fe object after prCh). */
#ifdef FAITHFUL
#include "parts/prsCmd.c"
#endif
