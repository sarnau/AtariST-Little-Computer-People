/* mgSetup: prep the top status strip for the game menu.
   Freezes text-scroll pane and disables keyboard input so keys
   don't leak into the parser while a mini-game is running. */

void
mgSetup()
{
        gameTick(5);
        fillPanel(PANEL_ROWS_GAME);
        textTimer = -1;
        keysBlocked = YES;
}
