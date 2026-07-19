/*
 * ai.c -- AI decision engine and event dispatcher.
 *
 * Two entry points:
 *   chk_actT() -- called ~1 Hz from the main tick
 *     loop.  Walks a 9-priority ladder (event queue, alarm, bathroom,
 *     thirst, hunger, meals, wake/sleep, command queue, time-based
 *     random) and calls doAct() with the winning ACTION_ID.
 *   execEv() -- dispatches deferred events out of the FIFO to
 *     the matching event_receive_* / event_answer_* handler.
 *
 * addr: chk_actT(), execEv()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    t_hour;
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   lunT_trg;
extern BOOL16   dinT_trg;
extern BOOL16   wkT_trg;
extern BOOL16   bedT_trg;
extern short    g_trel[];
extern BOOL16   in_evrt;
extern short    lastAct;
extern short    g_trac;
extern BOOL16   alarm_p;
extern short    lcp_watr;
extern short    g_aliss;
extern short    g_aqueu[];
extern short    g_apriq[];
extern void     a_getd();
extern char     g_cdinb[];
extern char *   cmd_inp;
extern short    g_aprio;
extern short    rndRng();                  /* random.c */
extern short    getEv();          /* events.c  */
extern void     execEv();                /* ai.c      */
extern void     chk_actT();/* ai.c      */
extern void     doAct();                    /* actions.c */
/* Forward-declared handlers (real ports arrive later, one .c per group). */
extern void     a_gioob();
extern void     er_recd();
extern void     er_food();
extern void     er_bood();
extern void     er_dogf();
extern void     ev_ansPh();

/* execEv: dispatch a single deferred event to its handler.
   Guards against recursion via in_evrt and forces
   the resident out of bed first if asleep.  Food-delivery has an extra
   guard: the 3-bit food-count field must not be full (4) or the event
   is dropped silently.

   addr: execEv() */

void
execEv(event)
short   event;
{
        in_evrt = YES;

        if (lcp.is_sleeping != NO)
                a_gioob();

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

        in_evrt = NO;
}

/* ---- 9-priority AI decision engine ------------------------------------- */

/* Externals implemented elsewhere (or stubbed). */
extern void     doAct();
extern short    chk_timA();
extern short    rndRng();

/* Command-queue globals filled from typed input.  Priority is bumped
   every rejected round until it crosses the 8 threshold, at which point
   the queued command wins. */

/* chk_actT: pick the next action for the resident.
   The nine priority levels (in order):
     1. Event queue drained via execEv()
     2. Ctrl+A alarm -> ACTION_WAKE_FROM_ALARM
     3. Bathroom need -> ACTION_USE_TOILET
     4. Thirst (randomly skipped if sick)  -> ACTION_DRINK
     5. Hunger (randomly skipped if sick)  -> ACTION_KITCHEN_CABINET
     6. Scheduled lunch -> ACTION_EAT_MEAL (once per day)
     7. Scheduled dinner -> ACTION_EAT_MEAL (once per day)
     8. Scheduled wake -> ACTION_WAKE_UP_MORNING (once per day)
     9. Scheduled bedtime -> ACTION_GO_TO_BED_NIGHT (once per day)
    10. User command queue with priority escalation
    11. Random time/mood-based action from personality tables

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

        /* Sickness biases thirst/hunger: 66% skip when healthy so the
           resident doesn't guzzle water constantly, 0% skip when sick
           (so sickness always drives food/water). */
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
        /* P5: hunger.
           Ghidra fires KITCHEN_CABINET when hunger > 0 AND rnd > skip AND
             ( (sick   AND food_slots > 0) OR
               (healthy AND lastAct != ACTION_KITCHEN_CABINET) )
           The `sick AND food > 0` disjunct is what lets a sick resident
           chain multiple KITCHEN_CABINET visits.  The port previously
           had a boolean shape that OR-ed food_slots with healthy and
           only guarded lastAct at the outer AND, which meant a sick
           resident whose lastAct was already KITCHEN_CABINET would
           bounce out even with food available.  See Ghidra's
           check_for_any_action_triggers hunger branch (inverted skip
           form) for the exact boolean shape. */
        if (lcp.hunger_level > 0) {
                rnd = rndRng(1, 100);
                if (rnd > sickness_skip_probability &&
                    ((lcp.sickness_level != SICKNESS_HEALTHY &&
                      ((lcp.door_states_and_flags >> 9) & 7) > 0) ||
                     (lcp.sickness_level == SICKNESS_HEALTHY &&
                      lastAct != ACTION_KITCHEN_CABINET))) {
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

/* prsCmd: called from deal_kc when the user
   presses Enter.  Runs the natural-language parser
   chk_encm() over the current g_cdinb, and
   if it returns a valid ACTION_ID and the queue has room, appends it
   with the priority currently sitting in g_aprio.

   addr: prsCmd() */

extern short    chk_encm();

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
