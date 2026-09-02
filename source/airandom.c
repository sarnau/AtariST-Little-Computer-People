/*
 * airandom.c -- time-of-day / mood-based random action selector.
 * addr: chk_timA()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "ai.h"
#include "airandom.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "tables.h"

#define WEEKDAY_SUNDAY          0
#define WEEKDAY_SATURDAY        6

/* addr: chk_timA() */
short
chk_timA()
{
#ifdef FAITHFUL
        short   day;
        short   action_index;
        short   hours_since_wake;
        short   table_pick;

        hours_since_wake = t_hour - lcp.wake_hour;
        if (hours_since_wake < 0)
                hours_since_wake = hours_since_wake + 24;

        if (hours_since_wake < 18 && lcp.sickness_level < 2) {
                /* Ghidra: `*(short *)((int)ptr + byte_offset)` -- the
                   `<<1` and `<<4` values ARE byte offsets, so cast the
                   base pointer to `char *` before advancing. */
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
#else
        /* STX has three locals, not four: `table_pick` doubles as the
           hours-since-wake temporary (frame -10), and the retry is an
           explicit label rather than a loop -- the last arm carries no
           branch back to the top. */
        short   table_pick;
        short   action_index;
        short   day;

        table_pick = t_hour - lcp.wake_hour;
        if (table_pick < 0)
                table_pick += 24;

        if (table_pick >= 18 || lcp.sickness_level >= 2) {
                table_pick = 3;                 /* sleep/idle */
        } else {
                /* sch_tab is a real 2-D array here -- the shifted
                   row/column offsets, the two ext.l and the trailing
                   `add.l #base` are exactly Alcyon's `a[i][j]`. */
                table_pick = (table_pick / 2) % 3;
                table_pick = sch_tab[table_pick][lcp.activity_level];

                day = cWkday();
                if (table_pick == 0 && day == WEEKDAY_SUNDAY)
                        table_pick = 2;
                else if (table_pick == 0 && day == WEEKDAY_SATURDAY)
                        table_pick = 1;
        }

        /* The three table arms carry NO return statement: each one
           simply ends, and the ladder's jump to the function end
           leaves the freshly-compared value sitting in d0.  Only the
           sleep arm returns explicitly, and it does so with a real
           `else`. */
retry:
        if (table_pick == 0) {
                action_index = g_atact[rndRng(0, 15)];
                if (action_index == lastAct)
                        goto retry;
        } else if (table_pick == 1) {
                action_index = g_atmod[rndRng(0, 15)];
                if (action_index == lastAct)
                        goto retry;
        } else if (table_pick == 2) {
                action_index = g_atrel[rndRng(0, 15)];
                if (action_index == lastAct)
                        goto retry;
        } else {
                /* Sleep bucket -- either bed or nothing. */
                if (lcp.is_sleeping == NO)
                        return ACTION_GET_IN_OUT_OF_BED;
                else
                        return ACTION_NONE;
        }
#endif
}
