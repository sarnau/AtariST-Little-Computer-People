/*
 * parts/a_watat.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from adoors.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in adoors.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_watat()
{
        short   i;

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);

        if (lcp_flcO == NO) {
                lcp_flcO = YES;
                lcp_st = STATE_REACH_FORWARD;
                od_draw(od_fio1, 258, 47);
                gameTick(2);
                lcp_st = STATE_PICK_UP_FROM_FLOOR;
                od_draw(od_fio2, 258, 47);
                gameTick(2);
        } else {
                lcp_st = STATE_REACH_FORWARD;
                gameTick(1);
        }

        lcp_st = STATE_STOKE_FIREPLACE;
        gameTick(1);
        /* STX writes i++ here (addq straight to the frame slot). */
#ifdef FAITHFUL
        for (i = 0; i < 10; i = i + 1) {
#else
        for (i = 0; i < 10; i++) {
#endif
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
