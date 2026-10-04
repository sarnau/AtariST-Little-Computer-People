/* walk.c -- LCP & dog pathfinding + step animation. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* lcp_wkD: pump lcp_path() until arrival.
   Returns 0 on arrival, -1 on preemption when idle. */


short
lcp_wkD()
{
        short   result;

        result = 0;
        g_hamod       = HEAD_ANIM_WALKING;
        g_hastl = 0;

        while (g_wtx != 0 || g_wty != 0) {
                lcp_path();
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

/* lcp_flwp -> parts/lcp_flwp.c. */

/* dg_wkPth -> parts/dg_wkPth.c. */

/* lcp_fstp -> parts/lcp_fstp.c. */


/* lcp_path -> parts/lcp_path.c. */
