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
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        headMode         = HEAD_ANIM_DISABLED;
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        if (studyDoorOpen == NO) {
                resFacing = FACING_LEFT;
                animState = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_2, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                studyDoorOpen = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        resFacing = FACING_RIGHT;
        spriteLayer[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_WIDE_OPEN);
        pendX[spriteSlot[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_X;
        pendY[spriteSlot[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_Y;

        posToXY(POS_TOP_STUDY_DOOR,
                              &walkXTarget, &walkYTarget);
        /* Written as -= on purpose: `x = x - n` compiles differently. */
        walkYTarget -= 3;
        walkXTarget -= 10;
        noPreempt = YES;
        walkToTarget();
        noPreempt = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        spriteLayer[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_AJAR);
        pendX[spriteSlot[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_X;
        pendY[spriteSlot[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        hideResident();
        gameTick(1);
        spriteLayer[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        layoutSlots();

        /* Continue into the study; value != 0 -> save HYBER.  The
           test is written as `!= 0` on purpose: the inverted form
           compiles differently. */
        if (value != 0)
                studyVisit(YES, YES);
        else
                studyVisit(NO,  YES);
}
