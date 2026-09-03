/*
 * parts/lcp_rgt.c -- shared body; LCP_STX places it at the head of the 0xdece object (it
 * reaches lcp_wkD and sp_ssco there with bsr).
 */

/* lcp_rgt: reverse of lcp_lgt -- restore seated STATE_EAT_BITE pose
   with the +8y/+6x offset expected by mini-game overlays.
   addr: lcp_return_to_game_table() */

void
lcp_rgt()
{
        short   save_x;
        short   save_y;

        g_actif = YES;
        hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        g_wtx += 6;
        g_wty += 2;
        lcp_wkD();

        save_x = g_sepex[g_seslm[SPRITE_GAME_BOX]];
        save_y = g_sepey[g_seslm[SPRITE_GAME_BOX]];
        g_selaf[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_GAME_BOX] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_GAME_BOX);
        g_sepex[g_seslm[SPRITE_GAME_BOX]] = save_x;
        g_sepey[g_seslm[SPRITE_GAME_BOX]] = save_y;

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        hs_posXY(POS_BTM_TABLE_RIGHT, &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_TABLE_LEFT, &g_wtx, &g_wty);
        lcp_wkD();

        lcp_st   = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        g_hatas  = 8;
        lcp_hwt();

        lcp_st = STATE_EAT_BITE;
        lcp_y += 8;
        lcp_x += 6;
        gameTick(0);
        g_inpmd = NO;
        g_actif  = NO;
}
