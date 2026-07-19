/*
 * health.c -- sickness onset.
 *
 * lcp_sick() is called from sim.c when thirst or hunger reaches
 * the critical level (3+).  Kicks the resident into SICKNESS_MILD with
 * a 60-minute worsening timer, and nudges happiness one step toward SAD
 * so the mood cycle drifts in that direction until treated.  Palette
 * update is delegated to render.c so this file stays hardware-free.
 *
 * addr: lcp_sick()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "health.h"
#include "renderx.h"
/* lcp_sick: transition from healthy -> mildly sick.  Sets the
   60-minute worsening countdown and nudges the mood one step sad.
   Palette refresh happens via lcp_upal() in render.c.

   addr: lcp_sick() */

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

/* lcp_rcov: if both hunger and thirst are back to 0, flip
   sickness_direction to improving and set a 5-minute countdown so the
   sickness ticks its way back down to healthy over the next few game
   minutes.  Called after ACTION_DRINK, ACTION_KITCHEN_CABINET, and
   ACTION_EAT_MEAL.
   addr: lcp_rcov() */

void
lcp_rcov()
{
        if (lcp.hunger_level == NEED_SATISFIED &&
            lcp.thirst_level == NEED_SATISFIED) {
                lcp.sickness_direction = DIR_IMPROVING;
                lcp.sickness_countdown = 5;
        }
}
