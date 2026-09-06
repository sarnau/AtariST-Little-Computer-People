/*
 * parts/a_socwd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aleisure functions, so stx_u2.c includes it in LCP_STX order.
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
        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        lcp_y += 6;
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        lcp_y -= 3;
        g_selaf[SPRITE_READING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_READING_1);
        g_sepex[g_seslm[SPRITE_READING_1]] = 221;
        g_sepey[g_seslm[SPRITE_READING_1]] = 172;

        ticks = rndRng(30, 50);
        lcp_st = STATE_SIT_COUCH_PETTING_DOG;
        /* STX drives it from a post-decrement with the break in the
           body, and splits the trailing +3 / state assignment the
           same way as the entry sequence. */
        while (ticks--) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(3);
        }

        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        g_selaf[SPRITE_READING_1] = SPRITE_HIDDEN;
        sp_upds();
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        /* STX splits the -9 into two subq steps. */
        lcp_y -= 3;
        lcp_y -= 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(8);
        while (g_ptdoa != NO)
                gameTick(0);

        pat_ok = NO;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        gameTick(1);
}
