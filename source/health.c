/* health.c -- sickness onset and recovery. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "health.h"
#include "renderx.h"

void
lcp_sick()
{
        lcp.sickness_level      = SICKNESS_MILD;
        lcp.sickness_countdown  = SICK_DELAY_WORSENING;
        lcp.sickness_direction  = DIR_WORSENING;
        lcp.happiness_direction = DIR_WORSENING;
        if (lcp.happiness < MOOD_SAD)
                /* One mood step sadder (HAPPY -> CONTENT -> SAD).  Written
                   `+= 1` on purpose: `x = x + 1` compiles differently. */
                lcp.happiness += 1;
        lcp_upal();
}

void
lcp_rcov()
{
        if (lcp.hunger_level == NEED_SATISFIED &&
            lcp.thirst_level == NEED_SATISFIED) {
                lcp.sickness_direction = DIR_IMPROVING;
                lcp.sickness_countdown = SICK_DELAY_IMPROVING;
        }
}

/* lcp_upal must stay here, after lcp_rcov and in the same unit as
   lcp_sick, which calls it. */
void
lcp_upal()
{
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                main_pal[6] = ST_PEACH;
        else
                main_pal[6] = ST_SICK_GREEN;
        Setpalette(main_pal);
}
