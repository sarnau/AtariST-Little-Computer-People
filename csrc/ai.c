/*
 * ai.c -- AI decision engine and event dispatcher.
 *
 * Two entry points:
 *   check_for_any_action_triggers() -- called ~1 Hz from the main tick
 *     loop.  Walks a 9-priority ladder (event queue, alarm, bathroom,
 *     thirst, hunger, meals, wake/sleep, command queue, time-based
 *     random) and calls do_action() with the winning ACTION_ID.
 *   execute_event() -- dispatches deferred events out of the FIFO to
 *     the matching event_receive_* / event_answer_* handler.
 *
 * addr: check_for_any_action_triggers(), execute_event()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    time_hours;
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   lunch_meal_triggered_today;
extern BOOL16   dinner_meal_triggered_today;
extern BOOL16   morning_wakeup_triggered_today;
extern BOOL16   bedtime_triggered_today;
extern short    g_trel[];
extern BOOL16   in_execute_event_routine_flag;
extern short    last_action;
extern short    g_trac;
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    lcp_water_level;
extern short    g_aliss;
extern short    g_aqueu[];
extern short    g_apriq[];
extern void     a_getd();
extern char     g_cdinb[];
extern char *   _command_input_ptr;
extern short    g_aprio;
extern short    randomRange();                  /* random.c */
extern short    get_event_from_list();          /* events.c  */
extern void     execute_event();                /* ai.c      */
extern void     check_for_any_action_triggers();/* ai.c      */
extern void     do_action();                    /* actions.c */
/* Forward-declared handlers (real ports arrive later, one .c per group). */
extern void     a_gioob();
extern void     er_recd();
extern void     er_food();
extern void     er_bood();
extern void     er_dogf();
extern void     event_answer_phone();

/* execute_event: dispatch a single deferred event to its handler.
   Guards against recursion via in_execute_event_routine_flag and forces
   the resident out of bed first if asleep.  Food-delivery has an extra
   guard: the 3-bit food-count field must not be full (4) or the event
   is dropped silently.

   addr: execute_event() */

void
execute_event(event)
short   event;
{
        in_execute_event_routine_flag = YES;

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
                event_answer_phone();
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

        in_execute_event_routine_flag = NO;
}

/* ---- 9-priority AI decision engine ------------------------------------- */

/* Externals implemented elsewhere (or stubbed). */
extern void     do_action();
extern short    check_time_based_actions();
extern short    randomRange();

/* Command-queue globals filled from typed input.  Priority is bumped
   every rejected round until it crosses the 8 threshold, at which point
   the queued command wins. */

/* check_for_any_action_triggers: pick the next action for the resident.
   The nine priority levels (in order):
     1. Event queue drained via execute_event()
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

   addr: check_for_any_action_triggers() */

void
check_for_any_action_triggers()
{
        short   event;
        short   sickness_skip_probability;
        short   rnd;
        short   index;

        /* P1: process any deferred event first */
        if (g_trel[0] != ACTION_NONE) {
                event = get_event_from_list();
                execute_event(event);
                return;
        }

        /* P2: alarm clock */
        if (ctrl_a_alarm_pressed_flag != NO) {
                g_trac = ACTION_WAKE_FROM_ALARM;
                do_action();
                return;
        }

        /* P3: bathroom */
        if (lcp.bathroom_need != NO) {
                g_trac = ACTION_USE_TOILET;
                do_action();
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
                rnd = randomRange(1, 100);
                if (rnd > sickness_skip_probability &&
                    !(lcp.sickness_level != SICKNESS_HEALTHY &&
                      lcp_water_level == 0)) {
                        g_trac = ACTION_DRINK;
                        do_action();
                        return;
                }
        }

        /* P5: hunger */
        if (lcp.hunger_level > 0) {
                rnd = randomRange(1, 100);
                if (rnd > sickness_skip_probability &&
                    !(lcp.sickness_level != SICKNESS_HEALTHY &&
                      ((lcp.door_states_and_flags >> 9) & 7) == 0) &&
                    last_action != ACTION_KITCHEN_CABINET) {
                        g_trac = ACTION_KITCHEN_CABINET;
                        do_action();
                        last_action = ACTION_KITCHEN_CABINET;
                        return;
                }
        }

        /* P6-P9: once-per-day scheduled events */
        if (!lunch_meal_triggered_today && lcp.lunch_hour == time_hours) {
                g_trac = ACTION_EAT_MEAL;
                do_action();
                lunch_meal_triggered_today = YES;
                return;
        }
        if (!dinner_meal_triggered_today && lcp.dinner_hour == time_hours) {
                g_trac = ACTION_EAT_MEAL;
                do_action();
                dinner_meal_triggered_today = YES;
                return;
        }
        if (!morning_wakeup_triggered_today && lcp.wake_hour == time_hours) {
                g_trac = ACTION_WAKE_UP_MORNING;
                do_action();
                morning_wakeup_triggered_today = YES;
                return;
        }
        if (!bedtime_triggered_today && lcp.bedtime_hour == time_hours) {
                g_trac = ACTION_GO_TO_BED_NIGHT;
                do_action();
                bedtime_triggered_today = YES;
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
                        do_action();
                        return;
                } else {
                        g_apriq[0] =
                                g_apriq[0] + 1;
                }
        }

        /* P11: time/mood-based random pick */
        g_trac = check_time_based_actions();
        if (g_trac >= 0)
                do_action();
}

/* parse_command_to_action: called from deal_with_keycode when the user
   presses Enter.  Runs the natural-language parser
   check_entered_command() over the current g_cdinb, and
   if it returns a valid ACTION_ID and the queue has room, appends it
   with the priority currently sitting in g_aprio.

   addr: parse_command_to_action() */

extern short    check_entered_command();

void
parse_command_to_action()
{
        short   entered;

        _command_input_ptr = g_cdinb;
        entered = check_entered_command(g_cdinb);
        if (entered >= 0 && g_aliss < 10) {
                g_aqueu[g_aliss]           = entered;
                g_apriq[g_aliss]  = g_aprio;
                g_aliss = g_aliss + 1;
        }
}
