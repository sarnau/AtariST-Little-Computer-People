/*
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_peeka()
{
        short   saved_frame;

        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        g_hsfra      = 2;
        gameTick(6);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}
