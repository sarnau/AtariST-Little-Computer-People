/*
 * walk.c -- LCP & dog pathfinding + step animation.
 * addr: lcp_wkD(), lcp_path(), lcp_flwp(), dg_wkPth(), lcp_fstp()
 */

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
   Returns 0 on arrival, -1 on preemption when idle.
   addr: lcp_wkD() */

#ifdef FAITHFUL
short
lcp_wkD()
{
        g_hamod       = HEAD_ANIM_WALKING;
        g_hastl = 0;

        do {
                if (g_wtx == 0 && g_wty == 0)
                        return 0;
                lcp_path();
        } while (in_evrt != NO ||
                 g_trel[0] == ACTION_NONE ||
                 g_lcyof != NO ||
                 introSeq != NO ||
                 lcp_stR != NO ||
                 g_actif != NO);

        g_wty = 0;
        g_wtx = 0;
        return -1;
}
#else   /* STX: link #-6 -- the return value goes through a local,
           the loop is a while on the target coordinates, and each
           guard is its own continue. */

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
#endif

/* lcp_flwp -> parts/lcp_flwp.c (STX: 0x50bc, just before getFlrY). */
#ifdef FAITHFUL
#include "parts/lcp_flwp.c"
#endif

/* dg_wkPth -> parts/dg_wkPth.c (STX: 0x4586, immediately after dg_mvAni). */
#ifdef FAITHFUL
#include "parts/dg_wkPth.c"
#endif

/* lcp_fstp -> parts/lcp_fstp.c (STX: 0x4fec, in the 0x400c object with getFlrY). */
#ifdef FAITHFUL
#include "parts/lcp_fstp.c"
#endif

#ifdef FAITHFUL
#endif

#ifdef FAITHFUL
#endif

#ifdef FAITHFUL
#endif

/* lcp_path -> parts/lcp_path.c (STX: 0x470a, in the 0x400c object). */
#ifdef FAITHFUL
#include "parts/lcp_path.c"
#endif
