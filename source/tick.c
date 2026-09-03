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



/* addr: gameTick() (ROM 0xce28).  Carrying mode (g_lcyof) repositions
   the carried sprite and RETURNS -- in this binary Path B does not
   fall through into the animation loop. */
void
gameTick(counter)
short   counter;
{
        short           key;
        short           index;
        unsigned short  count;
        short           psi;    /* petting sprite id / scratch */

        /* The carrying-mode arm comes FIRST and the test is inverted.
           There is no `slot` local: g_seslm[g_lcieo] is recomputed at
           every use, the inner tests re-check g_lcyof redundantly (both
           arms of the first pair assign the same thing), the clamp
           indexes g_sepex with g_lcieo rather than the slot and lives
           INSIDE the non-carrying arm (the 0x1570e end-of-then jump
           clears it), and cy_yoff is INLINED as a switch. */
        if (g_lcyof != NO) {
                if (lcp_face == FACING_RIGHT) {
                        if (g_lcyof == NO)
                                g_sepex[g_seslm[g_lcieo]] = lcp_x + 10;
                        else
                                g_sepex[g_seslm[g_lcieo]] = lcp_x + 10;
                } else {
                        if (g_lcyof == NO)
                                g_sepex[g_seslm[g_lcieo]] =
                                        lcp_x - g_seacw[g_seslm[g_lcieo]] + 8;
                        else
                                g_sepex[g_seslm[g_lcieo]] =
                                        lcp_x - g_seacw[g_seslm[g_lcieo]] + 16;
                        if (g_sepex[g_lcieo] < 0)
                                g_sepex[g_lcieo] = 0;
                }

                switch (g_lcieo) {
                case SPRITE_SUITCASE:
                        g_sepey[g_seslm[SPRITE_SUITCASE]] = lcp_y - 20;
                        break;
                case SPRITE_GLASS:
                        g_sepey[g_seslm[SPRITE_GLASS]] = lcp_y - 20;
                        break;
                case SPRITE_GAME_BOX:
                        g_sepey[g_seslm[SPRITE_GAME_BOX]] = lcp_y - 20;
                        break;
                case SPRITE_BOOK:
                        g_sepey[g_seslm[SPRITE_BOOK]] = lcp_y - 20;
                        break;
                case SPRITE_FOOD_PACKAGE:
                        g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] = lcp_y - 20;
                        break;
                case SPRITE_FIREWOOD:
                        g_sepey[g_seslm[SPRITE_FIREWOOD]] = lcp_y - 20;
                        break;
                case SPRITE_COOKING_POT:
                        g_sepey[g_seslm[SPRITE_COOKING_POT]] = lcp_y - 20;
                        break;
                case SPRITE_VINYL_CARRY:
                        g_sepey[g_seslm[SPRITE_VINYL_CARRY]] = lcp_y - 20;
                        break;
                case SPRITE_COOKED_MEAL:
                        g_sepey[g_seslm[SPRITE_COOKED_MEAL]] = lcp_y - 20;
                        break;
                }
        }

        /* The tick loop is NOT an else arm in STX: carrying mode falls
           straight into it. */
        {
                count = ani_cnt;
                for (index = 0; index < counter + 1; index++) {
                        while (count == ani_cnt)
                                sc_ren8();
                        count = ani_cnt;

                        subAniC++;

                        /* Clock pendulum: 4-frame animation. */
                        psi = (subAniC >> 2) & 3;
                        od_draw(g_obcla[psi], 271, 92);
                        gameSim1();
                        cl_redrH();

                        /* Petting-dog animation cycle. */
                        if (g_ptdoa != NO) {
                                /* STX tests `> 10` and puts the finish
                                   arm first. */
                                if (g_ptanf > 10) {
                                        g_selaf[g_ptlss] =
                                                SPRITE_HIDDEN;
                                        sp_upds();
                                        g_ptdoa = NO;
                                } else {
                                        if (g_ptanf != 0) {
                                                g_selaf[g_ptdsi[g_ptanf - 1]] =
                                                        SPRITE_HIDDEN;
                                        }
                                        /* No `psi` local: the lookup
                                           is repeated at every use. */
                                        g_selaf[g_ptdsi[g_ptanf]] =
                                                SPRITE_BEHIND_LCP;
                                        sp_sprs(g_ptdsi[g_ptanf]);
                                        g_sepex[g_seslm[g_ptdsi[g_ptanf]]] =
                                                192;
                                        g_sepey[g_seslm[g_ptdsi[g_ptanf]]] =
                                                165;
                                        g_ptanf++;
                                }
                        }

                        /* Dog food bowl: current fill state + countdown. */
                        od_draw(g_obdea[lcp_bwlS], 8, 190);
                        if (dg_bwlch < 0) {
                                if (lcp_bwlS != BOWL_EMPTY)
                                        lcp_bwlS--;
                                if (lcp_bwlS < 0)
                                        lcp_bwlS = BOWL_EMPTY;
                        }
                        if (dg_bwlch > 0) {
                                lcp_bwlS++;
                                if (lcp_bwlS > 2)
                                        lcp_bwlS = BOWL_FULL;
                        }

                        /* Fireplace animation + auto-extinguish. */
                        if (fire_act != NO) {
                                od_draw(g_obfia[subAniC & 3],
                                        257, 170);
                                if (--fire_dur == 0)
                                        fire_ext = YES;
                        }
                        if (fire_ext != NO) {
                                fire_ext = NO;
                                fire_act = NO;
                                od_draw(od_fir0, 257, 170);
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
                                g_phrc--;
                                if (g_phrc > 10) {
                                        od_draw(g_obpha[subAniC & 3],
                                                190, 168);
                                } else {
                                        if (g_sfacf != NO &&
                                            g_sfpli == SFX_PHONE_RING)
                                                sf_so();
                                        od_draw(OBJ_PHONE_2, 190, 168);
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

                        if (g_srsdc > 0) {
                                sc_sctd();
                                g_srsdc--;
                        } else {
                                if (no_keyin == NO &&
                                    introSeq == NO) {
                                        key = getKey();
                                        /* STX's getKey returns -1 for
                                           "no key", not 0. */
                                        if (key != -1) {
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
                                        if ((key = getKey()) != -1)
                                                deal_kc(key);
                                }
                        }

                        sc_ren8();
                }
        }
}
