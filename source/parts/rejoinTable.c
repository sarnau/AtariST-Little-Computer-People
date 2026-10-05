/* Reverse of leaveGameTable: walk back to the table and restore the seated
   STATE_EAT_BITE pose with the +8y/+6x offset the minigame overlays
   expect. */

void
rejoinTable()
{
        short   saveX;
        short   saveY;

        noPreempt = YES;
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkXTarget += 6;
        walkYTarget += 2;
        walkToTarget();

        saveX = pendX[spriteSlot[SPRITE_GAME_BOX]];
        saveY = pendY[spriteSlot[SPRITE_GAME_BOX]];
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_GAME_BOX);
        pendX[spriteSlot[SPRITE_GAME_BOX]] = saveX;
        pendY[spriteSlot[SPRITE_GAME_BOX]] = saveY;

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TABLE_SETTING);
        pendX[spriteSlot[SPRITE_TABLE_SETTING]] = 103;
        pendY[spriteSlot[SPRITE_TABLE_SETTING]] = 180;

        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_TABLE_LEFT, &walkXTarget, &walkYTarget);
        walkToTarget();

        animState = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        animState = STATE_EAT_BITE;
        resY += 8;
        resX += 6;
        gameTick(0);
        typingOff = NO;
        noPreempt = NO;
}
