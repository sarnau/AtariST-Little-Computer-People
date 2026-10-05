/*
 * Restores the screen base saved before a minigame.
 */
void
panelEnd()
{
        Setscreen(panelLogbase, (void *)-1L, -1);     /* rez as word */
}
