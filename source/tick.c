/*
 * tick.c -- main frame driver (gameTick).
 * addr: gameTick() @ Ghidra 0x256A6
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "keyboard.h"
#include "render.h"
#include "renderf.h"
#include "renderx.h"
#include "sim.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprhead.h"
#include "sprites.h"
#include "tick.h"
#include "tick_tables.h"


/* addr: gameTick() */
void
gameTick(counter)
short   counter;
{
        short   count;
        short   index;
        short   slot;
        short   psi;                            /* petting sprite id */
        short   key;

        /* Carrying-mode position update (Ghidra 0x256a6..0x258e4).
           Path B in the ROM repositions the carried sprite then
           dispatches through carried_object_id_table (0x2b95a) to
           one of 9 per-object handlers at 0x257c6..0x258b0.  Each
           handler writes `g_sepey[g_seslm[SPRITE_X]] = lcp_y - 20`
           with the handler's own SPRITE_ID.  After the handler the
           ROM branches back into Path A's animation counter loop
           via 0x25d60, so Path B is NOT a separate return path --
           reproduced below by falling through into the loop.  A
           port that treated Path B as return-without-rendering
           froze the game whenever g_lcyof leaked YES (see
           source/tools/trace_lcyof.sh). */
        if (g_lcyof != NO) {
                slot = g_seslm[g_lcieo];
                if (lcp_face == FACING_RIGHT) {
                        g_sepex[slot] = lcp_x + 10;
                } else {
                        g_sepex[slot] = (lcp_x - g_seacw[slot]) + 16;
                        if (g_sepex[slot] < 0)
                                g_sepex[slot] = 0;
                }
                /* Per-object Y dispatch (Ghidra 0x257c6..0x258b0).
                   All 9 handlers write the same offset -20; the
                   individual cases exist for byte-fidelity with the
                   ROM's per-sprite-ID jump.  A default case
                   suppresses the ROM's out-of-table wild jump (dbeq
                   fall-through would land on the function pointer at
                   `table + 0x24` which is undefined when g_lcieo
                   matches no carried-object entry). */
                switch (g_lcieo) {
                case SPRITE_GLASS:
                        g_sepey[g_seslm[SPRITE_GLASS]]        = lcp_y - 20;
                        break;
                case SPRITE_GAME_BOX:
                        g_sepey[g_seslm[SPRITE_GAME_BOX]]     = lcp_y - 20;
                        break;
                case SPRITE_FOOD_PACKAGE:
                        g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] = lcp_y - 20;
                        break;
                case SPRITE_FIREWOOD:
                        g_sepey[g_seslm[SPRITE_FIREWOOD]]     = lcp_y - 20;
                        break;
                case SPRITE_COOKING_POT:
                        g_sepey[g_seslm[SPRITE_COOKING_POT]]  = lcp_y - 20;
                        break;
                case SPRITE_SUITCASE:
                        g_sepey[g_seslm[SPRITE_SUITCASE]]     = lcp_y - 20;
                        break;
                case SPRITE_BOOK:
                        g_sepey[g_seslm[SPRITE_BOOK]]         = lcp_y - 20;
                        break;
                case SPRITE_VINYL_CARRY:
                        g_sepey[g_seslm[SPRITE_VINYL_CARRY]]  = lcp_y - 20;
                        break;
                case SPRITE_COOKED_MEAL:
                        g_sepey[g_seslm[SPRITE_COOKED_MEAL]]  = lcp_y - 20;
                        break;
                }
        }

        {
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
                                od_draw(OBJ_FIRE_OFF, 257, 170);
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
                                        if (key != KEY_NONE) {
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
                                        if (key != KEY_NONE)
                                                deal_kc(key);
                                }
                        } else {
                                sc_sctd();
                                g_srsdc =
                                        g_srsdc - 1;
                        }

                        sc_ren8();
                }
        }
}
