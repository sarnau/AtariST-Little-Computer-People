/* useComputer: use the computer.  The resident walks to the computer
   desk and sits; then for 0x80..0x1ff steps he alternates hands-down
   (with a key click, sfxClick) and hands-up poses with random pauses,
   quitting early on a new action or during the intro.  Rarely, after
   a keystroke, he looks up while the computer's small screen is
   cleared and redrawn with a random animation (tvClearAnim). */
void
useComputer()
{
        short   randomVal;
        short   limit;
        short   typed;
        short   typeCounter;
        short   isEvenFrame;

        scratchArr[0] = STATE_HANDS_DOWN;
        scratchArr[1] = STATE_HANDS_UP;
        scratchArr[2] = STATE_SITTING_AT_DESK;

        posToXY(POS_MID_COMPUTER_DESK, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        headMode = HEAD_ANIM_COMPUTER;

        /* The first draw is computed and thrown away. */
        randomVal = (Random() & 7) | 3;
        limit      = (Random() & 0x1ff) | 0x80;

        animState = scratchArr[2];
        gameTick(25);

        typeCounter = 0;
        while (typeCounter < limit) {
                if (movingIn != NO)
                        break;
                if (eventQueue[0] != ACTION_NONE)
                        break;
                randomVal = Random() & 3;
                if (typeCounter & 1)
                        isEvenFrame = 0;
                else
                        isEvenFrame = 1;

                if (isEvenFrame == 0) {
                        resFacing = (Random() & 2) >> 1;
                        typed = 1;
                        animState = scratchArr[0];
                        sfxClick();
                } else {
                        resFacing = FACING_RIGHT;
                        typed = 0;
                        animState = scratchArr[1];
                }

                if (typed == 1)
                        gameTick(0);
                else
                        gameTick(randomVal);

                /* Rare "clear the screen" gesture. */
                if ((Random() & 0x7f) < 3 && typed != 0) {
                        headMode = HEAD_ANIM_DISABLED;
                        animState = scratchArr[2];
                        headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                        resFacing = FACING_RIGHT;
                        waitHeadTurn();
                        tvClearAnim();
                        gameTick(5);
                        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                        waitHeadTurn();
                        headMode = HEAD_ANIM_COMPUTER;
                }

                typeCounter++;
        }

        animState = STATE_STAND_FACING_SCREEN;
        resFacing = FACING_RIGHT;
        headMode = HEAD_ANIM_DISABLED;
        gameTick(5);
}
