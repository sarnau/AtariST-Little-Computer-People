/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* No `_stksize` is defined: alcyon2's GEMSTART.O, which this program
   links, has the stack size built in, and defining it would add four
   dead bytes at the head of the data segment.

   Program entry.  Sets up MIDI, AES and VDI, silences the key click,
   switches into the DATA folder, prepares the screen buffers and shows
   the title/guestbook screen.  Then it unpacks HOUSE.SCN into the back
   screen, loads the resident's body and outfit sprites, the object and
   sprite tables and the sound effects, draws the water tank, pipe,
   every door in its saved state and the dog bowl, runs the copy
   protection check, and plays the move-in cutscene for a new game
   before handing over to gameLoop, which never returns. */
int
#ifdef HOST
/* The host unit tests each supply their own main(), and this one now
   lives inside stx_u1.o where it cannot be left out of the link.
   Renaming it for the host keeps both linkable; the Atari build is
   untouched. */
lcp_main(argc, argv)
#else
main(argc, argv)
#endif
int     argc;
char ** argv;
{
        /* The conterm clear, the .SCN file handling, and the object and
           sprite loading are all written out inline here, which is why
           this function is so long.  pad1/pad2 are unused but must
           stay, and the declaration order is the original's frame
           layout. */
        short   i;
        short * p;
        short * q;
        short   w;
        short   h;
        short   wpr;
        short   pad1;
        short   pad2;
        short   fhandle;
        char *  conterm;
        long    ssp;
        short   r[4];

        hookTimerA();
        initAes();

        /* Clear bits 0..2 of TOS's `conterm` (key click, key repeat,
           bell) at 0x484, which needs supervisor mode. */
        conterm = (char *) 0x484L;
        ssp = Super(0L);
        *conterm = *conterm & 0xf8;
        Super(ssp);

        /* The data files live in a DATA subdirectory. */
        Dsetpath("data");

        vdiInit();
        initHouseBuf();
        initMirror();
        countSongs();
        loadedSave = loadSavedGame();
        titleScreen();

        /* The .SCN file handling is inlined here; only the nibble
           decoder is a function.  Note the handle is never closed. */
        fhandle = openFile("house.scn", RMODE_RD);
        readFile(fhandle, 2L, &scnSize);
        scnBuffer = (char *) Malloc((long) (scnSize - 32));
        if (scnBuffer == (char *) 0)
                outOfMemory();
        readFile(fhandle, 30L, scnDict);
        readFile(fhandle, (long) (scnSize - 32), scnBuffer);
        decodeScn(scnBuffer, housePtr, 16000);
        Mfree(scnBuffer);

        fillPanel(27);
        drawClock();

        /* body.lcp loads FIRST, then rollResident for a
           new game, then the PEx filename is patched and loaded. */
        loadFrameFile("body.lcp", (unsigned char *) bodyFrames);
        if (loadedSave == 0)
                rollResident();
        pexName[2] = resident.character_sprite_id + '0';
        loadFrameFile(pexName, (unsigned char *) pexFrames);

        buildMasks();

        /* Object and sprite tables: a fixed 56- and 50-iteration walk
           with no zero-record or size check. */
        loadObjects();
        p = (short *) objFileBuf;
        for (i = 0; i < 56; i++) {
                h = *p;
                p++;
                objHeights[i] = h;
                w = *p;
                objWidths[i] = w;
                p++;
                wpr = w / 16;
                if (w % 16)
                        wpr++;
                initMfdb(0L, &objMfdbs[i], p, wpr << 4, h);
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                p = (void *) ((char *) p + ((wpr * h) << 3));
#else
                (char *) p += (wpr * h) << 3;
#endif
        }

        loadSprites();
        p = (short *) sprFileBuf;
        q = (short *) genMaskBuf;
        for (i = 0; i < 50; i++) {
                h = *p;
                p++;
                w = *p;
                p++;
                wpr = w / 16;
                if (w % 16)
                        wpr++;
                defineSprite(spriteFileId[i], p, q, h, wpr << 4);
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                p = (void *) ((char *) p + ((wpr * h) << 3));
#else
                (char *) p += (wpr * h) << 3;
#endif
#ifdef HOST
                /* Alcyon takes a cast as an lvalue and the compound form is what
                   emits `add.l d0,mem`; clang cannot parse it at all.  Same
                   arithmetic, spelled for the host.  See CLAUDE.md. */
                q = (void *) ((char *) q + ((wpr * h) << 3));
#else
                (char *) q += (wpr * h) << 3;
#endif
        }

        loadSounds();
        placeDog();
        if (loadedSave == 0)
                setDogSprite(-1, 1, NO);
        updateWaterTank(0);

        /* Water pipe polyline (147..158, 175): the second point is
           written as offsets from the first. */
        beginDraw();
        r[0] = 147;
        r[1] = 175;
        r[2] = r[0] + 11;
        r[3] = r[1];
        vsl_color(vdiHandle, colorPens[COLOR_grey]);
        v_pline(vdiHandle, 2, r);
        endDraw();

        /* Door / cabinet draws.  HOUSE.SCN has a placeholder rectangle
           where every door and cabinet sits; these paint the open or
           closed object over each one (skip them and the placeholders
           show as streaks).  Each is a full if/else with the whole
           drawObject call duplicated, not a ternary in the argument. */
        if (kitchenCabOpen == NO)
                drawObject(OBJ_CABINET_CLOSED, KITCHEN_CAB_X, KITCHEN_CAB_Y);
        else
                drawObject(OBJ_CABINET_OPEN_2, KITCHEN_CAB_X, KITCHEN_CAB_Y);
        if (frontDoorOpen != NO)
                drawObject(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
        else
                drawObject(OBJ_DOOR_FRONT_CLOSED, FRONT_DOOR_X, FRONT_DOOR_Y);
        if (dresserOpen != NO)
                drawObject(OBJ_DRESSER_OPEN_2, DRESSER_X, DRESSER_Y);
        else
                drawObject(OBJ_DRESSER_CLOSED, DRESSER_X, DRESSER_Y);
        if (bedClosetOpen != NO)
                drawObject(OBJ_DOOR_CLOSET_OPEN_2, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        else
                drawObject(OBJ_DOOR_CLOSET_CLOSED, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        if (studyDoorOpen != NO)
                drawObject(OBJ_DOOR_STUDY_OPEN_2, STUDY_DOOR_X, STUDY_DOOR_Y);
        else
                drawObject(OBJ_DOOR_STUDY_CLOSED, STUDY_DOOR_X, STUDY_DOOR_Y);
        if (toiletDoorOpen != NO)
                drawObject(OBJ_DOOR_TOILET_OPEN_2, TOILET_DOOR_X, TOILET_DOOR_Y);
        else
                drawObject(OBJ_DOOR_TOILET_CLOSED, TOILET_DOOR_X, TOILET_DOOR_Y);
        if (filingCabOpen != NO)
                drawObject(OBJ_FILING_CAB_OPEN_2, FILING_CAB_X, FILING_CAB_Y);
        else
                drawObject(OBJ_FILING_CABINET_CLOSED, FILING_CAB_X, FILING_CAB_Y);

        /* Dog bowl: three explicit state tests with literal frame
           ids, not an index into bowlFrames. */
        if (bowlLevel == BOWL_EMPTY)
                drawObject(OBJ_DOG_FOOD_BOWL_3, DOG_BOWL_X, DOG_BOWL_Y);
        if (bowlLevel == BOWL_HALF)
                drawObject(OBJ_DOG_FOOD_BOWL_2, DOG_BOWL_X, DOG_BOWL_Y);
        if (bowlLevel == BOWL_FULL)
                drawObject(OBJ_DOG_FOOD_BOWL_1, DOG_BOWL_X, DOG_BOWL_Y);

        drawFoodCab();
        resetDailyFlags();
        pickClothes();
#ifdef SKIP_COPYPROT
        /* Test builds only.  checkCopyProt drives the 1772 directly to read
           the protected track, and no emulator here satisfies it: it
           returns 0, moveInScene parks the resident in
           `while (1) dozeOff(SLEEP_RANDOM);` -- which re-runs waitHeadTurn() every
           iteration, so it stands and waves for ever -- and gameLoop
           does the same.  A non-zero copyProtResult is all either test wants;
           the real routine ORs 0xf0000000 into its count.  Skipping
           the CALL rather than the check also avoids the FDC wait,
           which never terminates when the program was launched from a
           drive that is not the floppy.

           This is NOT part of the shipped configuration: the default
           build must stay byte-identical to the original. */
        copyProtResult = 0xf000000aL;
#else
        copyProtResult = checkCopyProt();  /* copy-protection check */
#endif
        initSlots();
        if (loadedSave == 0)
                moveInScene();        /* new game: move-in cutscene */

        /* gameLoop never returns; there is no Pterm here. */
        gameLoop();
}
