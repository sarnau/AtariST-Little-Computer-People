/*
 * parts/panelBegin.c -- included by games.c, right after textNormal; never
 * compiled on its own.
 */
/* Prepare for VDI drawing on the minigame/letter panel: remember the
   current logical screen in sv_lgb, make the panel buffer g_dscp the
   logical screen, and set replace mode and a solid fill in game colour
   12.  panelEnd restores the saved screen. */
void
panelBegin()
{
        sv_lgb = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1);     /* rez as word */
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, vdi_colt[COLOR_lt_grey]);
}
