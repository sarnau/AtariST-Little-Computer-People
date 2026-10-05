/*
 * parts/panelBegin.c -- included by games.c, right after textNormal; never
 * compiled on its own.
 */
/* Prepare for VDI drawing on the minigame/letter panel: remember the
   current logical screen in panelLogbase, make the panel buffer stripBuf the
   logical screen, and set replace mode and a solid fill in game colour
   12.  panelEnd restores the saved screen. */
void
panelBegin()
{
        panelLogbase = (void *) Logbase();
        Setscreen(stripBuf, (void *)-1L, -1);     /* rez as word */
        vswr_mode(vdiHandle, MD_REPLACE);
        vsf_interior(vdiHandle, FIS_PATTERN);
        vsf_style(vdiHandle, FILL_SOLID);
        vsf_color(vdiHandle, colorPens[COLOR_lt_grey]);
}
