/*
 * parts/a_opcuc.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_opcuc: go into the study (upstairs closet).  The resident walks
   to the study door, opens it if shut, walks in behind the wide-open
   door sprite and is hidden, then lcp_std takes over to close the door
   and bring him back out.  value non-zero asks lcp_std to save the
   game (HYBER) while he is inside.  Returns early if the first walk
   is preempted. */
void
a_opcuc(value)
short   value;
{
        /* Unused -- the call is tested in place -- but it must stay:
           removing it changes the compiled code. */
        short   result;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (studyDrO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_stcl,  178, 23);
                gameTick(2);
                od_draw(od_sto1,  178, 23);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_sto2,  178, 23);
                gameTick(2);
                studyDrO = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        /* Written as -= on purpose: `x = x - n` compiles differently. */
        g_wty -= 3;
        g_wtx -= 10;
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] =  23;
        od_draw(od_sto1, 178, 23);
        hideLcp();
        gameTick(1);
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();

        /* Continue into the study; value != 0 -> save HYBER.  The
           test is written as `!= 0` on purpose: the inverted form
           compiles differently. */
        if (value != 0)
                lcp_std(YES, YES);
        else
                lcp_std(NO,  YES);
}
