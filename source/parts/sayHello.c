/*
 * the resident waves and talks at the screen.
 */

void
sayHello()
{
        /* Declaration order matters: it sets the stack-frame layout,
           which must match the original. */
        short   saved_frame;
        short   pick;
        short   wave_count;
        short   prev_pick;
        short   wait;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headMode = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        saved_frame = headFrame;
        headTarget = HEAD_ANIM_DISABLED;
        headPose = HEAD_ANIM_DISABLED;

        wave_count = rndRng(20, 40);
        /* pick is cleared before prev_pick on purpose (statement order
           shows in the compiled code). */
        pick = 0;
        prev_pick = 0;
        /* Post-decrement in the condition, testing the old value --
           the original's loop shape. */
        while (wave_count--) {
                while (pick == prev_pick)
                        pick = rndRng(0, 2);
                prev_pick = pick;

                /* Must stay a switch: an if/else-if ladder compiles to
                   different code. */
                switch (pick) {
                case 0:
                        headFrame = 5;
                        sfxTvClick();
                        break;
                case 1:
                        headFrame = 6;
                        if (rndRng(0, 1) != 0)
                                sfxSpeech();
                        else
                                sfxGreeting();
                        break;
                case 2:
                        headFrame = 4;
                        sfxHeadNod();
                        break;
                }
                /* The assignment is nested in the call on purpose, so
                   the value is reused from the register. */
                gameTick(wait = rndRng(1, 2));
                sfxTicksLeft = (long) wait;
        }

        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headPose = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headFrame = saved_frame;
        gameTick(0);
}
