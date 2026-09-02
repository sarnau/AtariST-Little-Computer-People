/*
 * parts/sp_ssco.c -- shared body; LCP_ORG links it in sprites.o,
 * LCP_STX in the 0xdece object (see stx_u2.c).  Files under
 * parts/ are never compiled standalone.
 */

/* sp_ssco: activate sprite as carried object in behind-LCP layer.
   Per-frame X/Y update happens in update_carried_object_sprite().
   addr: sp_ssco() */

void
sp_ssco(g_seix)
short   g_seix;
{
#ifdef FAITHFUL
        short   slot;

        g_selaf[g_seix] = SPRITE_BEHIND_LCP;
        sp_upds();
        slot = g_seslm[g_seix];
        g_seaim[slot]  = g_sedim[g_seix];
        g_seams[slot]   = g_sedms[g_seix];
        g_seach[slot] = g_sedeh[g_seix];
        g_seacw[slot]  = g_sedew[g_seix];
#else
        /* STX has no slot local (as in sp_sprs). */
        g_selaf[g_seix] = SPRITE_BEHIND_LCP;
        sp_upds();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
#endif
        g_lcyof = YES;
        g_lcieo       = g_seix;
}
