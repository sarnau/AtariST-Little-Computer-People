/* Wait to be patted.  Unless patAllowed is already set, the
   resident first goes to the couch and crouches (callDog).  He then
   waits 100..200 ticks (10 during the intro), or until a new action is
   queued, for the player's Ctrl-P; afterwards patAllowed is cleared and he
   stands up. */
void
petDog()
{
        short   ticks;

        noPreempt = YES;
        if (patAllowed == NO)
                callDog();
        noPreempt = NO;

        ticks = rndRng(100, 200);
        if (movingIn != NO)
                ticks = 10;

        /* Pre-decrement loop condition with the break inside: this
           shape is what the original compiles from; keep it. */
        while (--ticks != 0) {
                gameTick(0);
                if (eventQueue[0] != ACTION_NONE)
                        break;
        }

        patAllowed = NO;
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
