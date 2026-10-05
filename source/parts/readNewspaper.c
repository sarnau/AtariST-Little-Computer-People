/* readNewspaper: read the newspaper.  The TV is switched on first (tvOn),
   then the resident walks to the armchair, sits, and reads for up to
   200 ticks -- holding the paper and turning a page about one tick in
   sixteen -- until a new action is queued.  He is lowered 8 pixels
   into the chair for the reading poses and raised again afterwards,
   and the TV is switched off at the end. */
void
readNewspaper()
{
        /* The limit is declared before the counter, and the walk call
           and the Random test are inline with no local; all as in the
           original. */
        short           t;
        short           i;

        scratchArr[0] = STATE_READ_PAPER_HOLD;
        scratchArr[1] = STATE_READ_PAPER_TURN_PAGE;
        tvOn();
        posToXY(POS_TOP_ARMCHAIR, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        headMode = HEAD_ANIM_READING;
        resFacing = FACING_LEFT;
        animState = STATE_SIT_IN_ARMCHAIR;
        headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);
        waitHeadTurn();
        /* The limit is set before the coordinate steps, and
           `resX += 0` is a no-op the original wrote.  Both kept on
           purpose. */
        t = 200;
        resX += 0;
        resY += 8;
        i = 0;

        while (i < t) {
                if (eventQueue[0] != ACTION_NONE)
                        break;
                resFacing = FACING_LEFT;
                animState = scratchArr[0];
                if ((Random() & 0xf) == 5)
                        animState = scratchArr[1];
                gameTick(1);
                i++;
        }

        resY -= 8;
        resFacing = FACING_LEFT;
        animState = STATE_SIT_IN_ARMCHAIR;
        gameTick(2);
        tvOff();
}
