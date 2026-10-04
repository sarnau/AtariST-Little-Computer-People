/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Start drawing on the back screen: remembers the current logical
   screen in g_srlgb, makes g_srptr the logical screen for the VDI, and
   resets the fill attributes (replace mode, solid colour-0 fill).
   Every VDI draw on the house picture is bracketed by this and
   sc_sdtf. */
void
sc_sdtb()
{
        g_srlgb = (void *) Logbase();
        Setscreen(g_srptr, (void *)-1L, -1);
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, 0);
}
