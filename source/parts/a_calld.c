/*
 * parts/a_calld.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_calld: the resident walks to the couch beside the phone on the
   ground floor, turns side-on and crouches down, then sets pat_ok so
   the player's Ctrl-P "pat" is accepted.  Gives up without crouching
   if the walk is preempted by a new action.  Called for ACTION_CALL_DOG
   and on the way into petting, the couch sit and answering the phone. */
void
a_calld()
{
        /* The walk call is tested in place, with no local; adding
           one would change the compiled code. */

        hs_posXY(POS_BTM_COUCH, &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(5);
        pat_ok = YES;
}
