/* ACTION_WASH_HANDS.  The resident walks to the bathroom sink (giving
   up if interrupted), turns to the screen and starts SFX_WATER_RUNNING,
   then plays 4..127 one-tick hand-washing poses picked at random --
   centre, left, right, or the left pose mirrored -- never the same
   pick twice running.  A queued action ends it early.  The water
   sound is stopped if it is still the effect playing. */
void
washHands()
{
        /* The walk call is tested in place, with no local.  The locals
           are declared in this order, all signed, as in washAtSink. */
        short           rnd;
        short           counter;
        short           last_pick;
        short           val;

        scratchArr[0] = STATE_WASH_HANDS_CENTER;
        scratchArr[1] = STATE_WASH_HANDS_LEFT;
        scratchArr[2] = STATE_WASH_HANDS_RIGHT;

        posToXY(POS_MID_BATHROOM_SINK, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        rnd = (unsigned short)(Random() & 0x7f) | 4;
        sfxSelect(SFX_WATER_RUNNING, 10000L);

        /* last_pick is never initialised (as in washAtSink), so the first
           comparison reads whatever the slot held.  1985 code, kept on
           purpose. */
        counter = 0;
        while (counter < rnd) {
                if (eventQueue[0] != ACTION_NONE)
                        break;
                val = Random() & 3;
                while (val == last_pick)
                        val = Random() & 3;
                last_pick = val;
                if (val != 3) {
                        animState = scratchArr[val];
                        resFacing = FACING_RIGHT;
                } else {
                        animState = scratchArr[1];
                        resFacing = FACING_LEFT;
                }
                gameTick(1);
                counter++;
        }

        if (sfxPlaying != NO &&
            sfxCurId == SFX_WATER_RUNNING)
                stopSfx();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
