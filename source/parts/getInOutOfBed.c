/*
 * parts/getInOutOfBed.c -- included by stx_u2.c; never compiled on its own.
 */

/* getInOutOfBed: get into or out of bed, toggling lcp.is_sleeping.  Awake:
   the resident walks to the bed and, unless preempted, undresses,
   gets in and lies down, stepping left as each pose plays.  Asleep:
   the same poses play in reverse, stepping right, and he ends
   standing by the bed.  runAction calls it first whenever an action
   arrives while he is asleep. */
void
getInOutOfBed()
{
        /* The call is tested in place, with no local. */

        pst_arr[0] = STATE_UNDRESS_AT_BED;
        pst_arr[1] = STATE_LIE_DOWN_GETTING_IN;
        pst_arr[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                posToXY(POS_MID_BED,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                waitHeadTurn();
                lcp.is_sleeping = YES;
                lcp_x -= 10;
                lcp_face = FACING_RIGHT;
                lcp_st = pst_arr[0]; gameTick(2);
                lcp_x -= 8;
                lcp_st = pst_arr[1]; gameTick(2);
                lcp_x -= 2;
                lcp_st = pst_arr[2]; gameTick(2);
        } else {
                lcp_face = FACING_RIGHT;
                lcp_x += 10;
                lcp_st = pst_arr[1]; gameTick(2);
                lcp_x += 10;
                lcp_st = pst_arr[0]; gameTick(2);
                lcp.is_sleeping = NO;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                waitHeadTurn();
                gameTick(2);
        }
}
