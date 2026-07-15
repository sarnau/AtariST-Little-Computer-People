/*
 * airandom.c -- time-of-day / mood-based random action selector.
 *
 * Called as the 11th and lowest-priority tier of the AI decision
 * ladder in check_for_any_action_triggers.  Picks one of three action
 * tables based on how many hours the resident has been awake, then
 * rolls a random index into that table.  Weekends bias toward the
 * relaxed table; sickness locks it to "sleep".
 *
 * Tables:
 *   g_atact[16]     -- morning / early day (0..6 h since wake)
 *   g_atmod[16]   -- midday (7..12 h since wake)
 *   g_atrel[16]    -- evening (13..17 h since wake)
 *
 * activity_schedule_table[8][3] is indexed by activity_level and by
 * (hours_since_wake / 2) % 3 to pick the effective table for the roll.
 *
 * addr: check_time_based_actions()
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
extern short    last_action;
extern short    calc_weekday();
extern short    randomRange();                  /* random.c */
extern void     check_for_any_action_triggers();/* ai.c      */
extern short    randomRange();

/* Weekday enum values used by the weekend-bias branch.  Ghidra's enum
   has SUN=0..SAT=6 but the code only tests for saturday and sunday. */
#define WEEKDAY_SUNDAY          0
#define WEEKDAY_SATURDAY        6

/* Three action tables (16 entries each) and the schedule indirection.
   Values match Ghidra activity_schedule_table[3][8] at 0x2b96e and
   g_atact/moderate/relaxed at 0x2b8fe/0x2b91e/0x2b93e. */
extern short *  activity_schedule_table[];      /* pointer array */
extern short    g_atact[];
extern short    g_atmod[];
extern short    g_atrel[];

/* check_time_based_actions: pick a random action for right now.
   Returns ACTION_NONE if the resident is sleeping and the time-of-day
   branch resolves to bedtime.
   addr: check_time_based_actions() */

short
check_time_based_actions()
{
        short   day;
        short   action_index;
        short   hours_since_wake;
        short   table_pick;

        hours_since_wake = time_hours - lcp.wake_hour;
        if (hours_since_wake < 0)
                hours_since_wake = hours_since_wake + 24;

        if (hours_since_wake < 18 && lcp.sickness_level < 2) {
                /* Indirection through the per-activity_level schedule:
                   activity_schedule_table[0] holds pointer arrays of
                   short[8], indexed via
                     activity_schedule_table[0][(activity_level << 1) +
                                                ((hours/2)%3 << 4)]
                   The << 4 (16 shorts per row) matches the 16-entry
                   action tables that follow. */
                table_pick = *((short *) activity_schedule_table[0] +
                        (lcp.activity_level << 1) +
                        (((hours_since_wake / 2) % 3) << 4));

                day = calc_weekday();
                if (table_pick == 0 && day == WEEKDAY_SUNDAY)
                        table_pick = 2;
                else if (table_pick == 0 && day == WEEKDAY_SATURDAY)
                        table_pick = 1;
        } else {
                table_pick = 3;                 /* sleep/idle */
        }

        /* Retry the roll until we get an action different from the
           last one (prevents "read newspaper" twice in a row). */
        for (;;) {
                if (table_pick == 0) {
                        action_index = randomRange(0, 15);
                        if (g_atact[action_index] != last_action)
                                return g_atact[action_index];
                } else if (table_pick == 1) {
                        action_index = randomRange(0, 15);
                        if (g_atmod[action_index] != last_action)
                                return g_atmod[action_index];
                } else if (table_pick == 2) {
                        action_index = randomRange(0, 15);
                        if (g_atrel[action_index] != last_action)
                                return g_atrel[action_index];
                } else {
                        /* Sleep bucket -- either bed or nothing. */
                        if (lcp.is_sleeping == NO)
                                return ACTION_GET_IN_OUT_OF_BED;
                        return ACTION_NONE;
                }
        }
}
