/*
 * parts/er_recd.c -- included by stx_u2.c; never compiled on its own.
 * A record delivery: fetch it from the front step and take it upstairs.
 */

void
er_recd()
{
        short   unused;         /* never written, but must stay */

        g_actif = YES;
        wkFrDr();
        /* The pick-up sequence is written out in each delivery
           handler, not factored into a helper. */
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

        sp_ssco(SPRITE_VINYL_CARRY);
        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_VINYL_CARRY] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD; gameTick(2);
        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        lcp_food++;                 /* 1985 typo, kept on purpose */
        g_actif = NO;
}
