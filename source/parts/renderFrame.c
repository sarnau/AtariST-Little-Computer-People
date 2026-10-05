/*
 * parts/renderFrame.c -- the per-frame compositor: throttles itself to the
 * 200 Hz and VBL clocks, runs the dog AI and SFX chaining, copies the
 * background, draws the sprites and flips the screen.  Included by
 * stx_u3.c, ahead of waitHeadTurn; never compiled on its own.
 *
 * Each call:
 *   1. returns early unless 25 ticks of the 200 Hz clock (~125 ms) have
 *      passed AND at least one VBL has crossed, so it never renders
 *      twice in one frame;
 *   2. moves the dog and runs its wander AI (idle countdown, food-bowl
 *      sequence, random pick among nine waypoints);
 *   3. times out long sound effects (doorbell -> echo, flush -> refill)
 *      and advances the dog's eating animation;
 *   4. copies the background from the house buffer, by textTimer's sign:
 *      < 0 only the letter strip, 0 the whole screen, > 0 split between
 *      the letter scroll region and the game area;
 *   5. promotes pending sprites in the 8 slots and draws the active ones;
 *   6. waits for vsync and flips the page with Setscreen;
 *   7. plays any queued effect through startSfx;
 *   8. toggles the compositing target between the physical screen and
 *      the alternate buffer;
 *   9. bumps frameCount.
 */
void
renderFrame()
{
        /* The two system-clock reads are written out inline: each
           Super block keeps its own pointer local, and the supervisor
           stack pointer slot is shared.  Most of these twenty-one
           locals are never read, but every one must stay, in this
           order -- they set the stack frame.  _hz_200's low word lives
           at $04BC and _vbclock at $0462. */
        short           index;
        long            fill1;
        long            fill2;
        long            fill3;
        short *         p_hz;
        unsigned short  limit;
        unsigned short  save_hz200;
        char *          c26;
        char *          c30;
        long            saveSSP;
        long            fill4;
        long            fill5;
        long *          p_vbc;
        long            save_vbclock;
        long            vbc2;
        long            fill6;
        long *          p_vbc2;
        short           s64;
        short           s66;
        short           fill7;
        short           fill8;
        short           s72;

        /* Frame-rate gate. */
        p_hz    = (short *) 0x04BCL;
        p_vbc   = (long *) 0x0462L;
        saveSSP = Super(0L);
        save_hz200   = *p_hz;
        save_vbclock = *p_vbc;
        Super(saveSSP);
        limit = last_hz + 25;
        if (save_hz200 - last_hz < 25)
                return;
        if (save_vbclock == lastFrameVbl)
                return;
        if (lastFrameVbl + 1 == save_vbclock)
                return;

        last_hz = save_hz200;

        /* --- Dog movement + wander AI --- */
        moveDog();

        if (dogIdleCount < 0 || dogIdleCount > 200)
                dogIdleCount = 5;

        /* Start eating if the dog is at its bowl. */
        if (dogXTarget == 0 && dogYTarget == 0 &&
            bowlLevel != BOWL_EMPTY &&
            dogMayEat != NO &&
            dogEating == NO &&
            dogX < 0x14 && dogY > 0xa0) {
                dogEating    = YES;
                dogEatCount = rndRng(0x52, 100);
        }

        /* Idle countdown while waiting for a target. */
        if (dogXTarget == 0 && dogYTarget == 0 &&
            dogIdleCount != 0 && dogEating == NO)
                dogIdleCount--;

        /* The dog's target picker, written out inline.  base = s72,
           pick = s66, dest_position = s64. */
        if (dogXTarget == 0 && dogYTarget == 0 &&
            dogIdleCount == 0 && dogEating == NO) {
                if (dogNoTopFlr != NO)
                        s72 = 3;
                else
                        s72 = 0;
                do {
                } while ((s66 = rndRng(s72, 8)) == dogLastPick);
                posToXY(s64 = dogRoamSpots[s66], &dogXTarget, &dogYTarget);
                dogYTarget += dogYNudge[s66];
                dogXTarget += dogXNudge[s66];
                dogLastPick = s66;
                if (s64 == POS_BTM_STAIR_LANDING)
                        dogMayEat = YES;
                dogIdleCount = rndRng(20, 200);
        }

        /* Eating animation cycle. */
        if (dogEating != NO) {
                if (--dogEatCount == 0) {
                        dogEating    = NO;
                        dogMayEat   = NO;
                        bowlChange = -1;
                } else {
                        if (dogEatCount == 60 ||
                            dogEatCount == 30 ||
                            dogEatCount == 4)
                                bowlChange = -1;
                        else
                                bowlChange = 0;
                        dogSpriteId = dogEatFrames[
                                dogEatCount % 3];
                        setDogSprite(dogSpriteId, 1, NO);
                }
        }

        /* --- SFX chaining --- */
        if (sfxTicksLeft > 0) {
                sfxTicksLeft--;
                if (sfxTicksLeft == 0) {
                        stopSfx();
                        if (sfxCurId == SFX_DOORBELL)
                                sfxSelect(SFX_DOORBELL_ECHO, 5L);
                        if (sfxCurId == SFX_TOILET_FLUSH)
                                sfxSelect(SFX_TOILET_REFILL, 15L);
                }
        }

        /* --- Background copy ---
           Both MFDBs are reached through pointer locals set up here;
           that, and the order of the textTimer tests, are the original's. */
        c26 = (char *) &houseMfdb;
        c30 = (char *) &frameMfdb;
        if (textTimer > 0) {
                /* Split copy for letter scroll. */
                copyBlocks32(stripBuf,
                          ((MFDB *) c30)->fd_addr, 135);
                copyBlocks32((char *) ((MFDB *) c26)->fd_addr + 4320,
                          (char *) ((MFDB *) c30)->fd_addr + 4320, 865);
                textTimer--;
        } else if (textTimer < 0) {
                /* Partial (top-strip only). */
                copyBlocks32(stripBuf,
                          ((MFDB *) c30)->fd_addr, 385);
                copyBlocks32((char *) ((MFDB *) c26)->fd_addr + 12320,
                          (char *) ((MFDB *) c30)->fd_addr + 12320,
                          615);
        } else {
                /* Full-screen. */
                copyBlocks32(((MFDB *) c26)->fd_addr,
                          ((MFDB *) c30)->fd_addr, 1000);
        }

        /* --- Sprite compositing --- */
        for (index = 0; index < SPRITE_HW_SLOTS; index++) {
                if (pendReady[index] == YES) {
                        pendReady[index]  = NO;
                        pendX[index]     = drawnX[index];
                        pendY[index]     = drawnY[index];
                        drawnImage[index]  = pendImage[index];
                        drawnMask[index]   = pendMask[index];
                        drawnHeight[index] = pendHeight[index];
                        drawnWidth[index]  = pendWidth[index];
                }
                if (drawnImage[index] != NULL)
                        drawSlot(index);
        }
        /* --- Page flip --- */
        flipMfdb = &frameMfdb;
        Vsync();
        Setscreen((void *) -1L, flipMfdb->fd_addr, -1);

        if (sfxPending != NO) {
                startSfx();
                sfxPending = NO;
        }

        /* Toggle the compositing buffer.  The off-screen target is
           the SAME aligned buffer initMfdb uses: `altScreen + 0x1FF`
           masked down to a 512-byte boundary, exactly as sprites.c
           does.  Do not write it as `&altScreen[0x8000]`: Alcyon's int
           is 16-bit, so 0x8000 is -32768 and that points far below
           the buffer. */
        if (flipMfdb->fd_addr != tosPhysbase)
                flipMfdb->fd_addr = tosPhysbase;
        else
                flipMfdb->fd_addr =
                        (void *) (((long) altScreen + 0x1FFL) & ~0x1FFL);

        frameCount++;

        /* Second inline _vbclock read: its own pointer local, the
           shared supervisor-stack slot. */
        p_vbc2  = (long *) 0x0462L;
        saveSSP = Super(0L);
        vbc2    = *p_vbc2;
        Super(saveSSP);
        lastFrameVbl = vbc2;
}
