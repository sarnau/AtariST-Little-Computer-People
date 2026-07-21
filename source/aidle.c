/*
 * aidle.c -- short "no-walk" idle / gesture handlers.
 *
 * All share the same shape: pick a pair of animation states, tick
 * through them for a short duration, return to STATE_STAND_SIDE_VIEW.
 * No walking, no world state mutation, no sound (except toggle_tv).
 *
 * addr: a_wandi(), a_peeka(),
 *       a_pacen(), a_toggt(), a_sleep()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "aidle.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* a_wandi: two-state shrug idle.
   addr: a_wandi() */

void
a_wandi()
{
        pst_arr[0]  = STATE_IDLE_SHRUG_START;
        pst_arr[1]  = STATE_IDLE_SHRUG_HOLD;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = pst_arr[1]; gameTick(5);
        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = STATE_STAND_SIDE_VIEW; gameTick(0);
}

/* a_peeka: 6-tick look-away with head frame 2.
   addr: a_peeka() */

void
a_peeka()
{
        short   saved_frame;

        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        g_hsfra      = 2;
        gameTick(6);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}

/* a_pacen: 15-frame side-shift alternation.
   addr: a_pacen() */

void
a_pacen()
{
        short   i;

        pst_arr[0]  = STATE_PACE_SHIFT_LEFT;
        pst_arr[1]  = STATE_PACE_SHIFT_RIGHT;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        for (i = 0; i < 15; i = i + 1) {
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* a_toggt: flip the TV state.  Both tt_on and tt_off
   handle their own SFX_TV_CLICK.
   addr: a_toggt() */

void
a_toggt()
{
        if (lcp_tv == NO)
                tt_on();
        else
                tt_off();
}

/* a_sleep: lie in bed, snore, optionally forever (value == -1 is
   the copy-protection punishment path).  On value == -1 the resident
   first walks to the current floor's center Y before lying down.
   addr: a_sleep() */

void
a_sleep(value)
short   value;
{
        short   duration;
        short   i;
        short   floor;

        pst_arr[0] = STATE_SLP_BREATHE_I;
        pst_arr[1] = STATE_SLP_BREATHE_O;

        if (lcp_stR != NO)
                return;

        if (value == -1) {
                g_wtx = lcp_x;
                floor = getFlrY(lcp_y);
                g_wty = flr_cy[floor - 1];
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();
        }

        duration = rndRng(7, 15);
        if (value != -1)
                duration = value;

        i = 0;
        while (i < duration &&
               g_trel[0] == ACTION_NONE) {
                lcp_st = pst_arr[0]; gameTick(1);
                lcp_st = pst_arr[1]; gameTick(0);
                sf_sele(SFX_SNORING, 3L);
                gameTick(1);
                lcp_st = pst_arr[0]; gameTick(1);
                i = i + 1;
        }

        if (value == -1) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
        }
}
