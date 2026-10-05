/* ACTION_READ_IN_ARMCHAIR: reading in the red armchair by the phone.
   crouchForPat walks the resident there (uninterruptibly, noPreempt) and
   lets him be patted;
   he sits, holds up a book (the SPRITE_OPEN_BOOK_1 prop, drawn over his
   face) and reads for 30..50 three-tick rounds unless a new action is
   queued.  Then he sits up, the prop is hidden, he crouches
   to stand, waits for a running Ctrl-P pat (patActive) to finish, and
   clears patAllowed so the player can no longer pat him. */
void
readInArmchair()
{
        short   ticks;

        noPreempt = YES;
        crouchForPat();
        noPreempt = NO;
        if (eventQueue[0] != ACTION_NONE) {
                animState = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                return;
        }

        /* The +9 is deliberately split into +3 and +6 around the state
           assignment; merging them changes the compiled code. */
        resY += 3;
        animState = STATE_SIT_ARMCHAIR;
        resY += 6;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        gameTick(3);

        resY -= 3;
        spriteLayer[SPRITE_OPEN_BOOK_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_OPEN_BOOK_1);
        pendX[spriteSlot[SPRITE_OPEN_BOOK_1]] = 221;
        pendY[spriteSlot[SPRITE_OPEN_BOOK_1]] = 172;

        ticks = rndRng(30, 50);
        animState = STATE_SIT_ARMCHAIR_READING;
        /* Post-decrement loop with the break in the body, and the
           trailing +3 / state assignment split the same way as the
           entry sequence -- both shapes are what the original compiles
           to. */
        while (ticks--) {
                if (eventQueue[0] != ACTION_NONE)
                        break;
                gameTick(3);
        }

        resY += 3;
        animState = STATE_SIT_ARMCHAIR;
        spriteLayer[SPRITE_OPEN_BOOK_1] = SPRITE_HIDDEN;
        layoutSlots();
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        gameTick(3);

        /* The -9 is deliberately two separate steps. */
        resY -= 3;
        resY -= 6;
        animState = STATE_CROUCH_DOWN;
        gameTick(8);
        while (patActive != NO)
                gameTick(0);

        patAllowed = NO;
        animState = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        gameTick(1);
}
