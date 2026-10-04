/*
 * parts/lcp_lgt.c -- included by stx_u2.c; never compiled on its own.
 */

/* lcp_lgt: leave the game table for an interrupt event (alarm,
   bathroom, thirst, delivery).  Walks the resident to the kitchen
   sink area, tucks away the game-box + table-setting sprites, and
   re-attaches the game-box in the "carried-behind" slot. */

void
lcp_lgt()
{
        short   save_x;
        short   save_y;

        /* g_inpmd, not no_keyin: this is the keyboard-input-mode flag
           that deal_kc and gameTick test; lcp_rgt clears it again. */
        g_inpmd  = YES;
        g_actif  = YES;
        g_lcyof  = NO;
        lcp_y   -= 8;
        lcp_x   -= 6;
        lcp_st   = STATE_STAND_SIDE_VIEW;
        gameTick(0);
        hs_posXY(POS_BTM_TABLE_RIGHT, &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        g_wty += 5;
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();

        save_x = g_sepex[g_seslm[SPRITE_GAME_BOX]];
        save_y = g_sepey[g_seslm[SPRITE_GAME_BOX]];
        g_selaf[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_GAME_BOX);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_GAME_BOX]] = save_x;
        g_sepey[g_seslm[SPRITE_GAME_BOX]] = save_y;
}
