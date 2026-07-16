/*
 * tick.c -- main frame driver.
 *
 * gameTick(counter) is the workhorse of gameLoop.
 * Ghidra 0x256A6.  Two entry paths:
 *
 *   g_lcyof == NO:  loop counter+1 frames.  Each frame:
 *     - wait for the next 8 Hz render tick (sc_ren8 spin)
 *     - advance subAniC, redraw clock pendulum
 *     - gameSim1, cl_redrH
 *     - petting-dog animation cycle (11 frames of head-pat sprites)
 *     - dog food bowl countdown + redraw
 *     - fire animation (4 frames while fire active) + auto-extinguish
 *     - alarm-clock animation + SFX loop
 *     - phone-ring animation + SFX loop
 *     - record-player needle animation (rp_anim)
 *     - TV static-noise animation (td_nois)
 *     - LCP body + head sprite update (sp_updb / sp_lcha / sp_lchu)
 *     - keyboard input dispatch (deal_kc)
 *     - screen scroll if a text-scroll countdown is active
 *     - final sc_ren8
 *
 *   g_lcyof != NO:  reposition the currently-carried object sprite
 *     (X = lcp_x + 10 facing right, or lcp_x - width + 16 facing left),
 *     then dispatch to a per-carried-object Y-offset handler by
 *     walking g_cotbl until the sprite id matches.
 *
 * addr: gameTick()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"

/* --- per-file extern block (auto-generated for Alcyon). */
extern short    ani_cnt;
extern short    lcp_x, lcp_y;
extern short    lcp_face;
extern short    lcp_st;
extern short    lcp_bwlS;
extern short    g_lcyof;                        /* lcp_carrying_object_flag */
extern short    g_lcieo;                        /* lcp_carried_object */
extern short    lcp_recP;
extern short    lcp_tv;
extern BOOL16   ph_call;
extern BOOL16   ph_hu;
extern short    g_phrc;
extern BOOL16   alarm_p;
extern BOOL16   g_alsts;
extern BOOL16   fire_act;
extern short    fire_dur;
extern BOOL16   fire_ext;
extern BOOL16   g_ptdoa;                        /* petting_dog_active */
extern short    g_ptanf;
extern short    g_ptlss;
extern short    dg_bwlch;
extern short    tx_sctm;
extern short    g_srsdc;
extern short    g_cdibp;
extern BOOL16   no_keyin;
extern BOOL16   introSeq;
extern BOOL16   g_inpmd;
extern short    subAniC;
extern short    g_sepex[], g_sepey[];
extern short    g_seacw[];
extern short    g_selaf[];                      /* sprite_layer_flags */
extern short    g_seslm[];                      /* sprite_slot_map */
extern short    g_sfacf;                        /* soundeffect_active_flag */
extern short    g_sfpli;                        /* soundeffect_playing_id */
extern short    g_obcla[];                      /* clock_animation[4] */
extern short    g_obdea[];                      /* dog_eating_animation[3] */
extern short    g_obala[];                      /* alarm_animation[2] */
extern short    g_obpha[];                      /* phone_animation[4] */
extern short    g_obfia[];                      /* fire_animation[4] */
extern short    g_ptdsi[];                      /* petting sprite ids[11] */
extern long     g_cotbl[];                      /* carried_object_id_table[10] */
extern short    g_obifo;                        /* object_id_fireplace_off */

extern void     sp_updb();
extern void     sp_lcha();
extern void     sp_lchu();
extern void     sc_ren8();
extern void     cl_redrH();
extern void     od_draw();
extern void     gameSim1();
extern void     sf_sele();                      /* soundeffect_select */
extern void     sf_so();                        /* soundeffects_off */
extern void     sp_sprs();                      /* spritedata_select */
extern void     sp_upds();                      /* sprite_update_slots */
extern void     rp_anim();                      /* record_player_animate_needle */
extern void     td_nois();                      /* tv_draw_static_noise */
extern void     fillTopR();
extern void     sc_sctd();                      /* screen_scroll_text_down */
extern short    getKey();
extern void     deal_kc();

/* Object ids referenced by gameTick as bare literals in
   the Ghidra disassembly (no port-side global covers them).  The
   values come from the OBJECTS-file record indices for the phone and
   the fire-off sprites. */
#define OBJ_PHONE_2             23      /* Ghidra literal 0x17 */

/* Carried-object per-frame Y offset (Ghidra 0x257c6..0x258b0 jump
   table).  Every listed sprite id uses -20; the default is "no update". */
static short
cy_yoff(id)
short   id;
{
        switch (id) {
        case 3:                 /* SPRITE_GLASS        */
        case 4:                 /* SPRITE_GAME_BOX     */
        case 9:                 /* SPRITE_FOOD_PACKAGE */
        case 22:                /* SPRITE_FIREWOOD     */
        case 23:                /* SPRITE_COOKING_POT  */
        case 48:                /* SPRITE_SUITCASE     */
        case 49:                /* SPRITE_BOOK         */
        case 50:                /* SPRITE_VINYL_CARRY  */
        case 55:                /* SPRITE_55           */
                return -20;
        }
        return 0x7fff;
}

/* gameTick: drive counter+1 frames of animation, or
   update the carried-object sprite if the resident is holding one.
   addr: gameTick() */

void
gameTick(counter)
short   counter;
{
        short   count;
        short   index;
        short   slot;
        short   y_off;
        short   psi;                            /* petting sprite id */
        short   key;

        /* ---- Path A: not carrying, run the animation loop. ---- */
        if (g_lcyof == NO) {
                count = ani_cnt;
                for (index = 0; index < counter + 1; index = index + 1) {
                        while (count == ani_cnt)
                                sc_ren8();
                        count = ani_cnt;

                        subAniC =
                                subAniC + 1;

                        /* Clock pendulum: 4-frame animation. */
                        od_draw(g_obcla[(subAniC >> 2) & 3],
                                271, 92);
                        gameSim1();
                        cl_redrH();

                        /* Petting-dog animation cycle. */
                        if (g_ptdoa != NO) {
                                if (g_ptanf < 11) {
                                        if (g_ptanf != 0) {
                                                g_selaf[g_ptdsi[g_ptanf - 1]] =
                                                        SPRITE_HIDDEN;
                                        }
                                        psi = g_ptdsi[g_ptanf];
                                        g_selaf[psi] = SPRITE_BEHIND_LCP;
                                        sp_sprs(psi);
                                        g_sepex[g_seslm[psi]] = 192;
                                        g_sepey[g_seslm[psi]] = 165;
                                        g_ptanf =
                                                g_ptanf + 1;
                                } else {
                                        g_selaf[g_ptlss] =
                                                SPRITE_HIDDEN;
                                        sp_upds();
                                        g_ptdoa = NO;
                                }
                        }

                        /* Dog food bowl: current fill state + countdown. */
                        od_draw(g_obdea[lcp_bwlS], 8, 190);
                        if (dg_bwlch < 0) {
                                if (lcp_bwlS != BOWL_EMPTY)
                                        lcp_bwlS =
                                                lcp_bwlS - 1;
                                if (lcp_bwlS < 0)
                                        lcp_bwlS = BOWL_EMPTY;
                        }
                        if (dg_bwlch > 0) {
                                lcp_bwlS =
                                        lcp_bwlS + 1;
                                if (lcp_bwlS > 2)
                                        lcp_bwlS = BOWL_FULL;
                        }

                        /* Fireplace animation + auto-extinguish. */
                        if (fire_act != NO) {
                                od_draw(g_obfia[subAniC & 3],
                                        257, 170);
                                fire_dur =
                                        fire_dur - 1;
                                if (fire_dur == 0)
                                        fire_ext = YES;
                        }
                        if (fire_ext != NO) {
                                fire_ext = NO;
                                fire_act = NO;
                                od_draw(g_obifo, 257, 170);
                        }

                        /* Alarm clock SFX + animation. */
                        if (alarm_p != NO) {
                                if (g_alsts == NO) {
                                        sf_sele(SFX_ALARM_CLOCK, 100000L);
                                        g_alsts = YES;
                                } else if (g_sfacf == NO) {
                                        sf_sele(SFX_ALARM_CLOCK, 100000L);
                                }
                                od_draw(g_obala[subAniC & 1],
                                        53, 102);
                        }
                        if (alarm_p == NO) {
                                g_alsts = NO;
                                if (g_sfacf != NO && g_sfpli == SFX_ALARM_CLOCK)
                                        sf_so();
                        }

                        /* Phone ring. */
                        if (ph_call != NO) {
                                if (g_phrc == 0) {
                                        sf_sele(SFX_PHONE_RING, 10000L);
                                        g_phrc = 26;
                                }
                                g_phrc =
                                        g_phrc - 1;
                                if (g_phrc < 11) {
                                        if (g_sfacf != NO &&
                                            g_sfpli == SFX_PHONE_RING)
                                                sf_so();
                                        od_draw(OBJ_PHONE_2, 190, 168);
                                } else {
                                        od_draw(g_obpha[subAniC & 3],
                                                190, 168);
                                }
                        }
                        if (ph_hu != NO) {
                                od_draw(OBJ_PHONE_2, 190, 168);
                                ph_hu = NO;
                                if (g_sfacf != NO && g_sfpli == SFX_PHONE_RING)
                                        sf_so();
                                g_phrc = 0;
                        }

                        if (lcp_recP != NO) rp_anim();
                        if (lcp_tv != NO)          td_nois();

                        sp_updb();
                        sp_lcha();
                        sp_lchu();

                        if (g_srsdc < 1) {
                                if (no_keyin == NO &&
                                    introSeq == NO) {
                                        key = getKey();
                                        if (key != 0) {
                                                if (key != KEY_CTRL_W_WATER &&
                                                    key != KEY_CTRL_B_BOOK &&
                                                    key != KEY_CTRL_R_RECORD &&
                                                    key != KEY_CTRL_F_FOOD &&
                                                    key != KEY_CTRL_C_CALL &&
                                                    key != KEY_CTRL_D_DOGFOOD &&
                                                    key != KEY_CTRL_A_ALARM &&
                                                    key != KEY_CTRL_P_PATTING) {
                                                        if (tx_sctm == 0) {
                                                                fillTopR(27);
                                                                g_cdibp = 0;
                                                        }
                                                        tx_sctm = 160;
                                                }
                                                deal_kc(key);
                                        }
                                } else if (g_inpmd != NO) {
                                        key = getKey();
                                        if (key != 0)
                                                deal_kc(key);
                                }
                        } else {
                                sc_sctd();
                                g_srsdc =
                                        g_srsdc - 1;
                        }

                        sc_ren8();
                }
                return;
        }

        /* ---- Path B: carrying an object.  Reposition its sprite. ---- */
        slot = g_seslm[g_lcieo];
        if (lcp_face == FACING_RIGHT) {
                g_sepex[slot] = lcp_x + 10;
        } else {
                g_sepex[slot] = (lcp_x - g_seacw[slot]) + 16;
                if (g_sepex[slot] < 0)
                        g_sepex[slot] = 0;
        }
        y_off = cy_yoff(g_lcieo);
        if (y_off != 0x7fff)
                g_sepey[slot] = lcp_y + y_off;
}
