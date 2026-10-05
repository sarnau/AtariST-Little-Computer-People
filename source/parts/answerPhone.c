/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* Answer the phone (ACTION_EVENT_PHONE_CALL).  callDog walks the
   resident to the couch beside the phone; he picks up the receiver
   (phoneAnswered set, phoneRinging cleared, the ring stopped by tick's phoneHangUp
   handling) and talks for 40..50 rounds, each a random head frame
   with one of the four chatter effects.  Then he hangs up, crouches,
   waits for a running Ctrl-P pat (patActive) to finish, clears patAllowed
   and stands side-on; phoneAnswered is cleared last.  The argument callers
   pass is ignored. */
void
answerPhone()
{
        short   saved_frame;
        short   ticks;
        short   subpick;

        noPreempt = YES;
        callDog();
        noPreempt = NO;

        headMode         = HEAD_ANIM_DISABLED;
        headTarget = 8;
        waitHeadTurn();

        resY += 6;
        animState = STATE_PHONE_PICKUP;
        gameTick(1);

        phoneAnswered    = YES;
        phoneRinging = NO;
        phoneHangUp      = YES;
        gameTick(0);
        drawObject(OBJ_PHONE_CALL, PHONE_X, PHONE_Y);

        animState = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = headFrame;
        headTarget = HEAD_ANIM_DISABLED;
        headPose      = HEAD_ANIM_DISABLED;

        ticks = rndRng(40, 50);
        while (ticks-- != 0) {
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
                        headFrame = saved_frame;
                        sfxHeadNod();
                        break;
                }
                gameTick(subpick = rndRng(1, 2));
                sfxTicksLeft = (long) subpick;
        }

        headFrame = saved_frame;
        phoneHangUp = YES;
        animState         = STATE_PHONE_PICKUP;
        gameTick(1);

        resY -= 6;
        animState = STATE_CROUCH_DOWN;
        gameTick(1);

        while (patActive != NO)
                gameTick(0);

        patAllowed = NO;
        resY -= 2;
        headTarget = 8;
        headPose      = 8;
        animState = STATE_STAND_SIDE_VIEW;
        waitHeadTurn();
        gameTick(0);
        phoneAnswered = NO;
}
