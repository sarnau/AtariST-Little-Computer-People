/*
 * parts/lcp_hwt.c -- shared body; LCP_STX puts it at 0x1568a, directly before gameTick (0x156a6),
 * so its call to gameTick is a SHORT bsr.
 */

/* lcp_hwt: tick until g_hacur == g_hatas.
   addr: lcp_hwt() */

void
lcp_hwt()
{
        while (g_hacur != g_hatas)
                gameTick(0);
}
