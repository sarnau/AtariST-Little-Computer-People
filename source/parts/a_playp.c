/*
 * parts/a_playp.c -- shared body; LCP_ORG links it in aleisure.c,
 * LCP_STX in the 0xdece object (0x13a62, right after a_lists).  Files under parts/
 * are never compiled standalone.
 */
/* a_playp: stop a currently-playing record so the resident can start
   writing/typing.  Walks to dance floor, drains MIDI buffer, frees it.
   addr: a_playp() */

void
a_playp()
{
        /* STX has no local: the walk result is tested in place. */
#ifdef FAITHFUL
        short   result;
#endif

        if (lcp_recP == NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

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
