/*
 * parts/sp_sprs.c -- shared body; LCP_ORG links it in sprites.o,
 * LCP_STX in the 0xdece object (see stx_u2.c).  Files under
 * parts/ are never compiled standalone.
 */

/* sp_sprs: generic sprite activator (save.c, pet animations).
   Recomputes 8-slot layout, copies definition into active slot.
   Bypasses the pending double-buffer.
   addr: sp_sprs() */

void
sp_sprs(g_seix)
short   g_seix;
{
#ifdef FAITHFUL
        short   slot;

        sp_upds();
        slot = g_seslm[g_seix];
        g_seaim[slot]  = g_sedim[g_seix];
        g_seams[slot]   = g_sedms[g_seix];
        g_seach[slot] = g_sedeh[g_seix];
        g_seacw[slot]  = g_sedew[g_seix];
#else
        /* STX has no slot local: the map is subscripted at each use. */
        sp_upds();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
#endif
}
