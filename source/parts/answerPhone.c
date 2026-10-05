/* Answer the phone (ACTION_EVENT_PHONE_CALL).  crouchForPat walks the
   resident to the armchair beside the phone; he picks up the receiver
   (phoneAnswered set, phoneRinging cleared, the ring stopped by tick's phoneHangUp
   handling) and talks for 40..50 rounds of one or two ticks, each a random head frame
   with one of the four chatter effects.  Then he hangs up, crouches,
   waits for a running Ctrl-P pat (patActive) to finish, clears patAllowed
   and stands side-on; phoneAnswered is cleared last.  It takes no
   parameter, although runEvent passes one (ignored). */
void
answerPhone()
{
        short   savedFrame;
        short   rounds;
        short   subpick;

        noPreempt = YES;
        crouchForPat();
        noPreempt = NO;

        headMode = HEAD_ANIM_DISABLED;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        resY += 6;
        animState = STATE_PHONE_PICKUP;
        gameTick(1);

        phoneAnswered = YES;
        phoneRinging = NO;
        phoneHangUp = YES;
        gameTick(0);
        drawObject(OBJ_PHONE_OFF_HOOK, PHONE_X, PHONE_Y);

        animState = STATE_PHONE_TALKING;
        gameTick(1);

        savedFrame = headFrame;
        headTarget = HEAD_ANIM_DISABLED;
        headPose = HEAD_ANIM_DISABLED;

        rounds = rndRng(40, 50);
        while (rounds-- != 0) {
                switch (rndRng(0, 2)) {
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
                        headFrame = savedFrame;
                        sfxHeadNod();
                        break;
                }
                gameTick(subpick = rndRng(1, 2));
                sfxTicksLeft = (long) subpick;
        }

        headFrame = savedFrame;
        phoneHangUp = YES;
        animState = STATE_PHONE_PICKUP;
        gameTick(1);

        resY -= 6;
        animState = STATE_CROUCH_DOWN;
        gameTick(1);

        while (patActive != NO)
                gameTick(0);

        patAllowed = NO;
        resY -= 2;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headPose = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        animState = STATE_STAND_SIDE_VIEW;
        waitHeadTurn();
        gameTick(0);
        phoneAnswered = NO;
}
