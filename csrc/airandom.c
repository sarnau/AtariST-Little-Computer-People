/*
 * airandom.c -- time-of-day / mood-based random action selector.
 *
 * Called as the 11th and lowest-priority tier of the AI decision
 * ladder in chk_actT.  Picks one of three action
 * tables based on how many hours the resident has been awake, then
 * rolls a random index into that table.  Weekends bias toward the
 * relaxed table; sickness locks it to "sleep".
 *
 * Tables:
 *   g_atact[16]     -- morning / early day (0..6 h since wake)
 *   g_atmod[16]   -- midday (7..12 h since wake)
 *   g_atrel[16]    -- evening (13..17 h since wake)
 *
 * sch_tab[8][3] is indexed by activity_level and by
 * (hours_since_wake / 2) % 3 to pick the effective table for the roll.
 *
 * addr: chk_timA()
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
extern short    lastAct;
extern short    cWkday();
extern short    rndRng();                  /* random.c */
extern void     chk_actT();/* ai.c      */
extern short    rndRng();

/* Weekday enum values used by the weekend-bias branch.  Ghidra's enum
   has SUN=0..SAT=6 but the code only tests for saturday and sunday. */
#define WEEKDAY_SUNDAY          0
#define WEEKDAY_SATURDAY        6

/* Three action tables (16 entries each) and the schedule indirection.
   Values match Ghidra sch_tab[3][8] at 0x2b96e and
   g_atact/moderate/relaxed at 0x2b8fe/0x2b91e/0x2b93e. */
extern short *  sch_tab[];      /* pointer array */
extern short    g_atact[];
extern short    g_atmod[];
extern short    g_atrel[];

/* chk_timA: pick a random action for right now.
   Returns ACTION_NONE if the resident is sleeping and the time-of-day
   branch resolves to bedtime.
   addr: chk_timA() */

short
chk_timA()
{
        short   day;
        short   action_index;
        short   hours_since_wake;
        short   table_pick;

        hours_since_wake = t_hour - lcp.wake_hour;
        if (hours_since_wake < 0)
                hours_since_wake = hours_since_wake + 24;

        if (hours_since_wake < 18 && lcp.sickness_level < 2) {
                /* Indirection through the per-activity_level schedule:
                   sch_tab[0] holds pointer arrays of
                   short[8], indexed via
                     sch_tab[0][(activity_level << 1) +
                                                ((hours/2)%3 << 4)]
                   The << 4 (16 shorts per row) matches the 16-entry
                   action tables that follow. */
                /* Ghidra: `*(short *)((int)ptr + byte_offset)` -- the
                   `<<1` and `<<4` values ARE byte offsets, so we cast
                   the base pointer to `char *` before advancing, then
                   cast to `short *` for the dereference.  Our older
                   port had `(short *)ptr + N` which scales N by 2
                   (short-pointer arithmetic), reading twice as far
                   into the row and hitting garbage past _schedule_row_0's
                   8-short bound. */
                table_pick = *(short *) ((char *) sch_tab[0] +
                        (lcp.activity_level << 1) +
                        (((hours_since_wake / 2) % 3) << 4));

                day = cWkday();
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
                        action_index = rndRng(0, 15);
                        if (g_atact[action_index] != lastAct)
                                return g_atact[action_index];
                } else if (table_pick == 1) {
                        action_index = rndRng(0, 15);
                        if (g_atmod[action_index] != lastAct)
                                return g_atmod[action_index];
                } else if (table_pick == 2) {
                        action_index = rndRng(0, 15);
                        if (g_atrel[action_index] != lastAct)
                                return g_atrel[action_index];
                } else {
                        /* Sleep bucket -- either bed or nothing. */
                        if (lcp.is_sleeping == NO)
                                return ACTION_GET_IN_OUT_OF_BED;
                        return ACTION_NONE;
                }
        }
}
