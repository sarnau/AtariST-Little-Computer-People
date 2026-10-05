/* Shared cleanup at exit from any game.  Takes no argument and does
   not free -- the minigame mains free their own buffer inline.
   Nothing calls it, but the original contains it, so it must stay.

   Must sit between mgSetup and textBig. */
static void
gameCleanup()
{
        textTimer = 0;
        keysBlocked = NO;
}
