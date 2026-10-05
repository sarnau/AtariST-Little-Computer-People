/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* peekAround: a quick glance.  The head is settled at position 8, head
   animation is suspended so frame 2 can be forced through g_hsfra
   for 6 ticks, then the saved frame and position 8 are restored.
   Used as an action and by the War card game after the resident's
   remark on a round. */
void
peekAround()
{
        short   saved_frame;

        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        waitHeadTurn();

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
