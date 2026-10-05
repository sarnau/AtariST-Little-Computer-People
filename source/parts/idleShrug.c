/*
 * parts/idleShrug.c -- included by stx_u2.c; never compiled on its own.
 */

/* ACTION_WANDER_IDLY (also agames.c and the move-in cutscene).
   Despite the name nobody walks: the resident turns side-on, waits
   for his head to settle, and shrugs -- the start pose for 2 ticks,
   held for 5, released -- then stands side-on again. */
void
idleShrug()
{
        /* Unused, but it must stay: removing it changes the compiled
           code. */
        short   unused;

        scratchArr[0]  = STATE_IDLE_SHRUG_START;
        scratchArr[1]  = STATE_IDLE_SHRUG_HOLD;
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = 8;
        waitHeadTurn();

        animState = scratchArr[0]; gameTick(2);
        animState = scratchArr[1]; gameTick(5);
        animState = scratchArr[0]; gameTick(2);
        animState = STATE_STAND_SIDE_VIEW; gameTick(0);
}
