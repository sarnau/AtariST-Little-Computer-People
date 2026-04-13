"""
AI decision engine for Little Computer People (Atari ST).
Translated from Ghidra decompilation of check_for_any_action_triggers().

addr: check_for_any_action_triggers()

Called once per game loop iteration (after game_tick_and_animate).
Evaluates conditions in strict priority order and dispatches the next action.

Priority order (from decompiled C):
  1. Event queue (triggered_event_list[0]) — doorbell, food, phone, etc.
  2. Ctrl+A alarm pressed → ACTION_WAKE_FROM_ALARM
  3. Bathroom need → ACTION_USE_TOILET
  4. Thirst (randomised by sickness level, 66% skip chance if sick) → ACTION_DRINK
  5. Hunger (same randomisation) → ACTION_KITCHEN_CABINET
  6. Scheduled meals at configured hours → ACTION_EAT_MEAL
  7. Scheduled sleep/wake → ACTION_GO_TO_BED_NIGHT / ACTION_WAKE_UP_MORNING
  8. Player command queue (with priority escalation)
  9. Random personality-weighted action → check_time_based_actions()
"""

import random
from typing import TYPE_CHECKING

from .enums import ACTION_ID, SICKNESS_LEVEL, HAPPINESS_LEVEL
from .constants import (
    ACTION_TABLES, ACTIVITY_SCHEDULE_TABLE,
    ACTION_TABLE_ACTIVE, ACTION_TABLE_MODERATE, ACTION_TABLE_RELAXED,
)
from .state import GameState

if TYPE_CHECKING:
    pass


# ---------------------------------------------------------------------------
# Forward references — do_action and action_get_dressed are imported lazily
# inside the calling functions to avoid circular imports.
# ---------------------------------------------------------------------------


def random_range(lo: int, hi: int) -> int:
    """addr: randomRange() utility"""
    return random.randint(lo, hi)


# ---------------------------------------------------------------------------
# Event queue helpers
# addr: put_event_to_list(), get_event_from_list(), execute_event()
# ---------------------------------------------------------------------------

def get_event_from_list(gs: GameState) -> int:
    """
    Dequeue and return the first event from triggered_event_list[].
    Shifts remaining events down by one slot.
    addr: get_event_from_list()
    """
    event = gs.triggered_event_list[0]
    for i in range(len(gs.triggered_event_list) - 1):
        gs.triggered_event_list[i] = gs.triggered_event_list[i + 1]
    gs.triggered_event_list[-1] = ACTION_ID.ACTION_NONE
    return event


def execute_event(gs: GameState, event: int) -> None:
    """
    Execute a triggered event (doorbell, delivery, phone call, etc.).
    Has its own switch — does NOT go through do_action().
    addr: execute_event()
    """
    from .actions import (
        action_get_in_out_of_bed, action_get_dressed,
        event_receive_record_delivery, event_receive_food_delivery,
        event_answer_phone, event_receive_dog_food,
        event_receive_book_delivery,
    )
    gs.in_execute_event_routine_flag = 1
    if gs.lcp.is_sleeping:
        action_get_in_out_of_bed(gs)
    if event == ACTION_ID.ACTION_EVENT_RECORD_DELIVERY:
        event_receive_record_delivery(gs)
    elif event == ACTION_ID.ACTION_EVENT_FOOD_DELIVERY:
        food_count = (gs.lcp.door_states_and_flags >> 9) & 7
        if food_count != 4:
            event_receive_food_delivery(gs)
    elif event == ACTION_ID.ACTION_EVENT_PHONE_CALL:
        event_answer_phone(gs)
    elif event == ACTION_ID.ACTION_EVENT_DOG_FOOD:
        event_receive_dog_food(gs)
    elif event == ACTION_ID.ACTION_EVENT_BOOK_DELIVERY:
        event_receive_book_delivery(gs)
    elif event == ACTION_ID.ACTION_GET_DRESSED:
        action_get_dressed(gs)
    gs.in_execute_event_routine_flag = 0


# ---------------------------------------------------------------------------
# Time-based random action selector
# addr: check_time_based_actions()
# Selects a random action from the appropriate personality table based on
# time-of-day, activity_level, and day-of-week.
# Returns ACTION_NONE (0xFFFF) if LCP initiative threshold not crossed.
# ---------------------------------------------------------------------------

def calc_weekday(gs: GameState) -> int:
    """
    Calculate day of week (0=Sunday .. 6=Saturday) from current date.
    Uses Zeller-like formula matching the original game code.
    addr: calc_weekday()
    """
    day_offset = 1
    for y in range(gs.date_year):
        day_offset += 2 if (y % 4 == 0) else 1
    from .simulation import days_in_month
    for m in range(gs.date_month):
        day_offset += days_in_month(m, gs.date_year)
    return (gs.date_day + day_offset) % 7


def check_time_based_actions(gs: GameState) -> int:
    """
    Pick a random action from the personality/time-weighted action tables.
    Returns ACTION_NONE if sleeping, else an action from the appropriate table.
    addr: check_time_based_actions()
    """
    hours_since_wake = gs.time_hours - gs.lcp.wake_hour
    if hours_since_wake < 0:
        hours_since_wake += 24

    if hours_since_wake >= 18 or gs.lcp.sickness_level >= 2:
        # Bed mode
        table_index = 3
    else:
        # Look up activity schedule table
        time_period = (hours_since_wake // 2) % 3
        activity_idx = min(int(gs.lcp.activity_level), 7)
        table_index = ACTIVITY_SCHEDULE_TABLE[time_period][activity_idx]

        # Day-of-week override: only when schedule says "active" (0)
        if table_index == 0:
            day = calc_weekday(gs)
            if day == 0:       # Sunday → relaxed
                table_index = 2
            elif day == 6:     # Saturday → moderate
                table_index = 1

    # Dispatch by table_index
    if table_index == 0:
        while True:
            action = ACTION_TABLE_ACTIVE[random_range(0, 15)]
            if action != gs.last_action:
                return action
    elif table_index == 1:
        while True:
            action = ACTION_TABLE_MODERATE[random_range(0, 15)]
            if action != gs.last_action:
                return action
    elif table_index == 2:
        while True:
            action = ACTION_TABLE_RELAXED[random_range(0, 15)]
            if action != gs.last_action:
                return action
    else:
        # table_index == 3: bed mode
        if gs.lcp.is_sleeping == 0:
            return ACTION_ID.ACTION_GET_IN_OUT_OF_BED
        return ACTION_ID.ACTION_NONE


# ---------------------------------------------------------------------------
# Player command queue helpers
# addr: _action_queue[], _action_priority_queue[], _action_list_size
# ---------------------------------------------------------------------------

def add_command_to_queue(gs: GameState, action: int, priority: int) -> None:
    """
    Push a player-typed command into the command queue.
    addr: (NLP result insertion into _action_queue)
    """
    if gs.action_list_size < 10:
        idx = gs.action_list_size
        gs.action_queue[idx]          = action
        gs.action_priority_queue[idx] = priority
        gs.action_list_size          += 1


def _shift_command_queue(gs: GameState) -> None:
    """Shift all command queue entries down by one (remove front entry)."""
    for i in range(9):
        gs.action_queue[i]          = gs.action_queue[i + 1]
        gs.action_priority_queue[i] = gs.action_priority_queue[i + 1]
    gs.action_queue[9]          = ACTION_ID.ACTION_NONE
    gs.action_priority_queue[9] = 0


# ---------------------------------------------------------------------------
# Main AI decision function
# addr: check_for_any_action_triggers()
# ---------------------------------------------------------------------------

def check_for_any_action_triggers(gs: GameState) -> None:
    """
    Central AI decision function.  Evaluates all conditions in strict priority
    order and calls do_action() with the appropriate trigger_action set.
    addr: check_for_any_action_triggers()
    """
    from .actions import do_action, action_get_dressed

    # -----------------------------------------------------------------------
    # Priority 1: Process event queue (doorbell, food delivery, phone, etc.)
    # -----------------------------------------------------------------------
    if gs.triggered_event_list[0] != ACTION_ID.ACTION_NONE:
        event = get_event_from_list(gs)
        execute_event(gs, event)
        return

    # -----------------------------------------------------------------------
    # Priority 2: Ctrl+A alarm
    # -----------------------------------------------------------------------
    if gs.ctrl_a_alarm_pressed_flag != 0:
        gs.trigger_action = ACTION_ID.ACTION_WAKE_FROM_ALARM
        do_action(gs)
        return

    # -----------------------------------------------------------------------
    # Priority 3: Bathroom need
    # -----------------------------------------------------------------------
    if gs.lcp.bathroom_need != 0:
        gs.trigger_action = ACTION_ID.ACTION_USE_TOILET
        do_action(gs)
        return

    # -----------------------------------------------------------------------
    # Priority 4: Thirst
    # Sickness check: if sick, only 34% chance to skip thirst action (66% skip
    # chance = 66% probability the thirst is ignored, per original C logic).
    # -----------------------------------------------------------------------
    probability = 66 if gs.lcp.sickness_level < 1 else 0
    thirst_skip = (
        gs.lcp.thirst_level < 1
        or random_range(1, 100) <= probability
        or (gs.lcp.sickness_level != SICKNESS_LEVEL.SICKNESS_HEALTHY
            and gs.lcp_water_level == 0)
    )

    if not thirst_skip:
        gs.trigger_action = ACTION_ID.ACTION_DRINK
        do_action(gs)
        return

    # -----------------------------------------------------------------------
    # Priority 5: Hunger
    # Cabinet state: bits 9–11 of door_states_and_flags encode cabinet fill
    # -----------------------------------------------------------------------
    cabinet_state = (gs.lcp.door_states_and_flags >> 9) & 7
    hunger_skip = (
        gs.lcp.hunger_level < 1
        or random_range(1, 100) <= probability
        or ((gs.lcp.sickness_level == SICKNESS_LEVEL.SICKNESS_HEALTHY
             or cabinet_state == 0)
            and (gs.last_action == ACTION_ID.ACTION_KITCHEN_CABINET
                 or gs.lcp.sickness_level != SICKNESS_LEVEL.SICKNESS_HEALTHY))
    )
    if not hunger_skip:
        gs.trigger_action = ACTION_ID.ACTION_KITCHEN_CABINET
        do_action(gs)
        gs.last_action = ACTION_ID.ACTION_KITCHEN_CABINET
        return

    # -----------------------------------------------------------------------
    # Priority 6: Scheduled meals
    # -----------------------------------------------------------------------
    if gs.lunch_meal_triggered_today == 0 and gs.lcp.lunch_hour == gs.time_hours:
        gs.trigger_action = ACTION_ID.ACTION_EAT_MEAL
        do_action(gs)
        gs.lunch_meal_triggered_today = 1
        return

    if gs.dinner_meal_triggered_today == 0 and gs.lcp.dinner_hour == gs.time_hours:
        gs.trigger_action = ACTION_ID.ACTION_EAT_MEAL
        do_action(gs)
        gs.dinner_meal_triggered_today = 1
        return

    # -----------------------------------------------------------------------
    # Priority 7: Scheduled wake / bedtime
    # -----------------------------------------------------------------------
    if gs.morning_wakeup_triggered_today == 0 and gs.lcp.wake_hour == gs.time_hours:
        gs.trigger_action = ACTION_ID.ACTION_WAKE_UP_MORNING
        do_action(gs)
        gs.morning_wakeup_triggered_today = 1
        return

    if gs.bedtime_triggered_today == 0 and gs.lcp.bedtime_hour == gs.time_hours:
        gs.trigger_action = ACTION_ID.ACTION_GO_TO_BED_NIGHT
        do_action(gs)
        gs.bedtime_triggered_today = 1
        return

    # -----------------------------------------------------------------------
    # Priority 8: Player command queue with priority escalation
    # -----------------------------------------------------------------------
    if gs.action_list_size > 0:
        front_priority = gs.action_priority_queue[0]

        if front_priority < 4:
            # Priority too low — discard this command
            _shift_command_queue(gs)
            # Fall through to random action
        elif front_priority > 7:
            # Priority high enough — execute immediately
            gs.trigger_action = gs.action_queue[0]
            if gs.action_queue[0] in (ACTION_ID.ACTION_PLAY_A_GAME,
                                       ACTION_ID.ACTION_PLAY_WITH_RECORD):
                action_get_dressed(gs)
            _shift_command_queue(gs)
            gs.action_list_size -= 1
            do_action(gs)
            return
        else:
            # Escalate priority (will execute next time if not pre-empted)
            gs.action_priority_queue[0] += 1
            # Fall through to random action this tick

    # -----------------------------------------------------------------------
    # Priority 9: Random time / mood based action
    # -----------------------------------------------------------------------
    gs.trigger_action = check_time_based_actions(gs)
    # Original: if (-1 < (short)trigger_action) — i.e. >= 0 (ACTION_NONE=0xFFFF=-1 as signed short)
    if gs.trigger_action >= 0 and gs.trigger_action != ACTION_ID.ACTION_NONE:
        do_action(gs)
