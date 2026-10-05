/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* danceToMusic: dance to a record.  If none is playing (lcp_recP) one is
   started with playRecord first.  The resident then walks to the dance
   floor and alternates the left/right dance-step poses every two
   ticks for as long as music plays (mi_play), stopping early when a
   new action is queued in g_trel[0]. */
void
danceToMusic()
{
        /* One local (the loop counter); the walk result is tested
           in place. */
        short   i;

        pst_arr[0] = STATE_DANCE_STEP_LEFT;
        pst_arr[1] = STATE_DANCE_STEP_RIGHT;

        if (lcp_recP == NO) {
                g_actif = YES;
                playRecord();
        }
        g_actif = NO;

        posToXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        g_wty += 8;
        if (walkToTarget() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();

        /* i is never initialised -- the first iteration reads
           whatever the frame slot held.  Kept as in the original. */
        while (mi_play != NO) {
                i++;
                lcp_st = pst_arr[i & 1];
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(2);
        }

        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
