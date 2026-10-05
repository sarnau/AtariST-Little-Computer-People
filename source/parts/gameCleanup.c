/*
 * Must sit between mgSetup and textBig.
 */

/* Shared cleanup at exit from any game.  Takes no argument and does
   not free -- the minigame mains free their own buffer inline.
   Nothing calls it, but the original contains it, so it must stay. */

static void
gameCleanup()
{
        textTimer  = 0;
        keysBlocked = NO;
}
