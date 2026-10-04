/*
 * parts/exitVdi.c -- included by games.c; never compiled on its own.
 * Restores the screen base saved before a minigame.
 */
void
exitVdi()
{
        Setscreen(sv_lgb, (void *)-1L, -1);     /* rez as word */
}
