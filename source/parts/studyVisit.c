/*
 * parts/studyVisit.c -- included by stx_u2.c right after enterStudy, which
 * must stay close enough for a short call; never compiled on its own.
 */

/* Study-door save flow: close door, optionally write HYBER, reopen,
   walk resident back to door, close.  Food-count nibble (bits 9..11)
   is preserved via the FE00 mask so the 3-bit delivery counter survives. */
void
studyVisit(do_save, p_dosnd)
BOOL16  do_save;
BOOL16  p_dosnd;
{
        short   saved_x;        /* the delay is passed straight to
                                   gameTick, not kept in a local */

        saved_x = lcp_x;

        /* Phase 1: door closes (sprite in front of the resident). */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_1);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_1]] = STUDY_DOOR_X;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_1]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);

        if (p_dosnd != NO)
                sfxSelect(SFX_DOOR_CLOSE, 6L);

        gameTick(1);
        gameTick(rndRng(15, 30));

        /* Phase 2: repack door state and write HYBER. */
        if (do_save != NO) {
                lcp.water_level = lcp_watr;
                /* Mask in place, then OR the bits back -- lowest shift
                   first, front door last; this order is the original's. */
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
                saveFile("hyber", 0x80, &lcp);
        }

        /* Phase 3a: door swings ajar. */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_HIDDEN;
        layoutSlots();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_X;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        /* Phase 3b: door wide open, resident visible. */
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        layoutSlots();
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_X;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_2, STUDY_DOOR_X, STUDY_DOOR_Y);
        showResident();
        gameTick(1);

        /* Phase 4: walk resident back to the study door. */
        lcp_x = saved_x;
        posToXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();
        g_actif = NO;

        /* Phase 5: close door, clear the "study door open" flag. */
        if (studyDrO != NO) {
                g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] =
                        SPRITE_HIDDEN;
                layoutSlots();
                gameTick(0);
        }
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        gameTick(2);
        drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        studyDrO = NO;
}
