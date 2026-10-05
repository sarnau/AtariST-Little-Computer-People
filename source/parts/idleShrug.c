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

        pst_arr[0]  = STATE_IDLE_SHRUG_START;
        pst_arr[1]  = STATE_IDLE_SHRUG_HOLD;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();

        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = pst_arr[1]; gameTick(5);
        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = STATE_STAND_SIDE_VIEW; gameTick(0);
}
