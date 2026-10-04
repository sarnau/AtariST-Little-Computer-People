/*
 * parts/showLcp.c -- included by stx_u2.c; never compiled on its own.
 */

/* showLcp: restore the pointers hideLcp() stashed. */

void
showLcp()
{
        g_seaim[HW_SLOT_LCP_BODY] = sv_bodyP;
        g_seaim[HW_SLOT_LCP_HEAD] = sv_headP;
        g_lssh     = NO;
}
