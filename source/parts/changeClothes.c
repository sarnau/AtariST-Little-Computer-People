/* changeClothes: change in the bedroom closet.  The resident walks to the
   dresser and opens a drawer (perhaps closing it again on a random
   roll), walks to the closet, opens it and steps inside; the closet
   sprites hide him while the door swings shut.  After 45..60 ticks
   the palette changes -- new clothing colours for value 0, a new skin
   colour otherwise -- unless movingIn is set, and he steps back out.
   The closet is closed after him on a random roll, or always during
   the intro.  Only the first walk can be preempted. */
void
changeClothes(value)
short   value;
{
        short   saved_x;

        posToXY(POS_MID_DRESSER,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        openDresser(DOOR_OPEN);
        if (resident.initiative_threshold < rndRng(0, 100))
                openDresser(DOOR_CLOSE);

        posToXY(POS_MID_BEDROOM_CLOSET,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();
        noPreempt = NO;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        if (bedClosetOpen == NO) {
                resFacing = FACING_LEFT;
                animState = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_CLOSET_CLOSED, CLOSET_DOOR_X, CLOSET_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_CLOSET_OPEN_1, CLOSET_DOOR_X, CLOSET_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_CLOSET_OPEN_2, CLOSET_DOOR_X, CLOSET_DOOR_Y);
                gameTick(2);
                bedClosetOpen = YES;
        }

        /* Walk into the closet. */
        resFacing = FACING_RIGHT;
        spriteLayer[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_CLOSET_WIDE_OPEN);
        pendX[spriteSlot[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        pendY[spriteSlot[SPRITE_CLOSET_WIDE_OPEN]] = 87;

        posToXY(POS_MID_BEDROOM_CLOSET,
                              &walkXTarget, &walkYTarget);
        /* Written as -= so the update goes straight to memory. */
        walkYTarget -= 3;
        walkXTarget -= 10;
        noPreempt = YES;
        walkToTarget();
        noPreempt = NO;                   /* cleared before saving resX, as in the original */
        saved_x = resX;

        /* Close door behind: wide -> ajar -> resident inside. */
        spriteLayer[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_CLOSET_AJAR);
        pendX[spriteSlot[SPRITE_CLOSET_AJAR]] = 75;
        pendY[spriteSlot[SPRITE_CLOSET_AJAR]] = 87;
        drawObject(OBJ_DOOR_CLOSET_OPEN_1, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        gameTick(1);

        spriteLayer[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_CLOSET_LCP_INSIDE);
        hideResident();
        pendX[spriteSlot[SPRITE_CLOSET_LCP_INSIDE]] = 75;
        pendY[spriteSlot[SPRITE_CLOSET_LCP_INSIDE]] = 87;
        drawObject(OBJ_DOOR_CLOSET_CLOSED, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

        gameTick(rndRng(45, 60));
        if (movingIn == NO) {
                if (value == 0)
                        pickClothes();
                else
                        pickSkin();
        }

        /* Open door back up + walk out. */
        spriteLayer[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_CLOSET_AJAR);
        showResident();
        pendX[spriteSlot[SPRITE_CLOSET_AJAR]] = 75;
        pendY[spriteSlot[SPRITE_CLOSET_AJAR]] = 87;
        drawObject(OBJ_DOOR_CLOSET_OPEN_1, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        spriteLayer[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_CLOSET_WIDE_OPEN);
        pendX[spriteSlot[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        pendY[spriteSlot[SPRITE_CLOSET_WIDE_OPEN]] = 87;
        drawObject(OBJ_DOOR_CLOSET_OPEN_2, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        gameTick(1);
        bedClosetOpen = YES;

        resX = saved_x;
        posToXY(POS_MID_BEDROOM_CLOSET,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();
        noPreempt = NO;

        if (bedClosetOpen != NO) {
                spriteLayer[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
                layoutSlots();
                gameTick(0);
        }

        if (resident.initiative_threshold < rndRng(0, 100) ||
            movingIn != NO)
                closeBedCloset();
}
