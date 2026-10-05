/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Turns the TV on: if it is off, the resident walks to the TV in the
   top-floor living room, does the look gesture (tvStoop), sets lcp_tv
   so the tick draws the flickering picture, and plays the TV sound
   (sfxTvClick).  Returns -1 if the walk was interrupted, 0 when done;
   returns no value when the TV was already on. */
short

tvOn()
{
        if (lcp_tv != NO)
                return;

        posToXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        g_wtx += 0;
        if (walkToTarget() != 0)
                return -1;

        gameTick(2);
        tvStoop();
        lcp_tv = YES;
        sfxTvClick();
        return 0;
}
