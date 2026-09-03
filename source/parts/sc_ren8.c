/*
 * parts/sc_ren8.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x15138, in the sprite object ahead of lcp_hwt). Files under
 * parts/ are never compiled standalone.
 */
/* addr: sc_ren8() */

void
sc_ren8()
{
        /* STX inlines rd_hz/rd_vbc (each Super block keeps its own
           pointer local; the supervisor stack pointer is shared), and
           its frame is -76 -- twenty-one locals, most of them never
           read.  _hz_200's low word lives at $04BC and _vbclock at
           $0462. */
        short           index;          /* -2 */
        long            fill1;          /* -6 */
        long            fill2;          /* -10 */
        long            fill3;          /* -14 */
        short *         p_hz;           /* -18 */
        unsigned short  limit;          /* -20 */
        unsigned short  save_hz200;     /* -22 */
        char *          c26;            /* -26 */
        char *          c30;            /* -30 */
        long            saveSSP;        /* -34 */
        long            fill4;          /* -38 */
        long            fill5;          /* -42 */
        long *          p_vbc;          /* -46 */
        long            save_vbclock;   /* -50 */
        long            vbc2;           /* -54 */
        long            fill6;          /* -58 */
        long *          p_vbc2;         /* -62 */
        short           s64;            /* -64 */
        short           s66;            /* -66 */
        short           fill7;          /* -68 */
        short           fill8;          /* -70 */
        short           s72;            /* -72 */

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
        dg_mvAni();

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

        /* STX inlines the target picker here (the port factored it out
           as dg_pkTgt for readability).  base = s72, pick = s66,
           dest_position = s64. */
        if (g_dtx == 0 && g_dty == 0 &&
            dg_idlcd == 0 && g_deact == NO) {
                if (dg_vis != NO)
                        s72 = 3;
                else
                        s72 = 0;
                do {
                } while ((s66 = rndRng(s72, 8)) == dg_ltgtI);
                hs_posXY(s64 = g_ddipt[s66], &g_dtx, &g_dty);
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
                        sp_spud(g_dsid, 1, NO);
                }
        }

        /* --- SFX chaining --- */
        if (g_sfret > 0) {
                g_sfret--;
                if (g_sfret == 0) {
                        sf_so();
                        if (g_sfpli == SFX_DOORBELL)
                                sf_sele(SFX_DOORBELL_ECHO, 5L);
                        if (g_sfpli == SFX_TOILET_FLUSH)
                                sf_sele(SFX_TOILET_REFILL, 15L);
                }
        }

        /* --- Background copy ---
           STX reaches both MFDBs through pointer locals set up here,
           and tests tx_sctm the other way round. */
        c26 = (char *) &mf_scrp;
        c30 = (char *) &g_srmfd;
        if (tx_sctm > 0) {
                /* Split copy for letter scroll. */
                blkcp32(g_dscp,
                          ((MFDB *) c30)->fd_addr, 135);
                blkcp32((char *) ((MFDB *) c26)->fd_addr + 4320,
                          (char *) ((MFDB *) c30)->fd_addr + 4320, 865);
                tx_sctm--;
        } else if (tx_sctm < 0) {
                /* Partial (top-strip only). */
                blkcp32(g_dscp,
                          ((MFDB *) c30)->fd_addr, 385);
                blkcp32((char *) ((MFDB *) c26)->fd_addr + 12320,
                          (char *) ((MFDB *) c30)->fd_addr + 12320,
                          615);
        } else {
                /* Full-screen. */
                blkcp32(((MFDB *) c26)->fd_addr,
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
                        sp_draw(index);
        }
        /* --- Page flip --- */
        cur_mf = &g_srmfd;
        Vsync();
        Setscreen((void *) -1L, cur_mf->fd_addr, -1);

        if (g_sfacf != NO) {
                sf_irqp();
                g_sfacf = NO;
        }

        /* Toggle the compositing buffer.  STX tests the other way
           round and folds the alternate buffer into ONE relocatable
           constant masked to a 512-byte boundary -- no runtime
           alignment arithmetic and no +0x8000 add. */
        if (cur_mf->fd_addr != sv_phb)
                cur_mf->fd_addr = sv_phb;
        else
                cur_mf->fd_addr =
                        (void *) ((long) &scrbufA[0x8000] & ~0x1FFL);

        ani_cnt++;

        /* Second inlined rd_vbc: its own pointer local, the shared
           supervisor-stack slot. */
        p_vbc2  = (long *) 0x0462L;
        saveSSP = Super(0L);
        vbc2    = *p_vbc2;
        Super(saveSSP);
        last_vbc = vbc2;
}
