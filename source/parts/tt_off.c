/*
 * parts/tt_off.c -- included by stx_u2.c immediately after tt_on;
 * never compiled on its own.
 */

/* Turns the TV off: if it is on, the resident walks to the TV in the
   top-floor living room, does the look gesture (li_lool), clears lcp_tv
   and blanks the picture.  Returns -1 if the walk was interrupted, 0
   when done; returns no value when the TV was already off. */
short

tt_off()
{
        if (lcp_tv == NO)
                return;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        g_wtx += 0;
        if (lcp_wkD() != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = NO;
        td_line(COLOR_white);
        return 0;
}
