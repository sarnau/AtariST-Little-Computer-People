/* health.c -- sickness onset and recovery. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "health.h"
#include "renderx.h"

/* Make the resident sick: called by gameSim1 when a need timer expires
   while thirst or hunger is already at its worst.  Sets him mildly
   sick and worsening, starts the worsening countdown, turns his mood
   downwards (one step sadder unless already sad) and recolours his
   skin through lcp_upal. */
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

/* Start recovery once the resident has eaten and drunk: called after
   a drink and after a meal, it turns the sickness direction to
   improving (with the improving countdown) only when both hunger and
   thirst are fully satisfied.  The level itself is stepped later by
   gameSim1. */
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
