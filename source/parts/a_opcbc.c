/*
 * parts/a_opcbc.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aleisure functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_opcbc(value)
short   value;
{
        short   saved_x;

        hs_posXY(POS_MID_DRESSER,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opecd(0);
        if (lcp.initiative_threshold < rndRng(0, 100))
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
        g_wty -= 3;
        g_wtx -= 10;
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;                   /* STX clears the flag first */
        saved_x = lcp_x;

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

        gameTick(rndRng(45, 60));
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

        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
                a_clocd();
}
