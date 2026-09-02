/*
 * parts/sc_ren8.c -- shared body; LCP_ORG links it in renderf.c,
 * LCP_STX in the 0xdece object (0x15138, in the sprite object ahead of lcp_hwt).  Files under parts/
 * are never compiled standalone.
 */
/* addr: sc_ren8() */

void
sc_ren8()
{
        short   save_hz200;
        long    save_vbclock;
        short   index;

        /* Frame-rate gate. */
        save_hz200   = rd_hz();
        save_vbclock = rd_vbc();
        if ((unsigned short) (save_hz200 - last_hz) <= 24)
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
                dg_idlcd = dg_idlcd - 1;

        if (g_dtx == 0 && g_dty == 0 &&
            dg_idlcd == 0 && g_deact == NO)
                dg_pkTgt();

        /* Eating animation cycle. */
        if (g_deact != NO) {
                g_decou = g_decou - 1;
                if (g_decou == 0) {
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
                g_sfret =
                        g_sfret - 1;
                if (g_sfret == 0) {
                        sf_so();
                        if (g_sfpli == SFX_DOORBELL)
                                sf_sele(SFX_DOORBELL_ECHO, 5L);
                        if (g_sfpli == SFX_TOILET_FLUSH)
                                sf_sele(SFX_TOILET_REFILL, 15L);
                }
        }

        /* --- Background copy --- */
        if (tx_sctm < 1) {
                if (tx_sctm < 0) {
                        /* Partial (top-strip only). */
                        blkcp32(g_dscp,
                                  g_srmfd.fd_addr, 385);
                        blkcp32((char *) mf_scrp.fd_addr + 12320,
                                  (char *) g_srmfd.fd_addr + 12320,
                                  615);
                } else {
                        /* Full-screen. */
                        blkcp32(mf_scrp.fd_addr,
                                  g_srmfd.fd_addr, 1000);
                }
        } else {
                /* Split copy for letter scroll. */
                blkcp32(g_dscp,
                          g_srmfd.fd_addr, 135);
                blkcp32((char *) mf_scrp.fd_addr + 4320,
                          (char *) g_srmfd.fd_addr + 4320, 865);
                tx_sctm = tx_sctm - 1;
        }

        /* --- Sprite compositing --- */
        for (index = 0; index < SPRITE_HW_SLOTS; index = index + 1) {
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
        Setscreen((void *)-1L, cur_mf->fd_addr, -1L);

        if (g_sfacf != NO) {
                sf_irqp();
                g_sfacf = NO;
        }

        /* Toggle compositing buffer between the physbase we started
           with and the alternate.

           Ghidra's screen_render_8hz uses a hardcoded 0x2CA00 as the
           alt buffer.  In the 1985 binary that literal is
           SCREEN_BUFFER_A + 0x19A -- a 32 KB region INSIDE the same
           BSS array that sp_imfs stashes the compositing MFDB at
           (SCREEN_BUFFER_A + 0xCD).  Our port's linker places scrbufA
           at a different BSS address, so 0x2CA00 as a literal lands
           on totally unrelated globals; when blkcp32 writes 32000
           bytes there it silently corrupts our own state (which is
           why the third sc_ren8 iteration crashed in TOS ROM with
           an implausible MFDB pointer).  Compute the same relative
           offset off scrbufA instead. */
        if (cur_mf->fd_addr == sv_phb) {
                long alt = ((long) scrbufA + 0xFFL) & ~0xFFL;   /* ROM: 256-align */
                alt = alt + 0x8000L;
                cur_mf->fd_addr = (void *) alt;
        } else {
                cur_mf->fd_addr = sv_phb;
        }

        ani_cnt = ani_cnt + 1;
        last_vbc = rd_vbc();
}
