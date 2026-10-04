/*
 * Must sit directly before a_clotd.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_USE_TOILET (also from games.c and cs_mvIn).  The resident
   walks to the bathroom toilet door, opens it if it is shut
   (SFX_DOOR_OPEN), steps in and the door closes behind him in three
   sprite phases while he is hidden (SFX_DOOR_CLOSE).  After 45..60
   ticks the toilet flushes (SFX_TOILET_FLUSH), the door opens again,
   he reappears and walks back out, and leaves it open unless a
   0..100 roll beats his initiative_threshold or the cutscene runs, in
   which case a_clotd shuts it.  Clears lcp.bathroom_need and resets
   the bathroom timer. */
void
a_uset()
{
        /* The walk call is tested in place, with no local. */
        short   saved_x;

        hs_posXY(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

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
        g_wty -= 3;
        g_wtx -= 10;
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
        gameTick(rndRng(45, 60));       /* no temporary, on purpose */
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

        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
                a_clotd();

        lcp.bathroom_need  = NO;
        lcp.bathroom_timer = BATHROOM_TIMER_OFF;
        g_actif = NO;
}
