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

/* lcp_flwp: pick next waypoint.  Same-floor -> straight to g_wtx/y;
   cross-floor -> through stair_wp[].  Middle floor has an extra
   stair_ty/stair_by landing branch top/bottom don't need.
   addr: lcp_flwp() */

void
lcp_flwp()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = getFlrY(g_wty);
        current_floor = getFlrY(lcp_y);

        if (current_floor == target_floor) {
                lcp_stR = NO;
                g_wyx = g_wtx;
                g_wyy = g_wty;
                return;
        }

        target_floor    = getFlrY(lcp_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_wyx = stair_wp[stair_index];
        g_wyy = stair_wp[current_floor + 1];

        target_floor = getFlrY(lcp_y);
        if (target_floor == 2) {
                target_floor = getFlrY(g_wty);
                dest_floor   = getFlrY(lcp_y);
                if (target_floor < dest_floor) {
                        g_wyx = stair_ty;
                        g_wyy = stair_by;
                }
        }

        lcp_stR = NO;
        if (lcp_x == g_wyx && lcp_y == g_wyy) {
                lcp_stR = YES;
                if (g_wty < lcp_y) {
                        g_wyx = stair_wp[current_floor + 2];
                        g_wyy = stair_wp[current_floor + 3];
                } else {
                        g_wyy = stair_wp[current_floor - 1];
                        g_wyx = stair_wp[current_floor - 2];
                }
                target_floor = getFlrY(lcp_y);
                if (target_floor == 1) {
                        g_wyx = stair_ty;
                        g_wyy = stair_by;
                }
        }
}

/* dg_wkPth: dog waypoint math.  Same shape as lcp_flwp but uses
   dog_x/y and applies -3 X on middle-floor landing + -8 X on stair
   crest.
   addr: dg_wkPth() */

void
dg_wkPth()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = getFlrY(g_dty);
        current_floor = getFlrY(dog_y);

        if (current_floor == target_floor) {
                dg_stair = NO;
                g_dyx = g_dtx;
                g_dyy = g_dty;
                return;
        }

        target_floor    = getFlrY(dog_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_dyx  = stair_wp[stair_index];
        g_dyy  = stair_wp[current_floor + 1];

        target_floor = getFlrY(dog_y);
        if (target_floor == 2) {
                target_floor = getFlrY(g_dty);
                dest_floor   = getFlrY(dog_y);
                if (target_floor < dest_floor) {
                        g_dyx = stair_ty - 3;
                        g_dyy = stair_by;
                }
        }

        dg_stair = NO;
        if (dog_x == g_dyx && dog_y == g_dyy) {
                target_floor = getFlrY(dog_y);
                if (target_floor == 3)
                        dog_x = dog_x - 8;
                dg_stair = YES;
                if (g_dty < dog_y) {
                        g_dyx = stair_wp[current_floor + 2];
                        g_dyy = stair_wp[current_floor + 3];
                } else {
                        g_dyy = stair_wp[current_floor - 1];
                        g_dyx = stair_wp[current_floor - 2];
                }
                target_floor = getFlrY(dog_y);
                if (target_floor == 1) {
                        g_dyx = stair_ty;
                        g_dyy = stair_by;
                }
        }
}

/* lcp_fstp -> parts/lcp_fstp.c (STX: 0x4fec, in the 0x400c object with getFlrY). */
#ifdef FAITHFUL
#include "parts/lcp_fstp.c"
#endif

/* wkCyc -> parts/wkCyc.c (STX: 0x400c object, with lcp_path). */
#ifdef FAITHFUL
#include "parts/wkCyc.c"
#endif

/* stairCyc -> parts/stairCyc.c (STX: 0x400c object, with lcp_path). */
#ifdef FAITHFUL
#include "parts/stairCyc.c"
#endif

/* setHTgt -> parts/setHTgt.c (STX: 0x400c object, with lcp_path). */
#ifdef FAITHFUL
#include "parts/setHTgt.c"
#endif

/* lcp_path -> parts/lcp_path.c (STX: 0x470a, in the 0x400c object). */
#ifdef FAITHFUL
#include "parts/lcp_path.c"
#endif
