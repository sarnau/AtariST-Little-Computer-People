/*
 * parts/a_uset.c -- shared body; LCP_ORG links it in ahouse.c,
 * LCP_STX in the 0xdece object (0x101be, immediately before a_clotd).  Files under parts/
 * are never compiled standalone.
 */

void
a_uset()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
        short   saved_x;
        short   counter;
#else
        short   saved_x;
#endif

        hs_posXY(POS_MID_TOILET_DOOR,
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

        if (lcp_toiO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_tocl, 187, 87);
                gameTick(2);
                od_draw(od_too1, 187, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_too2, 187, 87);
                gameTick(2);
                lcp_toiO = YES;
        }

        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;

        hs_posXY(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
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
        saved_x = lcp_x;

        /* Close door behind resident (3 sprite phases). */
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(od_too1, 187, 87);
        gameTick(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_1);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_1]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_1]] = 87;
        od_draw(od_tocl, 187, 87);
        hideLcp();
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

        /* 45..60 ticks, then flush + 16 tick refill. */
#ifdef FAITHFUL
        counter = rndRng(45, 60);
        gameTick(counter);
#else
        gameTick(rndRng(45, 60));       /* STX: no temporary */
#endif
        sf_sele(SFX_TOILET_FLUSH, 6L);
        gameTick(16);

        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        showLcp();
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(od_too1, 187, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;
        od_draw(od_too2, 187, 87);
        gameTick(1);
        lcp_toiO = YES;

        lcp_x = saved_x;
        hs_posXY(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        if (lcp_toiO != NO) {
                g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
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
                a_clotd();

        lcp.bathroom_need  = NO;
        lcp.bathroom_timer = 9999;
        g_actif = NO;
}
