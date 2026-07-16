/*
 * renderf.c -- sc_ren8, the main 8Hz compositor.
 *
 * Structure:
 *   1. Rate-gate on the 200 Hz clock: skip until at least 25 ticks
 *      (~125 ms) have elapsed since the previous frame AND we've
 *      crossed at least one VBL boundary (prevents double-rendering
 *      when the 200 Hz check races the VBL).
 *   2. Advance dog movement + wander AI (idle countdown, food-bowl
 *      sequence, random-destination pick over 9 waypoints).
 *   3. Time out any long-running SFX (doorbell -> echo, toilet flush
 *      -> refill), advance dog eating animation.
 *   4. Background copy from house buffer to compositing buffer.
 *      Three modes based on text_scroll_timer sign:
 *          <0: partial top-strip copy (letter typewriter panel)
 *          =0: full-screen copy
 *          >0: split copy (letter scroll region + game area)
 *   5. Iterate 8 hardware sprite slots.  Any slot with the pending
 *      flag set gets its pending state promoted to active.  Any slot
 *      with a non-NULL active image gets drawn via sp_draw.
 *   6. Vsync + Setscreen to page-flip.
 *   7. Play any queued SFX via sf_irqp.
 *   8. Toggle the compositing target for the next frame between the
 *      original physbase and the alternate buffer at 0x2CA00.
 *   9. Bump animation_tick_counter.
 *
 * addr: sc_ren8()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    animation_tick_counter;
extern long     g_sfret;
extern short    lcp_dog_bowl_status;
extern short    dog_food_bowl_change;
extern short    g_sfpli;
extern short    text_scroll_timer;
extern void *   g_dscp;
extern BOOL16   g_sfacf;
extern short    last_hz200;
extern long     last_vbclock;
extern void *   save_physbase;
extern unsigned char    scrbufA[];
extern MFDB     g_srmfd;
extern MFDB     MFDB_screen_ptr;        /* alias with older name */
extern MFDB *   current_screen_mfdb;
extern short    g_hzlo;
extern long     _vbclock;
extern BOOL16   dog_visible;
extern short    dog_idle_countdown;
extern BOOL16   dog_near_food_bowl;
extern BOOL16   g_deact;
extern short    g_decou;
extern short    dog_last_target_index;
extern short    g_dseat[];
extern short    g_ddipt[];
extern short    g_ddxot[];
extern short    g_ddyot[];
extern void     house_get_position_xy();
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dsid;
extern short    g_sepef[];
extern short *  g_sepim[];
extern short *  g_sepms[];
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_sepeh[];
extern short    g_sepew[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seacx[];
extern short    g_seacy[];
extern short    g_seach[];
extern short    g_seacw[];
extern short    randomRange();                  /* random.c */
#include <osbind.h>

extern short    randomRange();
extern void     dog_move_and_animate();
extern void     sp_spud();
extern void     sf_so();
extern void     sf_sele();
extern void     sf_irqp();
extern void     blkcopy32();
extern void     sp_draw();

/* Read the 200 Hz clock via GEMDOS Super mode.  Matches Ghidra's
   screen_render_8hz shape:
       saveSSP = _gemdos(Super, 0);
       save_hz200 = _hz_200._2_2_;   // low word of _hz_200 (long)
       save_vbclock = _vbclock;
       _gemdos(Super, saveSSP);
   _hz_200 lives at absolute address $04BA in TOS's low-memory system
   variables (populated by the TOS timer IRQ, no game-side handler
   needed); _vbclock lives at $0462 and is bumped by TOS's VBL IRQ.
   Both must be read in supervisor mode. */

static short
read_hz_200()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) _gemdos(GEMDOS_Super, 0L, 0L, 0L);
        v = *((long *) 0x04BAL);
        _gemdos(GEMDOS_Super, (long) saveSSP, 0L, 0L);
        return (short) (v & 0xFFFFL);
}

static long
read_vbclock()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) _gemdos(GEMDOS_Super, 0L, 0L, 0L);
        v = *((long *) 0x0462L);
        _gemdos(GEMDOS_Super, (long) saveSSP, 0L, 0L);
        return v;
}

/* dog_pick_new_wander_target: choose the next dog destination from a
   9-entry table.  Extracted from sc_ren8 for readability.
   The `dog_visible` flag broadens the acceptable-random range to
   include the food-adjacent positions when the dog is currently
   visible on-screen. */

static void
dog_pick_new_wander_target()
{
        short   base;
        short   pick;
        short   dest_position;

        base = (dog_visible == NO) ? 0 : 3;
        do {
                pick = randomRange(base, 8);
        } while (pick == dog_last_target_index);

        dest_position = g_ddipt[pick];
        house_get_position_xy(dest_position,
                              &g_dtx, &g_dty);
        g_dty = g_ddyot[pick] + g_dty;
        g_dtx = g_ddxot[pick] + g_dtx;

        if (dest_position == POS_BTM_STAIR_LANDING)
                dog_near_food_bowl = YES;

        dog_last_target_index = pick;
        dog_idle_countdown    = randomRange(20, 200);
}

/* sc_ren8: the frame driver.
   addr: sc_ren8() */

void
sc_ren8()
{
        short   save_hz200;
        long    save_vbclock;
        short   index;

        /* Frame-rate gate. */
        save_hz200   = read_hz_200();
        save_vbclock = read_vbclock();
        if ((unsigned short) (save_hz200 - last_hz200) <= 24)
                return;
        if (save_vbclock == last_vbclock)
                return;
        if (last_vbclock + 1 == save_vbclock)
                return;

        last_hz200 = save_hz200;

        /* --- Dog movement + wander AI --- */
        dog_move_and_animate();

        if (dog_idle_countdown < 0 || dog_idle_countdown > 200)
                dog_idle_countdown = 5;

        /* Start eating if the dog is at its bowl. */
        if (g_dtx == 0 && g_dty == 0 &&
            lcp_dog_bowl_status != BOWL_EMPTY &&
            dog_near_food_bowl != NO &&
            g_deact == NO &&
            dog_x < 0x14 && dog_y > 0xa0) {
                g_deact    = YES;
                g_decou = randomRange(0x52, 100);
        }

        /* Idle countdown while waiting for a target. */
        if (g_dtx == 0 && g_dty == 0 &&
            dog_idle_countdown != 0 && g_deact == NO)
                dog_idle_countdown = dog_idle_countdown - 1;

        if (g_dtx == 0 && g_dty == 0 &&
            dog_idle_countdown == 0 && g_deact == NO)
                dog_pick_new_wander_target();

        /* Eating animation cycle. */
        if (g_deact != NO) {
                g_decou = g_decou - 1;
                if (g_decou == 0) {
                        g_deact    = NO;
                        dog_near_food_bowl   = NO;
                        dog_food_bowl_change = -1;
                } else {
                        if (g_decou == 60 ||
                            g_decou == 30 ||
                            g_decou == 4)
                                dog_food_bowl_change = -1;
                        else
                                dog_food_bowl_change = 0;
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
        if (text_scroll_timer < 1) {
                if (text_scroll_timer < 0) {
                        /* Partial (top-strip only). */
                        blkcopy32(g_dscp,
                                  g_srmfd.fd_addr, 385);
                        blkcopy32((char *) MFDB_screen_ptr.fd_addr + 12320,
                                  (char *) g_srmfd.fd_addr + 12320,
                                  615);
                } else {
                        /* Full-screen. */
                        blkcopy32(MFDB_screen_ptr.fd_addr,
                                  g_srmfd.fd_addr, 1000);
                }
        } else {
                /* Split copy for letter scroll. */
                blkcopy32(g_dscp,
                          g_srmfd.fd_addr, 135);
                blkcopy32((char *) MFDB_screen_ptr.fd_addr + 4320,
                          (char *) g_srmfd.fd_addr + 4320, 865);
                text_scroll_timer = text_scroll_timer - 1;
        }

        /* --- Sprite compositing --- */
        for (index = 0; index < 8; index = index + 1) {
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
        current_screen_mfdb = &g_srmfd;
        _xbios(XBIOS_Vsync, 0L, 0L, 0L);
        _xbios(XBIOS_Setscreen,
               -1L, (long) current_screen_mfdb->fd_addr, -1L);

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
           on totally unrelated globals; when blkcopy32 writes 32000
           bytes there it silently corrupts our own state (which is
           why the third sc_ren8 iteration crashed in TOS ROM with
           an implausible MFDB pointer).  Compute the same relative
           offset off scrbufA instead. */
        if (current_screen_mfdb->fd_addr == save_physbase) {
                /* Alt buffer = the SECOND 32K screen inside scrbufA
                   (base is 64K = 2 * 32K to fit both).  Both this
                   address and the compositor's initial address (set
                   in sp_imfs) are 256-aligned so the shifter's
                   Physbase register displays exactly what we
                   composed. */
                long alt = ((long) scrbufA + 0xFFL) & ~0xFFL;
                alt = alt + 0x8000L;
                current_screen_mfdb->fd_addr = (void *) alt;
        } else {
                current_screen_mfdb->fd_addr = save_physbase;
        }

        animation_tick_counter = animation_tick_counter + 1;
        last_vbclock = read_vbclock();
}
