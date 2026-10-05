/*
 * parts/yawnAndStretch.c -- included by stx_u2.c; never compiled on its own.
 */

/* ACTION_YAWN_AND_STRETCH: the resident turns side-on and, for 15
   ticks, alternates the open-mouthed yawn and the arm stretch, then
   stands side-on again. */
void
yawnAndStretch()
{
        short   i;

        pst_arr[0]  = STATE_YAWN_MOUTH_OPEN;
        pst_arr[1]  = STATE_YAWN_STRETCH_ARMS;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();

        /* Written as i++ on purpose: `i = i + 1` compiles differently. */
        for (i = 0; i < 15; i++) {
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
