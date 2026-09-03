/*
 * parts/showLcp.c -- shared body; LCP_STX links it in the 0xdece
 * object (see stx_u2.c).
 */

/* showLcp: restore the pointers hideLcp() stashed.
   addr: showLcp() */

void
showLcp()
{
        g_seaim[HW_SLOT_LCP_BODY] = sv_bodyP;
        g_seaim[HW_SLOT_LCP_HEAD] = sv_headP;
        g_lssh     = NO;
}
