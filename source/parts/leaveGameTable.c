/*
 * parts/leaveGameTable.c -- included by stx_u2.c; never compiled on its own.
 */

/* leaveGameTable: leave the game table for an interrupt event (alarm,
   bathroom, thirst, delivery).  Walks the resident to the kitchen
   sink area, tucks away the game-box + table-setting sprites, and
   re-attaches the game-box in the "carried-behind" slot. */

void
leaveGameTable()
{
        short   save_x;
        short   save_y;

        /* g_inpmd, not no_keyin: this is the keyboard-input-mode flag
           that handleKey and gameTick test; rejoinTable clears it again. */
        g_inpmd  = YES;
        g_actif  = YES;
        g_lcyof  = NO;
        lcp_y   -= 8;
        lcp_x   -= 6;
        lcp_st   = STATE_STAND_SIDE_VIEW;
        gameTick(0);
        posToXY(POS_BTM_TABLE_RIGHT, &g_wtx, &g_wty);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        g_wty += 5;
        walkToTarget();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();

        save_x = g_sepex[g_seslm[SPRITE_GAME_BOX]];
        save_y = g_sepey[g_seslm[SPRITE_GAME_BOX]];
        g_selaf[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_GAME_BOX);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_GAME_BOX]] = save_x;
        g_sepey[g_seslm[SPRITE_GAME_BOX]] = save_y;
}
