/*
 * Stash the body/head image pointers, NULL them and raise g_lssh.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

void
hideResident()
{
        sv_bodyP  = g_seaim[HW_SLOT_LCP_BODY];
        sv_headP  = g_seaim[HW_SLOT_LCP_HEAD];
        g_seaim[HW_SLOT_LCP_BODY] = NULL;
        g_seaim[HW_SLOT_LCP_HEAD] = NULL;
        g_lssh     = YES;
}
