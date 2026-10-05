/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Start drawing on the back screen: remembers the current logical
   screen in g_srlgb, makes g_srptr the logical screen for the VDI, and
   resets the fill attributes (replace mode, solid colour-0 fill).
   Every VDI draw on the house picture is bracketed by this and
   endDraw.  Launch LCP.PRG directly (desktop or --auto): started from
   COMMAND.PRG, the later Setscreen leaves VDI line attributes invalid,
   vsl_color falls back to pen 15 and the water tank turns brown. */
void
beginDraw()
{
        g_srlgb = (void *) Logbase();
        Setscreen(g_srptr, (void *)-1L, -1);
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, 0);
}
