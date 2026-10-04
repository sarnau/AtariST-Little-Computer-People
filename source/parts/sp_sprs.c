/*
 * parts/sp_sprs.c -- included by stx_u2.c; never compiled on its own.
 */

/* Generic sprite activator (save.c, pet animations).
   Recomputes the 8-slot layout and copies the definition into the
   active slot, bypassing the pending double-buffer. */

void
sp_sprs(g_seix)
short   g_seix;
{
        /* No slot local: the map is subscripted at each use, as in
           the original. */
        sp_upds();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
}
