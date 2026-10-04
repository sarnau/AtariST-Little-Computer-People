/*
 * airandom.c -- time-of-day / mood-based random action selector.
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

short
chk_timA()
{
        /* Three locals, not four: `table_pick` doubles as the
           hours-since-wake temporary.  The retry is an explicit label
           rather than a loop, as in the original -- the last arm
           carries no branch back to the top. */
        short   table_pick;
        short   action_index;
        short   day;

        table_pick = t_hour - lcp.wake_hour;
        if (table_pick < 0)
                table_pick += 24;

        if (table_pick >= 18 || lcp.sickness_level >= SICKNESS_MODERATE) {
                table_pick = TIER_SLEEP;
        } else {
                /* sch_tab must stay a real 2-D array: a table of row
                   pointers compiles to different code. */
                table_pick = (table_pick / 2) % 3;
                table_pick = sch_tab[table_pick][lcp.activity_level];

                day = cWkday();
                if (table_pick == TIER_ACTIVE && day == WEEKDAY_SUNDAY)
                        table_pick = TIER_RELAXED;
                else if (table_pick == TIER_ACTIVE && day == WEEKDAY_SATURDAY)
                        table_pick = TIER_MODERATE;
        }

        /* The three table arms deliberately have NO return statement:
           each one simply ends, and the function returns whatever the
           last comparison left in the return register.  Only the sleep
           arm returns explicitly, with a real `else`.  Do not add the
           missing returns -- they change the compiled code. */
retry:
        if (table_pick == TIER_ACTIVE) {
                action_index = g_atact[rndRng(0, 15)];
                if (action_index == lastAct)
                        goto retry;
        } else if (table_pick == TIER_MODERATE) {
                action_index = g_atmod[rndRng(0, 15)];
                if (action_index == lastAct)
                        goto retry;
        } else if (table_pick == TIER_RELAXED) {
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
}
