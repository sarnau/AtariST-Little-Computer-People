/*
 * parts/enterStudy.c -- included by stx_u2.c; never compiled on its own.
 */

/* enterStudy: go into the study (upstairs closet).  The resident walks
   to the study door, opens it if shut, walks in behind the wide-open
   door sprite and is hidden, then studyVisit takes over to close the door
   and bring him back out.  value non-zero asks studyVisit to save the
   game (HYBER) while he is inside.  Returns early if the first walk
   is preempted. */
void
enterStudy(value)
short   value;
{
        /* Unused -- the call is tested in place -- but it must stay:
           removing it changes the compiled code. */
        short   result;

        posToXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        if (studyDrO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_2, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                studyDrO = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_X;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_Y;

        posToXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        /* Written as -= on purpose: `x = x - n` compiles differently. */
        g_wty -= 3;
        g_wtx -= 10;
        g_actif = YES;
        walkToTarget();
        g_actif = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        layoutSlots();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_X;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        hideResident();
        gameTick(1);
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        layoutSlots();

        /* Continue into the study; value != 0 -> save HYBER.  The
           test is written as `!= 0` on purpose: the inverted form
           compiles differently. */
        if (value != 0)
                studyVisit(YES, YES);
        else
                studyVisit(NO,  YES);
}
