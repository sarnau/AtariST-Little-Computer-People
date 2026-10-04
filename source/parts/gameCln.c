/*
 * Must sit between mg_stp and vst_h20.
 *
 * Included by games.c; never compiled on its own.
 */

/* Shared cleanup at exit from any game.  Takes no argument and does
   not free -- the minigame mains free their own buffer inline.
   Nothing calls it, but the original contains it, so it must stay. */

static void
gameCln()
{
        tx_sctm  = 0;
        no_keyin = NO;
}
