/* leaveGameTable: leave the game table for an interrupt event (alarm,
   bathroom, thirst, delivery).  Walks the resident to the kitchen
   sink area, tucks away the game-box + table-setting sprites, and
   re-attaches the game-box in the "carried-behind" slot. */

void
leaveGameTable()
{
        short   saveX;
        short   saveY;

        /* typingOff, not keysBlocked: this is the keyboard-input-mode flag
           that handleKey and gameTick test; rejoinTable clears it again. */
        typingOff = YES;
        noPreempt = YES;
        isCarrying = NO;
        resY -= 8;
        resX -= 6;
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkYTarget += 5;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();

        saveX = pendX[spriteSlot[SPRITE_GAME_BOX]];
        saveY = pendY[spriteSlot[SPRITE_GAME_BOX]];
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_GAME_BOX);
        isCarrying = NO;
        pendX[spriteSlot[SPRITE_GAME_BOX]] = saveX;
        pendY[spriteSlot[SPRITE_GAME_BOX]] = saveY;
}
