/*
 * Included by stx_u2.c; never compiled on its own.
 */


void
ev_ansPh()
{
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        a_calld();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        lcp_hwt();

        lcp_y += 6;
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        od_draw(od_med1, 190, 168);

        lcp_st = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = rndRng(0x28, 0x32);
        while (ticks-- != 0) {
                switch (rndRng(0, 2)) {
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
                        g_hsfra = saved_frame;
                        p_sfhnd();
                        break;
                }
                gameTick(subpick = rndRng(1, 2));
                g_sfret = (long) subpick;
        }

        g_hsfra = saved_frame;
        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        gameTick(1);

        lcp_y -= 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        pat_ok = NO;
        lcp_y -= 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
