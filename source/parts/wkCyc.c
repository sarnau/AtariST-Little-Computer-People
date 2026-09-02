/*
 * parts/wkCyc.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x400c object, with lcp_path).  Files under parts/
 * are never compiled standalone.
 */
/* Cycle walk state through 0..7. */
static void
wkCyc()
{
        if (lcp_st < STATE_STAND_IDLE) {
                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                if (lcp_st > STATE_WALK_FRAME_7_STEP)
                        lcp_st = STATE_WALK_FRAME_0;
        } else {
                lcp_st = STATE_WALK_FRAME_0;
        }
}
