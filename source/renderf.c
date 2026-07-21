/*
 * renderf.c -- sc_ren8, the main 8Hz compositor.
 *
 * Structure:
 *   1. Rate-gate on 200 Hz clock: skip until >=25 ticks (~125 ms) elapsed
 *      AND at least one VBL crossed (prevents double-render race).
 *   2. Advance dog movement + wander AI (idle countdown, food-bowl
 *      sequence, random-destination pick over 9 waypoints).
 *   3. Time out long-running SFX (doorbell -> echo, flush -> refill),
 *      advance dog eating animation.
 *   4. Background copy from house buffer, mode by tx_sctm sign:
 *          <0: partial top-strip (letter typewriter panel)
 *          =0: full-screen
 *          >0: split (letter scroll region + game area)
 *   5. Iterate 8 sprite slots: promote pending -> active, draw active.
 *   6. Vsync + Setscreen page-flip.
 *   7. Play queued SFX via sf_irqp.
 *   8. Toggle compositing target between physbase and alt buffer.
 *   9. Bump ani_cnt.
 *
 * addr: sc_ren8()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "dog.h"
#include "gfx_prim.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "renderf.h"
#include "sprglobs.h"
#include "sfx_irq.h"
#include "sound.h"
#include "sprender.h"
#include "sprglobs.h"


/* Read the 200 Hz clock via GEMDOS Super mode.  Matches Ghidra's
   screen_render_8hz shape:
       saveSSP = Super(0);
       save_hz200 = _hz_200._2_2_;   // low word of _hz_200 (long)
       save_vbclock = _vbclock;
       Super(saveSSP);
   _hz_200 lives at absolute address $04BA in TOS's low-memory system
   variables (populated by the TOS timer IRQ, no game-side handler
   needed); _vbclock lives at $0462 and is bumped by TOS's VBL IRQ.
   Both must be read in supervisor mode. */

static short
rd_hz()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) Super(0L);
        v = *((long *) 0x04BAL);
        Super(saveSSP);
        return (short) (v & 0xFFFFL);
}

static long
rd_vbc()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) Super(0L);
        v = *((long *) 0x0462L);
        Super(saveSSP);
        return v;
}

/* dg_pkTgt: pick next dog destination from 9-entry table.
   Extracted from sc_ren8 for readability.
   dg_vis broadens random range to include food-adjacent positions
   when the dog is on-screen. */

static void
dg_pkTgt()
{
        short   base;
        short   pick;
        short   dest_position;

        base = (dg_vis == NO) ? 0 : 3;
        do {
                pick = rndRng(base, 8);
        } while (pick == dg_ltgtI);

        dest_position = g_ddipt[pick];
        hs_posXY(dest_position,
                              &g_dtx, &g_dty);
        g_dty = g_ddyot[pick] + g_dty;
        g_dtx = g_ddxot[pick] + g_dtx;

        if (dest_position == POS_BTM_STAIR_LANDING)
                dg_nrbwl = YES;

        dg_ltgtI = pick;
        dg_idlcd    = rndRng(20, 200);
}

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
        Setscreen((void *)-1L, cur_mf->fd_addr, -1);

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
                long alt = ((long) scrbufA + 0x200L) & ~0x1FFL;
                alt = alt + 0x8000L;
                cur_mf->fd_addr = (void *) alt;
        } else {
                cur_mf->fd_addr = sv_phb;
        }

        ani_cnt = ani_cnt + 1;
        last_vbc = rd_vbc();
}
