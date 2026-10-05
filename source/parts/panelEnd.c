/*
 * parts/panelEnd.c -- included by games.c; never compiled on its own.
 * Restores the screen base saved before a minigame.
 */
void
panelEnd()
{
        Setscreen(sv_lgb, (void *)-1L, -1);     /* rez as word */
}
