/*
 * parts/sayHello.c -- the resident waves and talks at the screen.
 * Included by stx_u2.c; never compiled on its own.
 */

void
sayHello()
{
        /* Declaration order matters: it sets the stack-frame layout,
           which must match the original. */
        short   saved_frame;
        short   pick;
        short   wave_count;
        short   prev_pick;
        short   wait;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        wave_count = rndRng(20, 40);
        /* pick is cleared before prev_pick on purpose (statement order
           shows in the compiled code). */
        pick       = 0;
        prev_pick  = 0;
        /* Post-decrement in the condition, testing the old value --
           the original's loop shape. */
        while (wave_count--) {
                while (pick == prev_pick)
                        pick = rndRng(0, 2);
                prev_pick = pick;

                /* Must stay a switch: an if/else-if ladder compiles to
                   different code. */
                switch (pick) {
                case 0:
                        g_hsfra = 5;
                        sfxTvClick();
                        break;
                case 1:
                        g_hsfra = 6;
                        if (rndRng(0, 1) != 0)
                                sfxSpeech();
                        else
                                sfxGreeting();
                        break;
                case 2:
                        g_hsfra = 4;
                        sfxHeadNod();
                        break;
                }
                /* The assignment is nested in the call on purpose, so
                   the value is reused from the register. */
                gameTick(wait = rndRng(1, 2));
                g_sfret = (long) wait;
        }

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}
