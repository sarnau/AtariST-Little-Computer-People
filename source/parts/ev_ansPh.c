/*
 * parts/ev_ansPh.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from delivery.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in delivery.c.
 * Files under parts/ are never compiled standalone.
 */

#ifdef FAITHFUL
void
ev_ansPh()
{
        short   pick;
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        a_calld();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        lcp_hwt();

#ifdef FAITHFUL
        lcp_y = lcp_y + 6;
#else
        lcp_y += 6;
#endif
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        /* ROM reads od_med1 (=40, OBJ_MEDICINE_OPEN_1) here, not the
           phone-call frame -- likely an original-game slip, kept. */
        od_draw(od_med1, 190, 168);

        lcp_st = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = rndRng(0x28, 0x32);
        while (ticks != 0) {
                pick = rndRng(0, 2);
                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        subpick = rndRng(0, 1);
                        if (subpick == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = saved_frame;
                        p_sfhnd();
                }
                subpick = rndRng(1, 2);
                gameTick(subpick);
                g_sfret = (long) subpick;
#ifdef FAITHFUL
                ticks = ticks - 1;
#else
                ticks -= 1;
#endif
        }

        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        g_hsfra = saved_frame;
        gameTick(1);

#ifdef FAITHFUL
        lcp_y = lcp_y - 6;
#else
        lcp_y -= 6;
#endif
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
#ifdef FAITHFUL
        lcp_y = lcp_y - 2;
#else
        lcp_y -= 2;
#endif
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
#else   /* STX: link #-10 -- saved_frame, ticks, subpick; the
           gesture pick is a switch, not an if/else ladder. */

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

        dg_petok = NO;
        lcp_y -= 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
#endif
