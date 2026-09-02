/*
 * abathrm.c -- hygiene handlers (bathroom-sink / shower).
 * addr: a_takes(), a_brust(), a_washh()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "abathrm.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_takes: shower.  20..25 scrub/wash cycles.  HEAD_ANIM_SHOWER bobs L/R.
   addr: a_takes() */

void
a_takes()
{
        short   result;
        short   count;
        short   pick;

        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        hs_posXY(POS_MID_SHOWER_INSIDE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_SHOWER_STAND;
        lcp_x = lcp_x - 8;
        lcp_y = lcp_y - 23;
        g_hatas = 8;
        lcp_hwt();
        g_hamod = HEAD_ANIM_SHOWER;

        count = rndRng(20, 25);
        while (count != 0) {
                pick = rndRng(0, 1);
                if (pick == 0) {
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                } else {
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                }
                count = count - 1;
        }

        lcp_st = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 29;
        gameTick(2);
        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        g_hamod = HEAD_ANIM_DISABLED;
        g_actif = NO;
}

/* a_brust: 24..35 tooth-brush cycles.  Reuses SPRITE_STUDY_DOOR_FRAME
   (id 6) as the brush overlay above the head.
   addr: a_brust() */

void
a_brust()
{
        unsigned short  brush_cycles;
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) rndRng(24, 35);
        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BRUSH_TEETH;
        g_hatas = 10;
#ifdef FAITHFUL
        lcp_y = lcp_y - 2;
#else
        lcp_y -= 2;
#endif
        lcp_hwt();

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        sp_sprs(SPRITE_STUDY_DOOR_FRAME);
        x_left  = lcp_x + 8;
        x_right = lcp_x + 12;
        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        g_sepey[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = lcp_y - 24;

        while (brush_cycles != 0) {
                if (((brush_cycles - 1) & 1) == 0)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                gameTick(0);
                brush_cycles = brush_cycles - 1;
        }

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        sp_upds();
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 2;
        gameTick(0);
}

/* a_washh: sink + water + 4..127 random wash cycles picking
   from 3 hand-position states.  Stops water on any interruption.
   addr: a_washh() */

void
a_washh()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
        unsigned short  rnd;
        unsigned short  val;
        unsigned short  last_pick;
        short           counter;

        pst_arr[0] = STATE_WASH_HANDS_CENTER;
        pst_arr[1] = STATE_WASH_HANDS_LEFT;
        pst_arr[2] = STATE_WASH_HANDS_RIGHT;

        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

#ifdef FAITHFUL
        rnd = (unsigned short) Random();
#else
        rnd = (unsigned short)(Random() & 0x7f) | 4;
#endif
        sf_sele(SFX_WATER_RUNNING, 10000L);

        counter   = 0;
        last_pick = 0;
        while (counter < (short) ((rnd & 0x7f) | 4) &&
               g_trel[0] == ACTION_NONE) {
                val = (unsigned short) Random();
                while ((val & 3) == last_pick)
                        val = (unsigned short) Random();
                val = val & 3;
                last_pick = val;
                if (val == 3)
                        lcp_st = pst_arr[1];
                else
                        lcp_st = pst_arr[val];
                lcp_face = (val == 3) ? FACING_LEFT : FACING_RIGHT;
                gameTick(1);
                counter = counter + 1;
        }

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_driwa: fill / drink a glass (carried_object
   pre-selected by the caller).  Runs the same 3-position hand-shift
   loop as a_washh but scoped to lower amplitudes (bit 0x1f
   instead of 0x7f), so it plays for ~4..35 ticks instead of ~4..127.
   The `value` argument is the SPRITE_ID of the object being carried
   (typically SPRITE_GLASS).
   addr: a_driwa() */

void
a_driwa(value)
short   value;
{
        /* STX declares them rnd, counter, last_pick, pick -- the
           frame offsets follow that order. */
#ifdef FAITHFUL
        unsigned short  rnd;
        unsigned short  pick;
        unsigned short  last_pick;
        short           counter;
#else
        short           rnd;
        short           counter;
        short           last_pick;
        short           pick;
#endif

        pst_arr[0] = STATE_WASH_HANDS_CENTER;
        pst_arr[1] = STATE_WASH_HANDS_LEFT;
        pst_arr[2] = STATE_WASH_HANDS_RIGHT;

        sp_ssco(value);
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();
        g_selaf[value] = SPRITE_HIDDEN;
        sp_upds();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        /* STX folds the mask into the assignment (computed once). */
#ifdef FAITHFUL
        rnd = (unsigned short) Random();
#else
        rnd = (unsigned short)(Random() & 0x1f) | 4;
#endif
        sf_sele(SFX_WATER_RUNNING, 10000L);

#ifdef FAITHFUL
        last_pick = 0;
        for (counter = 0;
             counter < (short) ((rnd & 0x1f) | 4);
             counter = counter + 1) {
                pick = (unsigned short) Random();
                while ((pick & 3) == last_pick)
                        pick = (unsigned short) Random();
                pick = pick & 3;
                last_pick = pick;
                if (pick == 3)
                        lcp_st = pst_arr[1];
                else
                        lcp_st = pst_arr[pick];
                lcp_face = (pick == 3) ? FACING_LEFT : FACING_RIGHT;
                gameTick(1);
        }
#else
        /* STX masks at the assignment and never initialises
           last_pick -- the first comparison reads whatever the frame
           slot held.  Preserved as the original wrote it. */
        for (counter = 0; counter < rnd; counter++) {
                pick = Random() & 3;
                while (pick == last_pick)
                        pick = Random() & 3;
                last_pick = pick;
                if (pick != 3) {
                        lcp_st = pst_arr[pick];
                        lcp_face = FACING_RIGHT;
                } else {
                        lcp_st = pst_arr[1];
                        lcp_face = FACING_LEFT;
                }
                gameTick(1);
        }
#endif

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
