/*
 * Stop a currently-playing record so the resident can start
 * writing/typing: walks to the dance floor, drains the MIDI buffer and
 * frees it.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

void
stopRecord()
{
        /* No local: the walk result is tested in place. */

        if (lcp_recP == NO)
                return;

        posToXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        gameTick(2);

        if (mi_play != NO) {
                startSong(mi_sbuf, g_momap);
                while (mi_play != NO)
                        ;
        }
        recordStoop();
        lcp_recP = NO;
        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
}
