/*
 * parts/sp_ss02.c -- shared body; LCP_STX links it in the 0xdece
 * object at 0x12108 (see stx_u2.c). Files under parts/ are never
 * compiled standalone.
 */

/* sp_ss02: same as sp_ssco but in the in-front-of-LCP layer.
   addr: sp_ss02() */

void
sp_ss02(g_seix)
short   g_seix;
{
        /* STX has no slot local (as in sp_sprs). */
        g_selaf[g_seix] = SPRITE_IN_FRONT;
        sp_upds();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}
