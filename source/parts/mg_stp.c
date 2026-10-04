/*
 * parts/mg_stp.c -- included by games.c; never compiled on its own.
 */
/* mg_stp: prep the top status strip for the game menu.
   Freezes text-scroll pane and disables keyboard input so keys
   don't leak into the parser while a mini-game is running. */

void
mg_stp()
{
        gameTick(5);
        fillTopR(0x4d);
        tx_sctm      = -1;
        no_keyin = YES;
}
