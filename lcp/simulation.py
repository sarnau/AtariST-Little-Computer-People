"""
Time simulation for Little Computer People (Atari ST).
Translated from Ghidra decompilation of game_simulate_one_second().

addr: game_simulate_one_second at (LCP.PRG TEXT segment)

Called once per frame in game_tick_and_animate(). Acts only when
animation_tick_counter & 7 == 0 (i.e., every 8 frames ≈ 1 game-second).

Per game-second:
  - Increment game_seconds_counter (0→59)
  - On minute rollover (60 seconds): decrement need timers, sickness logic,
    bathroom timer, phone call randomness, happiness cycling, clock advance
  - Per hour rollover: happiness duration countdown
  - Per day rollover (24 hours): reset daily flags, advance calendar
"""

import random
from .state import GameState
from .enums import (
    SICKNESS_LEVEL, HAPPINESS_LEVEL, NEED_LEVEL, ACTION_ID,
)
from .constants import DAYS_IN_MONTH, PHONE_CALL_PROBABILITY


# ---------------------------------------------------------------------------
# Direction constants (used as +1 / -1 in sickness and happiness fields)
# addr: DIR_WORSENING, DIR_IMPROVING
# ---------------------------------------------------------------------------
DIR_IMPROVING = -1   # sickness_level / happiness decreases each countdown tick
DIR_WORSENING = +1   # sickness_level / happiness increases each countdown tick

# Sickness countdown values (game-minutes between level changes)
SICKNESS_RECOVER_COUNTDOWN = 5    # minutes between recovery steps  (DIR_IMPROVING)
SICKNESS_WORSEN_COUNTDOWN  = 60   # minutes between worsening steps (DIR_WORSENING)

# Happiness duration fallback (used if struct fields not initialised)
HAPPINESS_DURATION_BY_LEVEL = [12, 8, 6]   # HAPPY, CONTENT, SAD (in game-hours)


def random_range(lo: int, hi: int) -> int:
    """
    Port of randomRange(lo, hi) — uniform integer in [lo, hi].
    addr: randomRange() utility function
    """
    return random.randint(lo, hi)


def lcp_become_sick(gs: GameState) -> None:
    """
    Set sickness to MILD with 60-minute worsening countdown.
    Also nudges happiness toward SAD and sets happiness direction to worsening.
    addr: lcp_become_sick()
    """
    gs.lcp.sickness_level     = SICKNESS_LEVEL.SICKNESS_LEVEL_1
    gs.lcp.sickness_countdown = SICKNESS_WORSEN_COUNTDOWN
    gs.lcp.sickness_direction = DIR_WORSENING
    gs.lcp.happiness_direction = DIR_WORSENING
    if gs.lcp.happiness < HAPPINESS_LEVEL.MOOD_SAD:   # < 2
        gs.lcp.happiness += 1
    lcp_update_palette_colors(gs)


def lcp_update_palette_colors(gs: GameState) -> None:
    """
    Update palette entry 6 (skin colour) based on sickness level.
    When healthy → peach/normal skin; when sick → green tint.
    addr: lcp_update_palette_colors()
    """
    from .constants import ST_PEACH, ST_SICK_GREEN
    if gs.lcp.sickness_level == SICKNESS_LEVEL.SICKNESS_HEALTHY:
        gs.palette_skin_color = ST_PEACH
    else:
        gs.palette_skin_color = ST_SICK_GREEN


def put_event_to_list(gs: GameState, event: int) -> None:
    """
    Append an ACTION_ID event to the triggered event queue.
    addr: put_event_to_list()
    """
    for i in range(len(gs.triggered_event_list)):
        if gs.triggered_event_list[i] == ACTION_ID.ACTION_NONE:
            gs.triggered_event_list[i] = event
            return


def daily_reset_action_flags(gs: GameState) -> None:
    """
    Reset all daily-once action flags at midnight.
    addr: daily_reset_action_flags()
    """
    gs.reset_daily_flags()


def days_in_month(month: int, year: int) -> int:
    """
    Return the number of days in a given month (0-based) and year.
    addr: days_in_month()
    """
    days = DAYS_IN_MONTH[month % 12]
    if month % 12 == 1:   # February — simple leap year rule
        if (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0):
            days = 29
    return days


def _get_happiness_duration(gs: GameState) -> int:
    """
    Read the happiness duration for the current happiness level from the LCP struct.
    Original code: *(short *)(lcp.owner_name + happiness * 2 - 0x34)
    owner_name is at struct offset 0x5E, so:
      happiness=0 (happy)   → offset 0x5E + 0 - 0x34 = 0x2A → happiness_initial_countdown
      happiness=1 (content) → offset 0x5E + 2 - 0x34 = 0x2C → happiness_duration_happy
      happiness=2 (sad)     → offset 0x5E + 4 - 0x34 = 0x2E → happiness_duration_content
    These are per-character values set during lcp_create_random().
    addr: game_simulate_one_second() happiness reload
    """
    level = int(gs.lcp.happiness)
    if level == 0:
        return gs.lcp.happiness_initial_countdown or HAPPINESS_DURATION_BY_LEVEL[0]
    elif level == 1:
        return gs.lcp.happiness_duration_happy or HAPPINESS_DURATION_BY_LEVEL[1]
    else:
        return gs.lcp.happiness_duration_content or HAPPINESS_DURATION_BY_LEVEL[2]


# ---------------------------------------------------------------------------
# ACTION_EVENT_PHONE_CALL — event ID used for phone call trigger
# addr: ACTION_EVENT_PHONE_CALL constant (mapped to triggered_event_list)
# ---------------------------------------------------------------------------
ACTION_EVENT_PHONE_CALL = ACTION_ID.ACTION_EVENT_PHONE_CALL


def game_simulate_one_second(gs: GameState) -> None:
    """
    Simulate one game-second.  Must be called every frame; it gate-checks
    animation_tick_counter & 7 == 0 internally (same as the original C).

    addr: game_simulate_one_second()
    """
    # Gate: only act on every 8th frame tick (== one game-second)
    if (gs.animation_tick_counter & 7) != 0:
        return

    gs.game_seconds_counter += 1
    if gs.game_seconds_counter != 60:
        return

    # ---- Game-minute boundary ----
    gs.game_seconds_counter = 0

    # -- Thirst timer --
    gs.lcp.thirst_timer -= 1
    if gs.lcp.thirst_timer < 1:
        gs.lcp.thirst_timer = gs.lcp.thirst_timer_max
        if gs.lcp.thirst_level < 3:
            gs.lcp.thirst_level += NEED_LEVEL.NEED_MILD   # +1
        else:
            lcp_become_sick(gs)

    # -- Hunger timer --
    gs.lcp.hunger_timer -= 1
    if gs.lcp.hunger_timer < 1:
        gs.lcp.hunger_timer = gs.lcp.hunger_timer_max
        if gs.lcp.hunger_level < 3:
            gs.lcp.hunger_level += NEED_LEVEL.NEED_MILD   # +1
        else:
            lcp_become_sick(gs)

    # -- Sickness progression --
    if gs.lcp.sickness_level > SICKNESS_LEVEL.SICKNESS_HEALTHY:
        gs.lcp.sickness_countdown -= 1
        if gs.lcp.sickness_countdown == 0:
            gs.lcp.sickness_level += gs.lcp.sickness_direction
            if gs.lcp.sickness_level == SICKNESS_LEVEL.SICKNESS_HEALTHY:
                lcp_update_palette_colors(gs)
            if gs.lcp.sickness_level > SICKNESS_LEVEL.SICKNESS_LEVEL_1:
                gs.lcp.happiness = HAPPINESS_LEVEL.MOOD_SAD
            if gs.lcp.sickness_direction == DIR_IMPROVING:
                gs.lcp.sickness_countdown = SICKNESS_RECOVER_COUNTDOWN
            else:
                gs.lcp.sickness_countdown = SICKNESS_WORSEN_COUNTDOWN

    # -- Bathroom timer --
    gs.lcp.bathroom_timer -= 1
    if gs.lcp.bathroom_timer < 1:
        gs.lcp.bathroom_timer = 9999
        gs.lcp.bathroom_need = 1   # YES

    # -- Phone call (2% chance, 8am–10pm, only when not already active) --
    if (7 < gs.time_hours < 22):
        rnd = random_range(0, 100)
        if rnd < 2:
            if gs.phone_answered_flag == 0 and gs.intro_sequence_active == 0:
                gs.phone_call_active_flag = 1
                put_event_to_list(gs, ACTION_EVENT_PHONE_CALL)

    # -- Clock: advance minute --
    gs.time_minutes += 1
    if gs.time_minutes != 60:
        return

    # ---- Game-hour boundary ----
    gs.time_minutes = 0

    # -- Happiness mood cycle (once per hour) --
    # Only update if healthy or not already sad
    if not (gs.lcp.sickness_level == SICKNESS_LEVEL.SICKNESS_HEALTHY
            or gs.lcp.happiness != HAPPINESS_LEVEL.MOOD_SAD):
        pass   # sickness overrides happiness update
    else:
        gs.lcp.happiness_duration_active -= 1
        if gs.lcp.happiness_duration_active == 0:
            gs.lcp.happiness += gs.lcp.happiness_direction
            if gs.lcp.happiness < HAPPINESS_LEVEL.MOOD_CONTENT:   # < 1
                gs.lcp.happiness = HAPPINESS_LEVEL.MOOD_HAPPY
                gs.lcp.happiness_direction = DIR_WORSENING
            elif gs.lcp.happiness > HAPPINESS_LEVEL.MOOD_CONTENT:  # > 1
                gs.lcp.happiness = HAPPINESS_LEVEL.MOOD_SAD
                gs.lcp.happiness_direction = DIR_IMPROVING
            gs.lcp.happiness_duration_active = _get_happiness_duration(gs)

    # -- Clock: advance hour --
    gs.time_hours += 1
    if gs.time_hours != 24:
        return

    # ---- Game-day boundary (midnight) ----
    gs.time_hours = 0
    daily_reset_action_flags(gs)

    days_this_month = days_in_month(gs.date_month, gs.date_year)
    gs.date_day += 1
    if gs.date_day == days_this_month:
        gs.date_day = 0
        gs.date_month += 1
        if gs.date_month == 12:
            gs.date_month = 0
            gs.date_year += 1
