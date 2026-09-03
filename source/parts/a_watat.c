/*
 * parts/a_watat.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * adoors functions, so stx_u2.c includes it in LCP_STX order.
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
        for (i = 0; i < 10; i++) {
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
