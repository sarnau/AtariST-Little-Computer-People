/*
 * parts/hideLcp.c -- shared body; LCP_ORG links it in sprites.o,
 * LCP_STX in the 0xdece object (see stx_u2.c).
 */

/* hideLcp: stash body/head image pointers, NULL them, raise g_lssh.
   addr: hideLcp() */

void
hideLcp()
{
        sv_bodyP  = g_seaim[HW_SLOT_LCP_BODY];
        sv_headP  = g_seaim[HW_SLOT_LCP_HEAD];
        g_seaim[HW_SLOT_LCP_BODY] = NULL;
        g_seaim[HW_SLOT_LCP_HEAD] = NULL;
        g_lssh     = YES;
}
