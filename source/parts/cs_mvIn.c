/*
 * parts/cs_mvIn.c -- included by stx_u2.c; never compiled on its own.
 */

/* cs_mvIn: the new-resident move-in cutscene.  The screen is empty
   while the delivery van pulls up (two p_dobls door-bell blasts), the
   front door opens, the dog is placed on the step, then the resident
   walks in and does a full tour of the house -- dresser, sink, food,
   TV, bed -- before the dog is released and the intro flag drops. */

void
cs_mvIn()
{
        short   unused;         /* never referenced, but must stay */

        dg_init  = 1;
        introSeq = 1;
        hideLcp();
        gameTick(240);
        p_dobls();
        gameTick(80);
        p_dobls();
        gameTick(24);

        /* Front door swings open behind the doorbell sound. */
        od_draw(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(2);
        od_draw(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
        gameTick(2);
        lcp_frdO = 1;

        /* The dog is waiting on the step. */
        g_selaf[SPRITE_DOG_SIT] = 1;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        lcp_x = 300;
        lcp_y = 190;
        showLcp();
        hs_posXY(POS_BTM_SCREEN_EDGE, &g_wtx, &g_wty);
        g_wtx -= 50;
        lcp_wkD();
        lcp_st  = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();
        g_selaf[SPRITE_DOG_SIT] = 0;
        sp_upds();
        gameTick(16);

        /* The protection result gates the game: a failed check parks
           the resident asleep for ever. */
        if (cprot_r == 0)
                while (1)
                        a_sleep(SLEEP_RANDOM);

        hs_posXY(POS_BTM_KITCHEN_CABINET, &g_wtx, &g_wty);
        lcp_wkD();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        lcp_hwt();
        a_opecc(DOOR_OPEN);
        gameTick(16);
        a_opecc(DOOR_CLOSE);

        hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        lcp_wkD();
        gameTick(8);
        a_gesff();
        tt_on();
        lcp_st  = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();
        a_nodok();
        a_opcuc(0);
        a_wakfa();

        hs_posXY(POS_MID_DRESSER, &g_wtx, &g_wty);
        lcp_wkD();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        lcp_hwt();
        a_opcbc(0);
        a_uset();

        hs_posXY(POS_MID_BATHROOM_SINK, &g_wtx, &g_wty);
        lcp_wkD();
        a_gesff();
        a_playc();
        a_tidyh();
        a_wandi();
        tt_off();
        a_chefd(100);
        wkFrDr();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        lcp_hwt();
        a_opcfd(DOOR_OPEN);

        /* Bend down and pick the suitcase up off the step. */
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        sp_ssco(SPRITE_SUITCASE);

        hs_posXY(POS_MID_DRESSER, &g_wtx, &g_wty);
        lcp_wkD();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        g_selaf[SPRITE_SUITCASE] = 0;
        sp_upds();
        g_lcyof = 0;
        lcp_hwt();
        a_opecd(DOOR_OPEN);

        /* Let the dog in and seed its first wander target. */
        hs_posXY(POS_BTM_FRONT_DOOR, &dog_x, &dog_y);
        dog_y = 190;
        dog_x = 273;
        hs_posXY(dg_ltgtI = g_dgitx, &g_dtx, &g_dty);
        g_dty += g_dgiyo;
        g_dyx = g_dtx;
        g_dyy = g_dty;
        dg_stair = 0;
        dg_idlcd = 20;
        dg_init  = 0;
        sp_spud(SPRITE_DOG_LAY_DOWN, -1, 1);

        a_opcbc(0);
        a_opcuc(1);
        introSeq = 0;
}
