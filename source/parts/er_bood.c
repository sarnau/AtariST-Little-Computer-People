/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Book delivery (ACTION_EVENT_BOOK_DELIVERY).  The resident walks to
   the front door uninterruptibly, opens it, bends down to pick up the
   parcel, and closes the door again when a 0..100 roll beats his
   initiative_threshold.  He then carries the book (SPRITE_BOOK) up to
   the bathroom entrance on the middle floor, the carried sprite is
   dropped (g_lcyof cleared) and he reaches in to put it away. */
void
er_bood()
{
        g_actif = YES;
        wkFrDr();
        /* The pick-up sequence is written out in each handler rather
           than shared through a helper, as in the original. */
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opcfd(DOOR_OPEN);

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        if (lcp.initiative_threshold < rndRng(0, 100))
                a_opcfd(DOOR_CLOSE);

        sp_ssco(SPRITE_BOOK);
        hs_posXY(POS_MID_BATHROOM_ENTRANCE,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_BOOK] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_face     = FACING_RIGHT;
        lcp_st                = STATE_STAND_FACING_SCREEN;
        g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);
        g_actif = NO;
}
