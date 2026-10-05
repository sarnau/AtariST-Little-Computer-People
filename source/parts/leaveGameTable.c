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

        /* typingOff, not keysBlocked: this is the keyboard-input-mode flag
           that handleKey and gameTick test; rejoinTable clears it again. */
        typingOff  = YES;
        noPreempt  = YES;
        isCarrying  = NO;
        resY   -= 8;
        resX   -= 6;
        animState   = STATE_STAND_SIDE_VIEW;
        gameTick(0);
        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkYTarget += 5;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();

        save_x = pendX[spriteSlot[SPRITE_GAME_BOX]];
        save_y = pendY[spriteSlot[SPRITE_GAME_BOX]];
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_GAME_BOX);
        isCarrying = NO;
        pendX[spriteSlot[SPRITE_GAME_BOX]] = save_x;
        pendY[spriteSlot[SPRITE_GAME_BOX]] = save_y;
}
