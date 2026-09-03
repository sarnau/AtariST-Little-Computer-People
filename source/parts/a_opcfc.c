/*
 * parts/a_opcfc.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x11d9a, immediately after a_plaag). Files under parts/ are
 * never compiled standalone.
 */

void
a_opcfc()
{
        lcp_st = STATE_BEND_DOWN;         gameTick(1);
        lcp_st = STATE_REACH_FORWARD;     gameTick(2);
        lcp_st = STATE_PICK_UP_FROM_FLOOR;gameTick(2);
        lcp_st = STATE_REACH_FORWARD;
        od_draw(od_fio1, 258, 47);
        gameTick(1);
        lcp_st = STATE_BEND_DOWN;
        od_draw(od_ficl, 258, 47);
        gameTick(1);
        lcp_flcO = NO;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
