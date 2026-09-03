/*
 * parts/mg_stp.c -- shared body; LCP_STX puts mg_stp at 0x759c, after
 * ag_matc.  Files under parts/ are never compiled standalone.
 */
/* mg_stp: prep the top status strip for the game menu.
   Freezes text-scroll pane and disables keyboard input so keys
   don't leak into the parser while a mini-game is running.
   addr: mg_stp() */

void
mg_stp()
{
        gameTick(5);
        fillTopR(0x4d);
        tx_sctm      = -1;
        no_keyin = YES;
}
