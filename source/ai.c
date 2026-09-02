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

/* chk_actT: 9-priority AI ladder.
   1. Event queue -> execEv
   2. Alarm -> WAKE_FROM_ALARM   3. Bathroom -> USE_TOILET
   4. Thirst -> DRINK             5. Hunger -> KITCHEN_CABINET
   6. Lunch  7. Dinner  8. Wake  9. Bedtime (once/day scheduled)
   10. User command queue         11. Random time/mood-based
   addr: chk_actT() */

void
chk_actT()
{
        short   event;
        short   sickness_skip_probability;
        short   rnd;
        short   index;
        /* P1: process any deferred event first */
        if (g_trel[0] != ACTION_NONE) {
                event = getEv();
                execEv(event);
                return;
        }
        /* P2: alarm clock */
        if (alarm_p != NO) {
                g_trac = ACTION_WAKE_FROM_ALARM;
                doAct();
                return;
        }
        /* P3: bathroom */
        if (lcp.bathroom_need != NO) {
                g_trac = ACTION_USE_TOILET;
                doAct();
                return;
        }

        /* Sickness bias: 66% skip healthy, 0% sick. */
        if (lcp.sickness_level < 1)
                sickness_skip_probability = 66;
        else
                sickness_skip_probability = 0;
        /* P4: thirst */
        if (lcp.thirst_level > 0) {
                rnd = rndRng(1, 100);
                if (rnd > sickness_skip_probability &&
                    !(lcp.sickness_level != SICKNESS_HEALTHY &&
                      lcp_watr == 0)) {
                        g_trac = ACTION_DRINK;
                        doAct();
                        return;
                }
        }
        /* P5: hunger.  ROM 0x2e82: hunger > 0 AND rnd > skip AND
           (healthy OR food_slots != 0) AND lastAct != KITCHEN_CABINET
           -- the lastAct check gates the sick branch too. */
        if (lcp.hunger_level > 0) {
                rnd = rndRng(1, 100);
                if (rnd > sickness_skip_probability &&
                    (lcp.sickness_level == SICKNESS_HEALTHY ||
                     ((lcp.door_states_and_flags >> 9) & 7) != 0) &&
                    lastAct != ACTION_KITCHEN_CABINET) {
                        g_trac = ACTION_KITCHEN_CABINET;
                        doAct();
                        lastAct = ACTION_KITCHEN_CABINET;
                        return;
                }
        }

        /* P6-P9: once-per-day scheduled events */
        if (!lunT_trg && lcp.lunch_hour == t_hour) {
                g_trac = ACTION_EAT_MEAL;
                doAct();
                lunT_trg = YES;
                return;
        }
        if (!dinT_trg && lcp.dinner_hour == t_hour) {
                g_trac = ACTION_EAT_MEAL;
                doAct();
                dinT_trg = YES;
                return;
        }
        if (!wkT_trg && lcp.wake_hour == t_hour) {
                g_trac = ACTION_WAKE_UP_MORNING;
                doAct();
                wkT_trg = YES;
                return;
        }
        if (!bedT_trg && lcp.bedtime_hour == t_hour) {
                g_trac = ACTION_GO_TO_BED_NIGHT;
                doAct();
                bedT_trg = YES;
                return;
        }
        /* P10: command queue.  Low-priority (0..3) commands get shifted
           out on every rejected round; high-priority (>=8) fire
           immediately.  Middle-priority items get their priority
           incremented and stay in the queue for another shot. */
        if (g_aliss > 0) {
                if (g_apriq[0] < 4) {
                        for (index = 0; index < 9; index = index + 1) {
                                g_aqueu[index] = g_aqueu[index + 1];
                                g_apriq[index] =
                                        g_apriq[index + 1];
                        }
                } else if (g_apriq[0] > 7) {
                        g_trac = g_aqueu[0];
                        if (g_aqueu[0] == ACTION_PLAY_A_GAME ||
                            g_aqueu[0] == ACTION_PLAY_WITH_RECORD)
                                a_getd();
                        for (index = 0; index < 9; index = index + 1) {
                                g_aqueu[index] = g_aqueu[index + 1];
                                g_apriq[index] =
                                        g_apriq[index + 1];
                        }
                        g_aliss = g_aliss - 1;
                        doAct();
                        return;
                } else {
                        g_apriq[0] =
                                g_apriq[0] + 1;
                }
        }

        /* P11: time/mood-based random pick */
        g_trac = chk_timA();
        if (g_trac >= 0)
                doAct();
}

/* prsCmd: called from deal_kc on Enter.  Runs chk_encm() on g_cdinb;
   valid ACTION_ID with queue room is appended at g_aprio priority.
   addr: prsCmd() */


void
prsCmd()
{
        short   entered;

        cmd_inp = g_cdinb;
        entered = chk_encm(g_cdinb);
        if (entered >= 0 && g_aliss < 10) {
                g_aqueu[g_aliss]           = entered;
                g_apriq[g_aliss]  = g_aprio;
                g_aliss = g_aliss + 1;
        }
}
