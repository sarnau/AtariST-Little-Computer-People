/*
 * parts/a_playp.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x13a62, right after a_lists). Files under parts/ are never
 * compiled standalone.
 */
/* a_playp: stop a currently-playing record so the resident can start
   writing/typing.  Walks to dance floor, drains MIDI buffer, frees it.
   addr: a_playp() */

void
a_playp()
{
        /* STX has no local: the walk result is tested in place. */

        if (lcp_recP == NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        gameTick(2);

        if (mi_play != NO) {
                mq_inis(mi_sbuf, g_momap);
                while (mi_play != NO)
                        ;
        }
        li_loor();
        lcp_recP = NO;
        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
}
