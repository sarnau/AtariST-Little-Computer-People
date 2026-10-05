/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* The resident dozes off.  With SLEEP_RANDOM (ACTION_SLEEP, moveInScene,
   and gameLoop's endless loop when the copy protection fails) he first
   walks to the centre line of the floor he is on and turns side-on,
   then sleeps 7..15 rounds; any other value is the round count, slept
   on the spot (playGame's dozeOff(1) between key polls).  Each round
   plays the breathe-in/out poses from pst_arr and SFX_SNORING.  Does
   nothing while he is on the stairs (lcp_stR); a queued action
   (g_trel[0]) wakes him early. */
void
dozeOff(value)
short   value;
{
        short   i;
        short   duration;

        pst_arr[0] = STATE_SLP_BREATHE_I;
        pst_arr[1] = STATE_SLP_BREATHE_O;

        if (lcp_stR != NO)
                return;

        if (value == SLEEP_RANDOM) {
                g_wtx = lcp_x;
                g_wty = flr_cy[floorOfY(lcp_y) - 1];
                if (walkToTarget() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                waitHeadTurn();
        }

        duration = rndRng(7, 15);
        if (value != SLEEP_RANDOM)
                duration = value;

        i = 0;
        while (i < duration) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                lcp_st = pst_arr[0]; gameTick(1);
                lcp_st = pst_arr[1]; gameTick(0);
                sfxSelect(SFX_SNORING, 3L);
                gameTick(1);
                lcp_st = pst_arr[0]; gameTick(1);
                i++;
        }

        if (value == SLEEP_RANDOM) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
        }
}
