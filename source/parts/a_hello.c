/*
 * parts/a_hello.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_hello()
{
        /* Frame offsets pin STX's declaration order: saved_frame -2,
           pick -4, wave_count -6, prev_pick -8, wait -10. */
#ifdef FAITHFUL
        short   wave_count;
        short   pick;
        short   prev_pick;
        short   saved_frame;
        short   wait;
#else
        short   saved_frame;
        short   pick;
        short   wave_count;
        short   prev_pick;
        short   wait;
#endif

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
#ifdef FAITHFUL
        prev_pick  = 0;
        pick       = 0;
#else
        pick       = 0;
        prev_pick  = 0;
#endif
        /* STX drives the loop from a post-decrement in the
           condition (read, subq to memory, test the old value). */
#ifdef FAITHFUL
        while (wave_count != 0) {
#else
        while (wave_count--) {
#endif
                while (pick == prev_pick)
                        pick = rndRng(0, 2);
                prev_pick = pick;

                /* STX dispatches with a switch (Alcyon emits the
                   compare chain at the bottom); the port used an
                   if/else-if ladder. */
#ifdef FAITHFUL
                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        if (rndRng(0, 1) == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = 4;
                        p_sfhnd();
                }
#else
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
#endif
                /* STX nests the assignment in the call, so the value
                   stays in the register. */
#ifdef FAITHFUL
                wait = rndRng(1, 2);
                gameTick(wait);
#else
                gameTick(wait = rndRng(1, 2));
#endif
                g_sfret = (long) wait;
#ifdef FAITHFUL
                wave_count = wave_count - 1;
#endif
        }

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}
