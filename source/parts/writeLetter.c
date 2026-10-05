/* ACTION_WRITE_LETTER: the resident types a letter to the player.
   He stops a playing record (stopRecord), fetches paper from the filing
   cabinet, goes through the study door and sits at the typewriter.
   Keyboard input is blocked (keysBlocked), the paper is painted into the
   top panel (fillPanel), and the LETTER.TXT templates are loaded into a
   Malloc'd buffer.  The letter is the date, "Dear <owner>,", 2..4
   paragraphs from the four sections in shuffled order -- the line
   variants chosen by sickness or happiness -- a random sign-off from
   letterSignoffs and his name.  After a 60-tick pause everything is freed, the
   typing sprites hidden and he walks back out through the door. */
void
writeLetter()
{
        /* Declaration order fixes the stack-frame layout, so keep it:
           nine scalars and the section array LAST.  swapA doubles as
           the paragraph count and lineSpacing as the '-' test, and
           every call result is consumed in place. */
        short   sectionId;
        short   i;
        short   swapA;
        short   swapB;
        short   swapTemp;
        short   templateIndex;
        short   cursorY;
        short   lineSpacing;
        short   fullYear;
        short   sectionOrder[4];

        if (recordPlaying != NO)
                stopRecord();

        posToXY(POS_TOP_FILING_CABINET, &walkXTarget, &walkYTarget);
        if (walkToTarget())
                return;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        rummageCabinet();
        if (rndRng(0, 100) > resident.initiativeThreshold)
                closeFilingCab();

        posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
        if (walkToTarget())
                return;

        posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkYTarget += 3;
        if (walkToTarget())
                return;

        noPreempt = YES;

        spriteLayer[SPRITE_TYPEWRITER] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TYPEWRITER);
        pendX[spriteSlot[SPRITE_TYPEWRITER]] = 201;
        pendY[spriteSlot[SPRITE_TYPEWRITER]] = 51;
        spriteLayer[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TYPING_2);
        pendX[spriteSlot[SPRITE_TYPING_2]] = 211;
        pendY[spriteSlot[SPRITE_TYPING_2]] =  44;

        posToXY(POS_TOP_DESK_CHAIR, &walkXTarget, &walkYTarget);
        walkYTarget -= 4;
        walkXTarget -= 14;
        walkToTarget();

        animState = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        resX += 5;
        resY += 6;
        animState = STATE_WRITE_AT_DESK;
        gameTick(1);

        spriteLayer[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_TYPING_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TYPING_1);
        pendX[spriteSlot[SPRITE_TYPING_1]] = 211;
        pendY[spriteSlot[SPRITE_TYPING_1]] =  44;

        headMode = HEAD_ANIM_READING;
        keysBlocked = YES;
        fillPanel(PANEL_ROWS_TEXT);

        letterText = (char *) Malloc((long) LETTER_TEXT_SIZE);
        if (letterText == (char *) 0)
                outOfMemory();
        loadLetterText();

        textTimer = 9999;
        gameTick(2);

        fullYear = t_year + 1900;
        sprintf(inputLine, "%s %d, %4d",
                monthNames[t_mon],
                t_day + 1, fullYear);
        typedCursor = 0;
        typeString(inputLine, -12);
        typeChar('\r');

        sprintf(inputLine, "Dear %s,", resident.ownerName);
        typeString(inputLine, 0);
        typeChar('\r');

        /* Shuffle the 4 section indices via 16 random swaps. */
        for (i = 0; i < 4; i++)
                sectionOrder[i] = i;
        for (i = 0; i < 16; i++) {
                swapA = rndRng(0, 3);
                swapB = rndRng(0, 3);
                swapTemp = sectionOrder[swapA];
                sectionOrder[swapA] = sectionOrder[swapB];
                sectionOrder[swapB] = swapTemp;
        }

        /* Body: 2..4 paragraphs from the shuffled sections. */
        swapA = rndRng(2, 4);
        for (i = 0; i < swapA; i++) {
                sectionId     = sectionOrder[i];
                templateIndex = sectionId * LETTER_SECTION_LINES;
                if (sectionId == 3)
                        templateIndex += rndRng(0, 5) * LETTER_BLOCK_LINES;
                else if (resident.sicknessLevel > SICKNESS_HEALTHY)
                        templateIndex += rndRng(0, 1) * LETTER_HALF_LINES +
                                          LETTER_SICK_BLOCK * LETTER_BLOCK_LINES;
                else
                        templateIndex += rndRng(0, 1) * LETTER_HALF_LINES +
                                          resident.happiness * LETTER_BLOCK_LINES;

                /* Opening line -- indent 5 spaces on the first
                   paragraph only; the whole call is duplicated. */
                if (i == 0)
                        cursorY = typeString(
                                letterLines[rndRng(0, 3) + templateIndex],
                                -5);
                else
                        cursorY = typeString(
                                letterLines[rndRng(0, 3) + templateIndex],
                                2);

                /* Middle line */
                if (cursorY == '-')
                        lineSpacing = 0;
                else
                        lineSpacing = 1;
                cursorY = typeString(
                        letterLines[rndRng(0, 3) + templateIndex + 4],
                        lineSpacing);

                /* Ending line */
                if (cursorY == '-')
                        lineSpacing = 0;
                else
                        lineSpacing = 1;
                typeString(
                        letterLines[rndRng(0, 3) + templateIndex + 8],
                        lineSpacing);
        }

        /* Sign-off. */
        typeChar('\r');
        typeString(letterSignoffs[rndRng(0, 3)], -8);
        typeChar('\r');

        sprintf(inputLine, "%s", resident.characterName);
        typeString(inputLine, -10);
        gameTick(60);

        /* Cleanup: free buffer, hide typing sprites, walk out. */
        textTimer = 0;
        typedCursor = 0;
        keysBlocked = NO;
        Mfree(letterText);

        spriteLayer[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TYPING_2);
        pendX[spriteSlot[SPRITE_TYPING_2]] = 211;
        pendY[spriteSlot[SPRITE_TYPING_2]] =  44;
        gameTick(4);

        animState = STATE_STAND_SIDE_VIEW;
        headMode = HEAD_ANIM_DISABLED;
        resY -= 6;
        gameTick(0);
        noPreempt = YES;

        posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkYTarget += 3;
        walkToTarget();
        posToXY(POS_TOP_STUDY_DOOR, &walkXTarget, &walkYTarget);
        walkToTarget();

        spriteLayer[SPRITE_TYPEWRITER] = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_1]   = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_2]   = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_3]   = SPRITE_HIDDEN;
        spriteLayer[SPRITE_TYPING_4]   = SPRITE_HIDDEN;
        layoutSlots();
        noPreempt = NO;
}
