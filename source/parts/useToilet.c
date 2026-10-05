/*
 * Must sit directly before closeToiletDoor.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_USE_TOILET (also from games.c and moveInScene).  The resident
   walks to the bathroom toilet door, opens it if it is shut
   (SFX_DOOR_OPEN), steps in and the door closes behind him in three
   sprite phases while he is hidden (SFX_DOOR_CLOSE).  After 45..60
   ticks the toilet flushes (SFX_TOILET_FLUSH), the door opens again,
   he reappears and walks back out, and leaves it open unless a
   0..100 roll beats his initiative_threshold or the cutscene runs, in
   which case closeToiletDoor shuts it.  Clears resident.bathroom_need and resets
   the bathroom timer. */
void
useToilet()
{
        /* The walk call is tested in place, with no local. */
        short   saved_x;

        posToXY(POS_MID_TOILET_DOOR,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        if (toiletDoorOpen == NO) {
                resFacing = FACING_LEFT;
                animState = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_TOILET_CLOSED, TOILET_DOOR_X, TOILET_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_TOILET_OPEN_1, TOILET_DOOR_X, TOILET_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_TOILET_OPEN_2, TOILET_DOOR_X, TOILET_DOOR_Y);
                gameTick(2);
                toiletDoorOpen = YES;
        }

        resFacing = FACING_RIGHT;
        spriteLayer[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_ANIM_3);
        pendX[spriteSlot[SPRITE_DOOR_ANIM_3]] = 187;
        pendY[spriteSlot[SPRITE_DOOR_ANIM_3]] = 87;

        posToXY(POS_MID_TOILET_DOOR,
                              &walkXTarget, &walkYTarget);
        walkYTarget -= 3;
        walkXTarget -= 10;
        noPreempt = YES;
        walkToTarget();
        saved_x = resX;

        /* Close door behind resident (3 sprite phases). */
        spriteLayer[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_ANIM_2);
        pendX[spriteSlot[SPRITE_DOOR_ANIM_2]] = 187;
        pendY[spriteSlot[SPRITE_DOOR_ANIM_2]] = 87;
        drawObject(OBJ_DOOR_TOILET_OPEN_1, TOILET_DOOR_X, TOILET_DOOR_Y);
        gameTick(1);

        spriteLayer[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_ANIM_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_ANIM_1);
        pendX[spriteSlot[SPRITE_DOOR_ANIM_1]] = 187;
        pendY[spriteSlot[SPRITE_DOOR_ANIM_1]] = 87;
        drawObject(OBJ_DOOR_TOILET_CLOSED, TOILET_DOOR_X, TOILET_DOOR_Y);
        hideResident();
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

        /* 45..60 ticks, then flush + 16 tick refill. */
        gameTick(rndRng(45, 60));       /* no temporary, on purpose */
        sfxSelect(SFX_TOILET_FLUSH, 6L);
        gameTick(16);

        spriteLayer[SPRITE_DOOR_ANIM_1] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_ANIM_2);
        showResident();
        pendX[spriteSlot[SPRITE_DOOR_ANIM_2]] = 187;
        pendY[spriteSlot[SPRITE_DOOR_ANIM_2]] = 87;
        drawObject(OBJ_DOOR_TOILET_OPEN_1, TOILET_DOOR_X, TOILET_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        spriteLayer[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_ANIM_3);
        pendX[spriteSlot[SPRITE_DOOR_ANIM_3]] = 187;
        pendY[spriteSlot[SPRITE_DOOR_ANIM_3]] = 87;
        drawObject(OBJ_DOOR_TOILET_OPEN_2, TOILET_DOOR_X, TOILET_DOOR_Y);
        gameTick(1);
        toiletDoorOpen = YES;

        resX = saved_x;
        posToXY(POS_MID_TOILET_DOOR,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        if (toiletDoorOpen != NO) {
                spriteLayer[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
                layoutSlots();
                gameTick(0);
        }

        if (resident.initiative_threshold < rndRng(0, 100) ||
            movingIn != NO)
                closeToiletDoor();

        resident.bathroom_need  = NO;
        resident.bathroom_timer = BATHROOM_TIMER_OFF;
        noPreempt = NO;
}
