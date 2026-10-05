/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* danceToMusic: dance to a record.  If none is playing (recordPlaying) one is
   started with playRecord first.  The resident then walks to the dance
   floor and alternates the left/right dance-step poses every two
   ticks for as long as music plays (songPlaying), stopping early when a
   new action is queued in eventQueue[0]. */
void
danceToMusic()
{
        /* One local (the loop counter); the walk result is tested
           in place. */
        short   i;

        scratchArr[0] = STATE_DANCE_STEP_LEFT;
        scratchArr[1] = STATE_DANCE_STEP_RIGHT;

        if (recordPlaying == NO) {
                noPreempt = YES;
                playRecord();
        }
        noPreempt = NO;

        posToXY(POS_TOP_DANCE_FLOOR,
                              &walkXTarget, &walkYTarget);
        walkYTarget += 8;
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = 8;
        waitHeadTurn();

        /* i is never initialised -- the first iteration reads
           whatever the frame slot held.  Kept as in the original. */
        while (songPlaying != NO) {
                i++;
                animState = scratchArr[i & 1];
                if (eventQueue[0] != ACTION_NONE)
                        break;
                gameTick(2);
        }

        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
