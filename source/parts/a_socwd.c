/*
 * parts/a_socwd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_socwd()
{
        short   ticks;

        g_actif = YES;
        a_calld();
        g_actif = NO;
        if (g_trel[0] != ACTION_NONE) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                return;
        }

        /* STX splits the +9 into +3 and +6 around the state
           assignment, and uses += / -= straight to memory. */
#ifdef FAITHFUL
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        lcp_y = lcp_y + 9;
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        lcp_y = lcp_y - 3;
#else
        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        lcp_y += 6;
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        lcp_y -= 3;
#endif
        g_selaf[SPRITE_READING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_READING_1);
        g_sepex[g_seslm[SPRITE_READING_1]] = 221;
        g_sepey[g_seslm[SPRITE_READING_1]] = 172;

        ticks = rndRng(30, 50);
        lcp_st = STATE_SIT_COUCH_PETTING_DOG;
        /* STX drives it from a post-decrement with the break in the
           body, and splits the trailing +3 / state assignment the
           same way as the entry sequence. */
#ifdef FAITHFUL
        while (ticks != 0 && g_trel[0] == ACTION_NONE) {
                gameTick(3);
                ticks = ticks - 1;
        }

        lcp_y = lcp_y + 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
#else
        while (ticks--) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(3);
        }

        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
#endif
        g_selaf[SPRITE_READING_1] = SPRITE_HIDDEN;
        sp_upds();
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        /* STX splits the -9 into two subq steps. */
#ifdef FAITHFUL
        lcp_y = lcp_y - 9;
#else
        lcp_y -= 3;
        lcp_y -= 6;
#endif
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(8);
        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        gameTick(1);
}
