/* Study-door save flow: close door, optionally write HYBER, reopen,
   walk resident back to door, close.  Food-count nibble (bits 9..11)
   is preserved via the FE00 mask so the 3-bit delivery counter survives. */
void
studyVisit(doSave, dosndPtr)
BOOL16  doSave;
BOOL16  dosndPtr;
{
        short   savedX;        /* the delay is passed straight to
                                   gameTick, not kept in a local */

        savedX = resX;

        /* Phase 1: door closes (sprite in front of the resident). */
        spriteLayer[SPRITE_DOOR_STUDY_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_1);
        pendX[spriteSlot[SPRITE_DOOR_STUDY_1]] = STUDY_DOOR_X;
        pendY[spriteSlot[SPRITE_DOOR_STUDY_1]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);

        if (dosndPtr != NO)
                sfxSelect(SFX_DOOR_CLOSE, 6L);

        gameTick(1);
        gameTick(rndRng(15, 30));

        /* Phase 2: repack door state and write HYBER. */
        if (doSave != NO) {
                resident.water_level = waterLevel;
                /* Mask in place, then OR the bits back -- lowest shift
                   first, front door last; this order is the original's. */
                resident.door_states_and_flags &= DSF_PRESERVE_UPPER_MASK;
                resident.door_states_and_flags |=
                        (studyDoorOpen     << 1) |
                        (bedClosetOpen    << 2) |
                        (kitchenCabOpen        << 3) |
                        (dresserOpen        << 4) |
                        (toiletDoorOpen    << 5) |
                        (filingCabOpen << 6) |
                        (bowlLevel     << 7) |
                        frontDoorOpen;
                resident.record_playing = recordPlaying;
                resident.tv_on          = tvRunning;
                resident.food_supply    = foodSupply;
                saveFile("hyber", 0x80, &resident);
        }

        /* Phase 3a: door swings ajar. */
        spriteLayer[SPRITE_DOOR_STUDY_1] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_AJAR);
        pendX[spriteSlot[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_X;
        pendY[spriteSlot[SPRITE_DOOR_STUDY_AJAR]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        /* Phase 3b: door wide open, resident visible. */
        spriteLayer[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOOR_STUDY_WIDE_OPEN);
        pendX[spriteSlot[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_X;
        pendY[spriteSlot[SPRITE_DOOR_STUDY_WIDE_OPEN]] = STUDY_DOOR_Y;
        drawObject(OBJ_DOOR_STUDY_OPEN_2, STUDY_DOOR_X, STUDY_DOOR_Y);
        showResident();
        gameTick(1);

        /* Phase 4: walk resident back to the study door. */
        resX = savedX;
        posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();
        noPreempt = NO;

        /* Phase 5: close door, clear the "study door open" flag. */
        if (studyDoorOpen != NO) {
                spriteLayer[SPRITE_DOOR_STUDY_WIDE_OPEN] =
                        SPRITE_HIDDEN;
                layoutSlots();
                gameTick(0);
        }
        drawObject(OBJ_DOOR_STUDY_OPEN_1, STUDY_DOOR_X, STUDY_DOOR_Y);
        gameTick(2);
        drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        studyDoorOpen = NO;
}
