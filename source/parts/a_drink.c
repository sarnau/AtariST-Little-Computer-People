/*
 * parts/a_drink.c -- shared body; LCP_ORG links it in ahouse.c,
 * LCP_STX in the 0xdece object (0x121d6, immediately before updWtLv).  Files under parts/
 * are never compiled standalone.
 */

void
a_drink()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        g_actif = YES;
        sp_ssco(SPRITE_GLASS);
        hs_posXY(POS_BTM_WATER_TAP,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (lcp_watr != 0) {
                lcp_st = STATE_BEND_DOWN;
                lcp_face = FACING_RIGHT;
                gameTick(0);
                updWtLv(-3);
                g_hamod = HEAD_ANIM_DISABLED;
                lcp_st = STATE_DRINK_FROM_GLASS;
                gameTick(16);
                lcp_st = STATE_STAND_FACING_SCREEN;
#ifdef FAITHFUL
                lcp_y = lcp_y + 1;
#else
                lcp_y++;
#endif
                gameTick(3);
                a_driwa(3);
        }

        lcp.thirst_level = NEED_SATISFIED;
        lcp.thirst_timer = lcp.thirst_timer_max;
        lcp_rcov();
        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        g_actif = NO;
}
