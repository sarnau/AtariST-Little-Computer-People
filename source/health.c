/* health.c -- sickness onset and recovery. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "health.h"
#include "renderx.h"

/* addr: lcp_sick() */
void
lcp_sick()
{
        lcp.sickness_level      = SICKNESS_MILD;
        lcp.sickness_countdown  = 60;
        lcp.sickness_direction  = DIR_WORSENING;
        lcp.happiness_direction = DIR_WORSENING;
        if (lcp.happiness < 2)
                lcp.happiness = lcp.happiness + MOOD_CONTENT;
        lcp_upal();
}

/* addr: lcp_rcov() */
void
lcp_rcov()
{
        if (lcp.hunger_level == NEED_SATISFIED &&
            lcp.thirst_level == NEED_SATISFIED) {
                lcp.sickness_direction = DIR_IMPROVING;
                lcp.sickness_countdown = 5;
        }
}
