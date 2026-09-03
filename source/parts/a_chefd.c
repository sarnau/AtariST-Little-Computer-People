/*
 * parts/a_chefd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_chefd(value)
short   value;
{
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_BTM_FRONT_DOOR,
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
        if (lcp_frdO == NO)
                a_opcfd(0);
        g_actif = YES;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        lcp_wkD();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(value);
        showLcp();

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        lcp_wkD();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

#ifdef FAITHFUL
        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result) {
#else
        if (lcp.initiative_threshold < rndRng(0, 100)) {
#endif
                g_actif = YES;
                hs_posXY(POS_BTM_FRONT_DOOR,
                                      &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(1);
        }
        g_actif = NO;
}
