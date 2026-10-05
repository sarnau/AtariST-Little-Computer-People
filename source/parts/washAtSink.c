/*
 * The resident stands at the kitchen sink with the water running.
 */

void
washAtSink(value)
short   value;
{
        /* Declaration order matters: it sets the stack-frame layout,
           which must match the original. */
        short           rnd;
        short           counter;
        short           last_pick;
        short           pick;

        scratchArr[0] = STATE_WASH_HANDS_CENTER;
        scratchArr[1] = STATE_WASH_HANDS_LEFT;
        scratchArr[2] = STATE_WASH_HANDS_RIGHT;

        carryBehind(value);
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkToTarget();
        spriteLayer[value] = SPRITE_HIDDEN;
        layoutSlots();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        rnd = (unsigned short)(Random() & 0x1f) | 4;
        sfxSelect(SFX_WATER_RUNNING, 10000L);

        /* last_pick is never initialised, so the first comparison
           reads whatever the stack slot held.  That is how the 1985
           code is written; do not "fix" it. */
        for (counter = 0; counter < rnd; counter++) {
                pick = Random() & 3;
                while (pick == last_pick)
                        pick = Random() & 3;
                last_pick = pick;
                if (pick != 3) {
                        animState = scratchArr[pick];
                        resFacing = FACING_RIGHT;
                } else {
                        animState = scratchArr[1];
                        resFacing = FACING_LEFT;
                }
                gameTick(1);
        }

        if (sfxPlaying != NO &&
            sfxCurId == SFX_WATER_RUNNING)
                stopSfx();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
