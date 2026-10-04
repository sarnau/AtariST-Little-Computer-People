/*
 * parts/a_cleau.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_cleau: tidy up.  Visits each door the house tracks as open --
   filing cabinet, study door, toilet, bedroom closet, dresser, kitchen
   cabinet, front door -- walks to it and closes it, clearing its
   lcp_*O / studyDrO flag through the matching close routine.  Each
   walk can be preempted by a new action, which abandons the rest. */
void
a_cleau()
{
        /* Every call result is consumed in place, with no local. */

        if (lcp_flcO != NO) {
                hs_posXY(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfc();
        }
        if (studyDrO != NO) {
                hs_posXY(POS_TOP_STUDY_DOOR,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st            = STATE_STAND_FACING_SCREEN;
                /* The original really does compute g_hatas - 12 here
                   and throw it away.  It is an expression statement in
                   the 1985 source and the compiler emits it, so it is
                   kept on purpose. */
                g_hatas - 12;
                lcp_hwt();
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_sto1, 178, 23);
                gameTick(2);
                od_draw(od_stcl,  178, 23);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                studyDrO = NO;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(0);
        }
        if (lcp_toiO != NO) {
                hs_posXY(POS_MID_TOILET_DOOR,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                a_clotd();
        }
        if (lcp_clsO != NO) {
                hs_posXY(POS_MID_BEDROOM_CLOSET,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                a_clocd();
        }
        if (lcp_drsO != NO) {
                hs_posXY(POS_MID_DRESSER,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecd(DOOR_CLOSE);
        }
        if (lcp_cabO != NO) {
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecc(DOOR_CLOSE);
        }
        if (lcp_frdO != NO) {
                wkFrDr();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(DOOR_CLOSE);
        }
}
