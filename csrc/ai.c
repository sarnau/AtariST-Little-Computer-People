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
#include "globals.h"

/* Forward-declared handlers (real ports arrive later, one .c per group). */
extern void     action_get_in_out_of_bed();
extern void     action_get_dressed();
extern void     event_receive_record_delivery();
extern void     event_receive_food_delivery();
extern void     event_receive_book_delivery();
extern void     event_receive_dog_food();
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
                action_get_in_out_of_bed();

        switch (event) {
        case ACTION_EVENT_RECORD_DELIVERY:
                event_receive_record_delivery();
                break;
        case ACTION_EVENT_FOOD_DELIVERY:
                if (((lcp.door_states_and_flags >> 9) & 7) != 4)
                        event_receive_food_delivery();
                break;
        case ACTION_EVENT_PHONE_CALL:
                event_answer_phone();
                break;
        case ACTION_EVENT_DOG_FOOD:
                event_receive_dog_food();
                break;
        case ACTION_EVENT_BOOK_DELIVERY:
                event_receive_book_delivery();
                break;
        case ACTION_GET_DRESSED:
                action_get_dressed();
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
extern short    _action_list_size;
extern short    _action_queue[];
extern short    _action_priority_queue[];

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
        if (triggered_event_list[0] != ACTION_NONE) {
                event = get_event_from_list();
                execute_event(event);
                return;
        }

        /* P2: alarm clock */
        if (ctrl_a_alarm_pressed_flag != NO) {
                trigger_action = ACTION_WAKE_FROM_ALARM;
                do_action();
                return;
        }

        /* P3: bathroom */
        if (lcp.bathroom_need != NO) {
                trigger_action = ACTION_USE_TOILET;
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
                        trigger_action = ACTION_DRINK;
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
                        trigger_action = ACTION_KITCHEN_CABINET;
                        do_action();
                        last_action = ACTION_KITCHEN_CABINET;
                        return;
                }
        }

        /* P6-P9: once-per-day scheduled events */
        if (!lunch_meal_triggered_today && lcp.lunch_hour == time_hours) {
                trigger_action = ACTION_EAT_MEAL;
                do_action();
                lunch_meal_triggered_today = YES;
                return;
        }
        if (!dinner_meal_triggered_today && lcp.dinner_hour == time_hours) {
                trigger_action = ACTION_EAT_MEAL;
                do_action();
                dinner_meal_triggered_today = YES;
                return;
        }
        if (!morning_wakeup_triggered_today && lcp.wake_hour == time_hours) {
                trigger_action = ACTION_WAKE_UP_MORNING;
                do_action();
                morning_wakeup_triggered_today = YES;
                return;
        }
        if (!bedtime_triggered_today && lcp.bedtime_hour == time_hours) {
                trigger_action = ACTION_GO_TO_BED_NIGHT;
                do_action();
                bedtime_triggered_today = YES;
                return;
        }

        /* P10: command queue.  Low-priority (0..3) commands get shifted
           out on every rejected round; high-priority (>=8) fire
           immediately.  Middle-priority items get their priority
           incremented and stay in the queue for another shot. */
        if (_action_list_size > 0) {
                if (_action_priority_queue[0] < 4) {
                        for (index = 0; index < 9; index = index + 1) {
                                _action_queue[index] = _action_queue[index + 1];
                                _action_priority_queue[index] =
                                        _action_priority_queue[index + 1];
                        }
                } else if (_action_priority_queue[0] > 7) {
                        trigger_action = _action_queue[0];
                        if (_action_queue[0] == ACTION_PLAY_A_GAME ||
                            _action_queue[0] == ACTION_PLAY_WITH_RECORD)
                                action_get_dressed();
                        for (index = 0; index < 9; index = index + 1) {
                                _action_queue[index] = _action_queue[index + 1];
                                _action_priority_queue[index] =
                                        _action_priority_queue[index + 1];
                        }
                        _action_list_size = _action_list_size - 1;
                        do_action();
                        return;
                } else {
                        _action_priority_queue[0] =
                                _action_priority_queue[0] + 1;
                }
        }

        /* P11: time/mood-based random pick */
        trigger_action = check_time_based_actions();
        if (trigger_action >= 0)
                do_action();
}

/* parse_command_to_action: called from deal_with_keycode when the user
   presses Enter.  Runs the natural-language parser
   check_entered_command() over the current command_input_buffer, and
   if it returns a valid ACTION_ID and the queue has room, appends it
   with the priority currently sitting in _action_priority.

   addr: parse_command_to_action() */

extern short    check_entered_command();

void
parse_command_to_action()
{
        short   entered;

        _command_input_ptr = command_input_buffer;
        entered = check_entered_command(command_input_buffer);
        if (entered >= 0 && _action_list_size < 10) {
                _action_queue[_action_list_size]           = entered;
                _action_priority_queue[_action_list_size]  = _action_priority;
                _action_list_size = _action_list_size + 1;
        }
}
