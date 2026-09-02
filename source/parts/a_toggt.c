/*
 * parts/a_toggt.c -- shared body; LCP_ORG links it in aidle.c,
 * LCP_STX in the 0xdece object (0x13bb2, immediately before tt_on).  Files under parts/
 * are never compiled standalone.
 */

/* addr: a_toggt() */
void
a_toggt()
{
#ifdef FAITHFUL
        if (lcp_tv == NO)
                tt_on();
        else
                tt_off();
#else
        if (lcp_tv != NO)
                tt_off();
        else
                tt_on();
#endif
}
