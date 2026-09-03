/*
 * parts/lcp_std.c -- shared body; LCP_STX links it in right after
 * a_opcuc in the 0xdece object (a_opcuc reaches it with a short bsr).
 */

/* Study-door save flow: close door, optionally write HYBER, reopen,
   walk resident back to door, close.  Food-count nibble (bits 9..11)
   is preserved via the FE00 mask so the 3-bit delivery counter survives.
   addr: lcp_std() */
void
lcp_std(do_save, p_dosnd)
BOOL16  do_save;
BOOL16  p_dosnd;
{
        short   saved_x;        /* STX: link #-6, the delay is not
                                   latched */

        saved_x = lcp_x;

        /* Phase 1: door closes (sprite in front of the resident). */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_1);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_1]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_1]] =  23;
        od_draw(od_stcl, 178, 23);

        if (p_dosnd != NO)
                sf_sele(SFX_DOOR_CLOSE, 6L);

        gameTick(1);
        gameTick(rndRng(15, 30));

        /* Phase 2: repack door state and write HYBER. */
        if (do_save != NO) {
                lcp.water_level = lcp_watr;
                /* STX masks in place and ORs the bits back, lowest
                   shift first with the front door last. */
                lcp.door_states_and_flags &= DSF_PRESERVE_UPPER_MASK;
                lcp.door_states_and_flags |=
                        (studyDrO     << 1) |
                        (lcp_clsO    << 2) |
                        (lcp_cabO        << 3) |
                        (lcp_drsO        << 4) |
                        (lcp_toiO    << 5) |
                        (lcp_flcO << 6) |
                        (lcp_bwlS     << 7) |
                        lcp_frdO;
                lcp.record_playing = lcp_recP;
                lcp.tv_on          = lcp_tv;
                lcp.food_supply    = lcp_food;
                lcp_save("hyber", 0x80, &lcp);
        }

        /* Phase 3a: door swings ajar. */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] =  23;
        od_draw(od_sto1, 178, 23);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        /* Phase 3b: door wide open, resident visible. */
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;
        od_draw(od_sto2, 178, 23);
        showLcp();
        gameTick(1);

        /* Phase 4: walk resident back to the study door. */
        lcp_x = saved_x;
        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        /* Phase 5: close door, clear the "study door open" flag. */
        if (studyDrO != NO) {
                g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] =
                        SPRITE_HIDDEN;
                sp_upds();
                gameTick(0);
        }
        od_draw(od_sto1, 178, 23);
        gameTick(2);
        od_draw(od_stcl, 178, 23);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        studyDrO = NO;
}
