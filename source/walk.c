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
        g_hamod       = HEAD_ANIM_WALKING;
        g_hastl = 0;

        while (g_wtx != 0 || g_wty != 0) {
                walkStep();
                if (in_evrt != NO)
                        continue;
                if (g_trel[0] == ACTION_NONE)
                        continue;
                if (g_lcyof != NO)
                        continue;
                if (introSeq != NO)
                        continue;
                if (lcp_stR != NO)
                        continue;
                if (g_actif != NO)
                        continue;
                result = -1;
                g_wtx = 0;
                g_wty = 0;
                break;
        }
        return result;
}

/* nextWaypoint -> parts/nextWaypoint.c. */

/* dogNextWaypt -> parts/dogNextWaypt.c. */

/* playFootstep -> parts/playFootstep.c. */


/* walkStep -> parts/walkStep.c. */
