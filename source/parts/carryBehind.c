/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* carryBehind: activate sprite as carried object in behind-LCP layer.
   The per-frame X/Y update happens in gameTick's carrying path. */

void
carryBehind(g_seix)
short   g_seix;
{
        g_selaf[g_seix] = SPRITE_BEHIND_LCP;
        layoutSlots();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}
