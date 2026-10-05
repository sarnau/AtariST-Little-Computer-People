/*
 * parts/mgSetup.c -- included by games.c; never compiled on its own.
 */
/* mgSetup: prep the top status strip for the game menu.
   Freezes text-scroll pane and disables keyboard input so keys
   don't leak into the parser while a mini-game is running. */

void
mgSetup()
{
        gameTick(5);
        fillPanel(0x4d);
        tx_sctm      = -1;
        no_keyin = YES;
}
