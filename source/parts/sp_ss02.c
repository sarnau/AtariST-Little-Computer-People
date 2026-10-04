/*
 * parts/sp_ss02.c -- included by stx_u2.c; never compiled on its own.
 */

/* Same as sp_ssco but in the in-front-of-LCP layer. */

void
sp_ss02(g_seix)
short   g_seix;
{
        /* No local for the slot (as in sp_sprs): g_seslm[] is
           re-read at every use. */
        g_selaf[g_seix] = SPRITE_IN_FRONT;
        sp_upds();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}
