/*
 * airandom.c -- time-of-day / mood-based random action selector.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "protos.h"
#include "globals.h"
#include "tables.h"

#define WEEKDAY_SUNDAY          0
#define WEEKDAY_SATURDAY        6

/* Pick a random idle action for the resident.  The hours since he
   woke (and his sickness) select an activity tier: sleep after 18
   hours awake or when moderately sick, otherwise scheduleTiers indexed by
   two-hour slot and his activity level, softened on weekends.  The
   ACTIVE/MODERATE/RELAXED tiers draw one of 16 entries from
   activeActions/moderateActions/relaxedActions, re-rolling while it equals lastAction; the
   sleep tier returns GET_IN_OUT_OF_BED if he is awake, else
   ACTION_NONE.  chooseAction stores the result in nextAction. */
short
pickIdleAction()
{
        /* Three locals, not four: `tablePick` doubles as the
           hours-since-wake temporary.  The retry is an explicit label
           rather than a loop, as in the original -- the last arm
           carries no branch back to the top. */
        short   tablePick;
        short   actionIndex;
        short   day;

        tablePick = t_hour - resident.wakeHour;
        if (tablePick < 0)
                tablePick += 24;

        if (tablePick >= 18 || resident.sicknessLevel >= SICKNESS_MODERATE) {
                tablePick = TIER_SLEEP;
        } else {
                /* scheduleTiers must stay a real 2-D array: a table of row
                   pointers compiles to different code. */
                tablePick = (tablePick / 2) % 3;
                tablePick = scheduleTiers[tablePick][resident.activityLevel];

                day = calcWeekday();
                if (tablePick == TIER_ACTIVE && day == WEEKDAY_SUNDAY)
                        tablePick = TIER_RELAXED;
                else if (tablePick == TIER_ACTIVE && day == WEEKDAY_SATURDAY)
                        tablePick = TIER_MODERATE;
        }

        /* The three table arms deliberately have NO return statement:
           each one simply ends, and the function returns whatever the
           last comparison left in the return register.  Only the sleep
           arm returns explicitly, with a real `else`.  Do not add the
           missing returns -- they change the compiled code. */
retry:
        if (tablePick == TIER_ACTIVE) {
                actionIndex = activeActions[rndRng(0, 15)];
                if (actionIndex == lastAction)
                        goto retry;
        } else if (tablePick == TIER_MODERATE) {
                actionIndex = moderateActions[rndRng(0, 15)];
                if (actionIndex == lastAction)
                        goto retry;
        } else if (tablePick == TIER_RELAXED) {
                actionIndex = relaxedActions[rndRng(0, 15)];
                if (actionIndex == lastAction)
                        goto retry;
        } else {
                /* Sleep bucket -- either bed or nothing. */
                if (resident.isSleeping == NO)
                        return ACTION_GET_IN_OUT_OF_BED;
                else
                        return ACTION_NONE;
        }
}
