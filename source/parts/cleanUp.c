/*
 * parts/cleanUp.c -- included by stx_u2.c; never compiled on its own.
 */

/* cleanUp: tidy up.  Visits each door the house tracks as open --
   filing cabinet, study door, toilet, bedroom closet, dresser, kitchen
   cabinet, front door -- walks to it and closes it, clearing its
   lcp_*O / studyDrO flag through the matching close routine.  Each
   walk can be preempted by a new action, which abandons the rest. */
void
cleanUp()
{
        /* Every call result is consumed in place, with no local. */

        if (lcp_flcO != NO) {
                posToXY(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                closeFilingCab();
        }
        if (studyDrO != NO) {
                posToXY(POS_TOP_STUDY_DOOR,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st            = STATE_STAND_FACING_SCREEN;
                /* The original really does compute g_hatas - 12 here
                   and throw it away.  It is an expression statement in
                   the 1985 source and the compiler emits it, so it is
                   kept on purpose. */
                g_hatas - 12;
                waitHeadTurn();
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
                sfxSelect(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                studyDrO = NO;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(0);
        }
        if (lcp_toiO != NO) {
                posToXY(POS_MID_TOILET_DOOR,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                closeToiletDoor();
        }
        if (lcp_clsO != NO) {
                posToXY(POS_MID_BEDROOM_CLOSET,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                closeBedCloset();
        }
        if (lcp_drsO != NO) {
                posToXY(POS_MID_DRESSER,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                openDresser(DOOR_CLOSE);
        }
        if (lcp_cabO != NO) {
                posToXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                openKitchenCab(DOOR_CLOSE);
        }
        if (lcp_frdO != NO) {
                walkToFrontDoor();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                openFrontDoor(DOOR_CLOSE);
        }
}
