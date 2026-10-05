/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* Answer the phone (ACTION_EVENT_PHONE_CALL).  callDog walks the
   resident to the couch beside the phone; he picks up the receiver
   (ph_ans set, ph_call cleared, the ring stopped by tick's ph_hu
   handling) and talks for 40..50 rounds, each a random head frame
   with one of the four chatter effects.  Then he hangs up, crouches,
   waits for a running Ctrl-P pat (g_ptdoa) to finish, clears pat_ok
   and stands side-on; ph_ans is cleared last.  The argument callers
   pass is ignored. */
void
answerPhone()
{
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        callDog();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        waitHeadTurn();

        lcp_y += 6;
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        drawObject(OBJ_PHONE_CALL, PHONE_X, PHONE_Y);

        lcp_st = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = rndRng(0x28, 0x32);
        while (ticks-- != 0) {
                switch (rndRng(0, 2)) {
                case 0:
                        g_hsfra = 5;
                        sfxTvClick();
                        break;
                case 1:
                        g_hsfra = 6;
                        if (rndRng(0, 1) != 0)
                                sfxSpeech();
                        else
                                sfxGreeting();
                        break;
                case 2:
                        g_hsfra = saved_frame;
                        sfxHeadNod();
                        break;
                }
                gameTick(subpick = rndRng(1, 2));
                g_sfret = (long) subpick;
        }

        g_hsfra = saved_frame;
        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        gameTick(1);

        lcp_y -= 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        pat_ok = NO;
        lcp_y -= 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        waitHeadTurn();
        gameTick(0);
        ph_ans = NO;
}
