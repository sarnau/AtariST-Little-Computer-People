/*
 * parts/a_opcbc.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_opcbc(value)
short   value;
{
#ifdef FAITHFUL
        short   result;
        short   saved_x;
        short   counter;
#else
        short   saved_x;
#endif

        hs_posXY(POS_MID_DRESSER,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
#else
        if (lcp_wkD() != 0)
#endif
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opecd(0);
#ifdef FAITHFUL
        counter = rndRng(0, 100);
        if (lcp.initiative_threshold < counter)
#else
        if (lcp.initiative_threshold < rndRng(0, 100))
#endif
                a_opecd(1);

        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (lcp_clsO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_clcl, 75, 87);
                gameTick(2);
                od_draw(od_clo1, 75, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_clo2, 75, 87);
                gameTick(2);
                lcp_clsO = YES;
        }

        /* Walk into the closet. */
        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;

        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        /* STX: -= straight to memory. */
#ifdef FAITHFUL
        g_wty = g_wty - 3;
#else
        g_wty -= 3;
#endif
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        g_actif = YES;
        lcp_wkD();
#ifdef FAITHFUL
        saved_x = lcp_x;
        g_actif = NO;
#else
        g_actif = NO;                   /* STX clears the flag first */
        saved_x = lcp_x;
#endif

        /* Close door behind: wide -> ajar -> lcp-inside. */
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_AJAR);
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(od_clo1, 75, 87);
        gameTick(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_LCP_INSIDE);
        hideLcp();
        g_sepex[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 87;
        od_draw(od_clcl, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

#ifdef FAITHFUL
        counter = rndRng(45, 60);
        gameTick(counter);
#else
        gameTick(rndRng(45, 60));
#endif
        if (introSeq == NO) {
                if (value == 0)
                        pa_cloc();
                else
                        pa_skic();
        }

        /* Open door back up + walk out. */
        g_selaf[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_AJAR);
        showLcp();
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(od_clo1, 75, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;
        od_draw(od_clo2, 75, 87);
        gameTick(1);
        lcp_clsO = YES;

        lcp_x = saved_x;
        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        if (lcp_clsO != NO) {
                g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
                sp_upds();
                gameTick(0);
        }

#ifdef FAITHFUL
        counter = rndRng(0, 100);
        if (lcp.initiative_threshold < counter ||
            introSeq != NO)
#else
        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
#endif
                a_clocd();
}
