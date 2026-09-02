/*
 * parts/stairCyc.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x400c object, with lcp_path).  Files under parts/
 * are never compiled standalone.
 */
/* Advance stair-climb state, clamped at 12 (ROM 0xde66: holds the
   last stair frame; it does NOT wrap back to 9). */
static void
stairCyc()
{
        lcp_st = lcp_st + STATE_WALK_FRAME_1;
        if (lcp_st > STATE_STR_CLIMB_F3S)
                lcp_st = STATE_STR_CLIMB_F3S;
}
