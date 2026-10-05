/*
 * parts/tvOff.c -- included by stx_u2.c immediately after tvOn;
 * never compiled on its own.
 */

/* Turns the TV off: if it is on, the resident walks to the TV in the
   top-floor living room, does the look gesture (tvStoop), clears lcp_tv
   and blanks the picture.  Returns -1 if the walk was interrupted, 0
   when done; returns no value when the TV was already off. */
short

tvOff()
{
        if (lcp_tv == NO)
                return;

        posToXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        g_wtx += 0;
        if (walkToTarget() != 0)
                return -1;

        gameTick(2);
        tvStoop();
        lcp_tv = NO;
        drawTvPicture(COLOR_white);
        return 0;
}
