/* ACTION_TIDY_HOUSE (also run during the move-in cutscene).  The
   resident walks to the top-floor filing cabinet, turns to the screen
   and rummages in it with rummageCabinet, which opens it if it is shut.  He
   closes it again (closeFilingCab) when a 0..100 roll beats his
   initiative_threshold, and always during the cutscene (movingIn).
   Interrupted on the way, he simply gives up. */
void
tidyHouse()
{
        /* The walk call is tested inline, with no local for it. */

        posToXY(POS_TOP_FILING_CABINET,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        rummageCabinet();

        /* Both call results are used in place; adding a local here
           would change the compiled code. */
        if (resident.initiative_threshold < rndRng(0, 100) ||
            movingIn != NO)
                closeFilingCab();
}
