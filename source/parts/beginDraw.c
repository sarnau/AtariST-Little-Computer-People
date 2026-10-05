/* Start drawing on the back screen: remembers the current logical
   screen in drawLogbase, makes housePtr the logical screen for the VDI, and
   resets the fill attributes (replace mode, solid colour-0 fill).
   Every VDI draw on the house picture is bracketed by this and
   endDraw.  Launch LCP.PRG directly (desktop or --auto): started from
   COMMAND.PRG, the later Setscreen leaves VDI line attributes invalid,
   vsl_color falls back to pen 15 and the water tank turns brown. */
void
beginDraw()
{
        drawLogbase = (void *) Logbase();
        Setscreen(housePtr, (void *)-1L, -1);
        vswr_mode(vdiHandle, MD_REPLACE);
        vsf_interior(vdiHandle, FIS_PATTERN);
        vsf_style(vdiHandle, FILL_SOLID);
        vsf_color(vdiHandle, 0);
}
