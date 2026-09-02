/*
 * aidle.c -- short "no-walk" idle / gesture handlers.
 * addr: a_wandi(), a_peeka(), a_pacen(), a_toggt(), a_sleep()
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

/* addr: a_wandi() */
void
a_wandi()
{
#ifndef FAITHFUL
        /* STX's frame is 2 bytes larger (link #-6 vs #-4): the
           original declared a local here that the body never uses. */
        short   unused;
#endif

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

/* addr: a_peeka() */
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

/* addr: a_pacen() */
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

        /* LCP_ORG's source uses the register form; the STX revision
           writes i++ (addq straight to the frame slot). */
#ifdef FAITHFUL
        for (i = 0; i < 15; i = i + 1) {
#else
        for (i = 0; i < 15; i++) {
#endif
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* addr: a_toggt() */
void
a_toggt()
{
        if (lcp_tv == NO)
                tt_on();
        else
                tt_off();
}

/* value == -1 is the copy-protection punishment path (sleep forever);
   the resident first walks to the current floor's center Y before lying down.
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
