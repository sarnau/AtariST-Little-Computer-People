/* cleanUp: tidy up.  Visits each door the house tracks as open --
   filing cabinet, study door, toilet, bedroom closet, dresser, kitchen
   cabinet, front door -- walks to it and closes it, clearing its
   lcp_*O / studyDoorOpen flag through the matching close routine.  Each
   walk can be preempted by a new action, which abandons the rest. */
void
cleanUp()
{
        /* Every call result is consumed in place, with no local. */

        if (filingCabOpen != NO) {
                posToXY(POS_TOP_FILING_CABINET, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                closeFilingCab();
        }
        if (studyDoorOpen != NO) {
                posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                /* The original really does compute headTarget - 12 here
                   and throw it away.  It is an expression statement in
                   the 1985 source and the compiler emits it, so it is
                   kept on purpose. */
                headTarget - 12;
                waitHeadTurn();
                resFacing = FACING_LEFT;
                animState = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
                sfxSelect(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                studyDoorOpen = NO;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(0);
        }
        if (toiletDoorOpen != NO) {
                posToXY(POS_MID_TOILET_DOOR, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                closeToiletDoor();
        }
        if (bedClosetOpen != NO) {
                posToXY(POS_MID_BEDROOM_CLOSET, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                closeBedCloset();
        }
        if (dresserOpen != NO) {
                posToXY(POS_MID_DRESSER, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                openDresser(DOOR_CLOSE);
        }
        if (kitchenCabOpen != NO) {
                posToXY(POS_BTM_KITCHEN_CABINET, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                openKitchenCab(DOOR_CLOSE);
        }
        if (frontDoorOpen != NO) {
                walkToFrontDoor();
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                openFrontDoor(DOOR_CLOSE);
        }
}
