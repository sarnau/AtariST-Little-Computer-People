/*
 * parts/a_hello.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * asimple functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_hello()
{
        /* Frame offsets pin STX's declaration order: saved_frame -2,
           pick -4, wave_count -6, prev_pick -8, wait -10. */
        short   saved_frame;
        short   pick;
        short   wave_count;
        short   prev_pick;
        short   wait;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        wave_count = rndRng(20, 40);
        /* STX clears pick first. */
        pick       = 0;
        prev_pick  = 0;
        /* STX drives the loop from a post-decrement in the
           condition (read, subq to memory, test the old value). */
        while (wave_count--) {
                while (pick == prev_pick)
                        pick = rndRng(0, 2);
                prev_pick = pick;

                /* STX dispatches with a switch (Alcyon emits the
                   compare chain at the bottom); the port used an
                   if/else-if ladder. */
                switch (pick) {
                case 0:
                        g_hsfra = 5;
                        p_sftvc();
                        break;
                case 1:
                        g_hsfra = 6;
                        if (rndRng(0, 1) != 0)
                                p_sfspe();
                        else
                                p_sfgrt();
                        break;
                case 2:
                        g_hsfra = 4;
                        p_sfhnd();
                        break;
                }
                /* STX nests the assignment in the call, so the value
                   stays in the register. */
                gameTick(wait = rndRng(1, 2));
                g_sfret = (long) wait;
        }

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}
