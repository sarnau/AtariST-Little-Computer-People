/* health.c -- sickness onset and recovery. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "protos.h"

/* Make the resident sick: called by simStep when a need timer expires
   while thirst or hunger is already at its worst.  Sets him mildly
   sick and worsening, starts the worsening countdown, turns his mood
   downwards (one step sadder unless already sad) and recolours his
   skin through setSkinColor. */
void
fallSick()
{
        resident.sickness_level      = SICKNESS_MILD;
        resident.sickness_countdown  = SICK_DELAY_WORSENING;
        resident.sickness_direction  = DIR_WORSENING;
        resident.happiness_direction = DIR_WORSENING;
        if (resident.happiness < MOOD_SAD)
                /* One mood step sadder (HAPPY -> CONTENT -> SAD).  Written
                   `+= 1` on purpose: `x = x + 1` compiles differently. */
                resident.happiness += 1;
        setSkinColor();
}

/* Start recovery once the resident has eaten and drunk: called after
   a drink and after a meal, it turns the sickness direction to
   improving (with the improving countdown) only when both hunger and
   thirst are fully satisfied.  The level itself is stepped later by
   simStep. */
void
startRecovery()
{
        if (resident.hunger_level == NEED_SATISFIED &&
            resident.thirst_level == NEED_SATISFIED) {
                resident.sickness_direction = DIR_IMPROVING;
                resident.sickness_countdown = SICK_DELAY_IMPROVING;
        }
}

/* setSkinColor must stay here, after startRecovery and in the same unit as
   fallSick, which calls it. */
void
setSkinColor()
{
        if (resident.sickness_level == SICKNESS_HEALTHY)
                mainPalette[6] = ST_PEACH;
        else
                mainPalette[6] = ST_SICK_GREEN;
        Setpalette(mainPalette);
}
