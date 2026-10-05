/*
 * parts/rejoinTable.c -- included by stx_u2.c near the head of the object;
 * never compiled on its own.
 */

/* Reverse of leaveGameTable: walk back to the table and restore the seated
   STATE_EAT_BITE pose with the +8y/+6x offset the minigame overlays
   expect. */

void
rejoinTable()
{
        short   save_x;
        short   save_y;

        noPreempt = YES;
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkXTarget += 6;
        walkYTarget += 2;
        walkToTarget();

        save_x = pendX[spriteSlot[SPRITE_GAME_BOX]];
        save_y = pendY[spriteSlot[SPRITE_GAME_BOX]];
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_GAME_BOX);
        pendX[spriteSlot[SPRITE_GAME_BOX]] = save_x;
        pendY[spriteSlot[SPRITE_GAME_BOX]] = save_y;

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TABLE_SETTING);
        pendX[spriteSlot[SPRITE_TABLE_SETTING]] = 103;
        pendY[spriteSlot[SPRITE_TABLE_SETTING]] = 180;

        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_TABLE_LEFT, &walkXTarget, &walkYTarget);
        walkToTarget();

        animState   = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        headTarget  = 8;
        waitHeadTurn();

        animState = STATE_EAT_BITE;
        resY += 8;
        resX += 6;
        gameTick(0);
        typingOff = NO;
        noPreempt  = NO;
}
