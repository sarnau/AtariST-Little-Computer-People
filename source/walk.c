/* walk.c -- LCP & dog pathfinding + step animation. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "events.h"
#include "globals.h"
#include "protos.h"
#include "sprglobs.h"
#include "sprites.h"

/* walkToTarget: pump walkStep() until arrival.
   Returns 0 on arrival, -1 on preemption when idle. */

short
walkToTarget()
{
        short   result;

        result = 0;
        headMode       = HEAD_ANIM_WALKING;
        headLastWalk = 0;

        while (walkXTarget != 0 || walkYTarget != 0) {
                walkStep();
                if (inEvent != NO)
                        continue;
                if (eventQueue[0] == ACTION_NONE)
                        continue;
                if (isCarrying != NO)
                        continue;
                if (movingIn != NO)
                        continue;
                if (onStairs != NO)
                        continue;
                if (noPreempt != NO)
                        continue;
                result = -1;
                walkXTarget = 0;
                walkYTarget = 0;
                break;
        }
        return result;
}

