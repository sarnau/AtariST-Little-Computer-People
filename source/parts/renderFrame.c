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
 *   4. copies the background from the house buffer, by tx_sctm's sign:
 *      < 0 only the letter strip, 0 the whole screen, > 0 split between
 *      the letter scroll region and the game area;
 *   5. promotes pending sprites in the 8 slots and draws the active ones;
 *   6. waits for vsync and flips the page with Setscreen;
 *   7. plays any queued effect through startSfx;
 *   8. toggles the compositing target between the physical screen and
 *      the alternate buffer;
 *   9. bumps ani_cnt.
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
        if (save_vbclock == last_vbc)
                return;
        if (last_vbc + 1 == save_vbclock)
                return;

        last_hz = save_hz200;

        /* --- Dog movement + wander AI --- */
        moveDog();

        if (dg_idlcd < 0 || dg_idlcd > 200)
                dg_idlcd = 5;

        /* Start eating if the dog is at its bowl. */
        if (g_dtx == 0 && g_dty == 0 &&
            lcp_bwlS != BOWL_EMPTY &&
            dg_nrbwl != NO &&
            g_deact == NO &&
            dog_x < 0x14 && dog_y > 0xa0) {
                g_deact    = YES;
                g_decou = rndRng(0x52, 100);
        }

        /* Idle countdown while waiting for a target. */
        if (g_dtx == 0 && g_dty == 0 &&
            dg_idlcd != 0 && g_deact == NO)
                dg_idlcd--;

        /* The dog's target picker, written out inline.  base = s72,
           pick = s66, dest_position = s64. */
        if (g_dtx == 0 && g_dty == 0 &&
            dg_idlcd == 0 && g_deact == NO) {
                if (dg_vis != NO)
                        s72 = 3;
                else
                        s72 = 0;
                do {
                } while ((s66 = rndRng(s72, 8)) == dg_ltgtI);
                posToXY(s64 = g_ddipt[s66], &g_dtx, &g_dty);
                g_dty += g_ddyot[s66];
                g_dtx += g_ddxot[s66];
                dg_ltgtI = s66;
                if (s64 == POS_BTM_STAIR_LANDING)
                        dg_nrbwl = YES;
                dg_idlcd = rndRng(20, 200);
        }

        /* Eating animation cycle. */
        if (g_deact != NO) {
                if (--g_decou == 0) {
                        g_deact    = NO;
                        dg_nrbwl   = NO;
                        dg_bwlch = -1;
                } else {
                        if (g_decou == 60 ||
                            g_decou == 30 ||
                            g_decou == 4)
                                dg_bwlch = -1;
                        else
                                dg_bwlch = 0;
                        g_dsid = g_dseat[
                                g_decou % 3];
                        setDogSprite(g_dsid, 1, NO);
                }
        }

        /* --- SFX chaining --- */
        if (g_sfret > 0) {
                g_sfret--;
                if (g_sfret == 0) {
                        stopSfx();
                        if (g_sfpli == SFX_DOORBELL)
                                sfxSelect(SFX_DOORBELL_ECHO, 5L);
                        if (g_sfpli == SFX_TOILET_FLUSH)
                                sfxSelect(SFX_TOILET_REFILL, 15L);
                }
        }

        /* --- Background copy ---
           Both MFDBs are reached through pointer locals set up here;
           that, and the order of the tx_sctm tests, are the original's. */
        c26 = (char *) &mf_scrp;
        c30 = (char *) &g_srmfd;
        if (tx_sctm > 0) {
                /* Split copy for letter scroll. */
                copyBlocks32(g_dscp,
                          ((MFDB *) c30)->fd_addr, 135);
                copyBlocks32((char *) ((MFDB *) c26)->fd_addr + 4320,
                          (char *) ((MFDB *) c30)->fd_addr + 4320, 865);
                tx_sctm--;
        } else if (tx_sctm < 0) {
                /* Partial (top-strip only). */
                copyBlocks32(g_dscp,
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
                if (g_sepef[index] == YES) {
                        g_sepef[index]  = NO;
                        g_sepex[index]     = g_seacx[index];
                        g_sepey[index]     = g_seacy[index];
                        g_seaim[index]  = g_sepim[index];
                        g_seams[index]   = g_sepms[index];
                        g_seach[index] = g_sepeh[index];
                        g_seacw[index]  = g_sepew[index];
                }
                if (g_seaim[index] != NULL)
                        drawSlot(index);
        }
        /* --- Page flip --- */
        cur_mf = &g_srmfd;
        Vsync();
        Setscreen((void *) -1L, cur_mf->fd_addr, -1);

        if (g_sfacf != NO) {
                startSfx();
                g_sfacf = NO;
        }

        /* Toggle the compositing buffer.  The off-screen target is
           the SAME aligned buffer initMfdb uses: `scrbufA + 0x1FF`
           masked down to a 512-byte boundary, exactly as sprites.c
           does.  Do not write it as `&scrbufA[0x8000]`: Alcyon's int
           is 16-bit, so 0x8000 is -32768 and that points far below
           the buffer. */
        if (cur_mf->fd_addr != sv_phb)
                cur_mf->fd_addr = sv_phb;
        else
                cur_mf->fd_addr =
                        (void *) (((long) scrbufA + 0x1FFL) & ~0x1FFL);

        ani_cnt++;

        /* Second inline _vbclock read: its own pointer local, the
           shared supervisor-stack slot. */
        p_vbc2  = (long *) 0x0462L;
        saveSSP = Super(0L);
        vbc2    = *p_vbc2;
        Super(saveSSP);
        last_vbc = vbc2;
}
