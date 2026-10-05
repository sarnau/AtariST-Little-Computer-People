/*
 * parts/moveInScene.c -- included by stx_u2.c; never compiled on its own.
 */

/* moveInScene: the new-resident move-in cutscene.  The screen is empty
   while the delivery van pulls up (two playDoorbell door-bell blasts), the
   front door opens, the dog is placed on the step, then the resident
   walks in and does a full tour of the house -- dresser, sink, food,
   TV, bed -- before the dog is released and the intro flag drops. */

void
moveInScene()
{
        short   unused;         /* never referenced, but must stay */

        dg_init  = 1;
        introSeq = 1;
        hideResident();
        gameTick(240);
        playDoorbell();
        gameTick(80);
        playDoorbell();
        gameTick(24);

        /* Front door swings open behind the doorbell sound. */
        drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(2);
        drawObject(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
        gameTick(2);
        lcp_frdO = 1;

        /* The dog is waiting on the step. */
        g_selaf[SPRITE_DOG_SIT] = 1;
        activateSprite(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        lcp_x = 300;
        lcp_y = 190;
        showResident();
        posToXY(POS_BTM_SCREEN_EDGE, &g_wtx, &g_wty);
        g_wtx -= 50;
        walkToTarget();
        lcp_st  = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();
        g_selaf[SPRITE_DOG_SIT] = 0;
        layoutSlots();
        gameTick(16);

        /* The protection result gates the game: a failed check parks
           the resident asleep for ever. */
        if (cprot_r == 0)
                while (1)
                        dozeOff(SLEEP_RANDOM);

        posToXY(POS_BTM_KITCHEN_CABINET, &g_wtx, &g_wty);
        walkToTarget();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        waitHeadTurn();
        openKitchenCab(DOOR_OPEN);
        gameTick(16);
        openKitchenCab(DOOR_CLOSE);

        posToXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        walkToTarget();
        gameTick(8);
        goToFridge();
        tvOn();
        lcp_st  = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();
        nodOk();
        enterStudy(0);
        wakeFromAlarm();

        posToXY(POS_MID_DRESSER, &g_wtx, &g_wty);
        walkToTarget();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        waitHeadTurn();
        changeClothes(0);
        useToilet();

        posToXY(POS_MID_BATHROOM_SINK, &g_wtx, &g_wty);
        walkToTarget();
        goToFridge();
        useComputer();
        tidyHouse();
        idleShrug();
        tvOff();
        checkFrontDoor(100);
        walkToFrontDoor();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);

        /* Bend down and pick the suitcase up off the step. */
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        carryBehind(SPRITE_SUITCASE);

        posToXY(POS_MID_DRESSER, &g_wtx, &g_wty);
        walkToTarget();
        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = 12;
        g_selaf[SPRITE_SUITCASE] = 0;
        layoutSlots();
        g_lcyof = 0;
        waitHeadTurn();
        openDresser(DOOR_OPEN);

        /* Let the dog in and seed its first wander target. */
        posToXY(POS_BTM_FRONT_DOOR, &dog_x, &dog_y);
        dog_y = 190;
        dog_x = 273;
        posToXY(dg_ltgtI = g_dgitx, &g_dtx, &g_dty);
        g_dty += g_dgiyo;
        g_dyx = g_dtx;
        g_dyy = g_dty;
        dg_stair = 0;
        dg_idlcd = 20;
        dg_init  = 0;
        setDogSprite(SPRITE_DOG_LAY_DOWN, -1, 1);

        changeClothes(0);
        enterStudy(1);
        introSeq = 0;
}
