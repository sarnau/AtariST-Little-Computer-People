/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Turns the TV on: if it is off, the resident walks to the TV in the
   top-floor living room, does the look gesture (li_lool), sets lcp_tv
   so the tick draws the flickering picture, and plays the TV sound
   (p_sftvc).  Returns -1 if the walk was interrupted, 0 when done;
   returns no value when the TV was already on. */
short

tt_on()
{
        if (lcp_tv != NO)
                return;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        g_wtx += 0;
        if (lcp_wkD() != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = YES;
        p_sftvc();
        return 0;
}
