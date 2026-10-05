/*
 * parts/showResident.c -- included by stx_u2.c; never compiled on its own.
 */

/* showResident: restore the pointers hideResident() stashed. */

void
showResident()
{
        g_seaim[HW_SLOT_LCP_BODY] = sv_bodyP;
        g_seaim[HW_SLOT_LCP_HEAD] = sv_headP;
        g_lssh     = NO;
}
