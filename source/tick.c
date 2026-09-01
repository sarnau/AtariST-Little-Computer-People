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


/* cy_yoff: Y offset applied to the carried-object sprite; 32767 =
   not a carried object.  ROM 0xcdfa -- Alcyon compiles this switch to
   the value/handler table at data 0x13518. */
static short
cy_yoff(id)
short   id;
{
        switch (id) {
        case SPRITE_GLASS:
        case SPRITE_GAME_BOX:
        case SPRITE_FOOD_PACKAGE:
        case SPRITE_FIREWOOD:
        case SPRITE_COOKING_POT:
        case SPRITE_SUITCASE:
        case SPRITE_BOOK:
        case SPRITE_VINYL_CARRY:
        case SPRITE_COOKED_MEAL:
                return -20;
        }
        return 32767;
}

/* addr: gameTick() (ROM 0xce28).  Carrying mode (g_lcyof) repositions
   the carried sprite and RETURNS -- in this binary Path B does not
   fall through into the animation loop. */
void
gameTick(counter)
short   counter;
{
        short   count;
        short   index;
        short   slot;
        short   yoff;
        short   psi;                            /* petting sprite id */
        short   key;

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
                                        if (key != 0)     /* ROM getKey: 0 = no key */ {
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
                                        if (key != 0)     /* ROM getKey: 0 = no key */
                                                deal_kc(key);
                                }
                        } else {
                                sc_sctd();
                                g_srsdc =
                                        g_srsdc - 1;
                        }

                        sc_ren8();
                }
        } else {
                slot = g_seslm[g_lcieo];
                if (lcp_face == FACING_RIGHT) {
                        g_sepex[slot] = lcp_x + 10;
                } else {
                        g_sepex[slot] = (lcp_x - g_seacw[slot]) + 16;
                        if (g_sepex[slot] < 0)
                                g_sepex[slot] = 0;
                }
                yoff = cy_yoff(g_lcieo);
                if (yoff != 32767)
                        g_sepey[slot] = lcp_y + yoff;
        }
}
