/*
 * tick.c -- main frame driver.
 *
 * game_tick_and_animate(counter) is the workhorse of endless_game_loop.
 * Ghidra 0x256A6.  Two entry paths:
 *
 *   g_lcyof == NO:  loop counter+1 frames.  Each frame:
 *     - wait for the next 8 Hz render tick (sc_ren8 spin)
 *     - advance sub_animation_frame_counter, redraw clock pendulum
 *     - game_simulate_one_second, clock_redraw_hands
 *     - petting-dog animation cycle (11 frames of head-pat sprites)
 *     - dog food bowl countdown + redraw
 *     - fire animation (4 frames while fire active) + auto-extinguish
 *     - alarm-clock animation + SFX loop
 *     - phone-ring animation + SFX loop
 *     - record-player needle animation (rp_anim)
 *     - TV static-noise animation (td_nois)
 *     - LCP body + head sprite update (sp_updb / sp_lcha / sp_lchu)
 *     - keyboard input dispatch (deal_with_keycode)
 *     - screen scroll if a text-scroll countdown is active
 *     - final sc_ren8
 *
 *   g_lcyof != NO:  reposition the currently-carried object sprite
 *     (X = lcp_x + 10 facing right, or lcp_x - width + 16 facing left),
 *     then dispatch to a per-carried-object Y-offset handler by
 *     walking g_cotbl until the sprite id matches.
 *
 * addr: game_tick_and_animate()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"

/* --- per-file extern block (auto-generated for Alcyon). */
extern short    animation_tick_counter;
extern short    lcp_x, lcp_y;
extern short    lcp_facing_direction;
extern short    lcp_state;
extern short    lcp_dog_bowl_status;
extern short    g_lcyof;                        /* lcp_carrying_object_flag */
extern short    g_lcieo;                        /* lcp_carried_object */
extern short    lcp_record_playing;
extern short    lcp_tv_on;
extern BOOL16   phone_call_active_flag;
extern BOOL16   phone_hangup_flag;
extern short    g_phrc;
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern BOOL16   g_alsts;
extern BOOL16   fire_active_flag;
extern short    fire_duration_countdown;
extern BOOL16   fire_extinguish_flag;
extern BOOL16   g_ptdoa;                        /* petting_dog_active */
extern short    g_ptanf;
extern short    g_ptlss;
extern short    dog_food_bowl_change;
extern short    text_scroll_timer;
extern short    g_srsdc;
extern short    g_cdibp;
extern BOOL16   disable_key_input_flag;
extern BOOL16   intro_sequence_active;
extern BOOL16   game_input_mode_flag;
extern short    sub_animation_frame_counter;
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
extern void     clock_redraw_hands();
extern void     od_draw();
extern void     game_simulate_one_second();
extern void     sf_sele();                      /* soundeffect_select */
extern void     sf_so();                        /* soundeffects_off */
extern void     sp_sprs();                      /* spritedata_select */
extern void     sp_upds();                      /* sprite_update_slots */
extern void     rp_anim();                      /* record_player_animate_needle */
extern void     td_nois();                      /* tv_draw_static_noise */
extern void     fill_top_rect_with_background();
extern void     sc_sctd();                      /* screen_scroll_text_down */
extern short    get_pressed_key();
extern void     deal_with_keycode();

/* Object ids referenced by game_tick_and_animate as bare literals in
   the Ghidra disassembly (no port-side global covers them).  The
   values come from the OBJECTS-file record indices for the phone and
   the fire-off sprites. */
#define OBJ_PHONE_2             23      /* Ghidra literal 0x17 */

/* Carried-object per-frame Y offset (Ghidra 0x257c6..0x258b0 jump
   table).  Every listed sprite id uses -20; the default is "no update". */
static short
carry_y_offset(id)
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

/* game_tick_and_animate: drive counter+1 frames of animation, or
   update the carried-object sprite if the resident is holding one.
   addr: game_tick_and_animate() */

void
game_tick_and_animate(counter)
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
                count = animation_tick_counter;
                for (index = 0; index < counter + 1; index = index + 1) {
                        while (count == animation_tick_counter)
                                sc_ren8();
                        count = animation_tick_counter;

                        sub_animation_frame_counter =
                                sub_animation_frame_counter + 1;

                        /* Clock pendulum: 4-frame animation. */
                        od_draw(g_obcla[(sub_animation_frame_counter >> 2) & 3],
                                271, 92);
                        game_simulate_one_second();
                        clock_redraw_hands();

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
                        od_draw(g_obdea[lcp_dog_bowl_status], 8, 190);
                        if (dog_food_bowl_change < 0) {
                                if (lcp_dog_bowl_status != BOWL_EMPTY)
                                        lcp_dog_bowl_status =
                                                lcp_dog_bowl_status - 1;
                                if (lcp_dog_bowl_status < 0)
                                        lcp_dog_bowl_status = BOWL_EMPTY;
                        }
                        if (dog_food_bowl_change > 0) {
                                lcp_dog_bowl_status =
                                        lcp_dog_bowl_status + 1;
                                if (lcp_dog_bowl_status > 2)
                                        lcp_dog_bowl_status = BOWL_FULL;
                        }

                        /* Fireplace animation + auto-extinguish. */
                        if (fire_active_flag != NO) {
                                od_draw(g_obfia[sub_animation_frame_counter & 3],
                                        257, 170);
                                fire_duration_countdown =
                                        fire_duration_countdown - 1;
                                if (fire_duration_countdown == 0)
                                        fire_extinguish_flag = YES;
                        }
                        if (fire_extinguish_flag != NO) {
                                fire_extinguish_flag = NO;
                                fire_active_flag = NO;
                                od_draw(g_obifo, 257, 170);
                        }

                        /* Alarm clock SFX + animation. */
                        if (ctrl_a_alarm_pressed_flag != NO) {
                                if (g_alsts == NO) {
                                        sf_sele(SFX_ALARM_CLOCK, 100000L);
                                        g_alsts = YES;
                                } else if (g_sfacf == NO) {
                                        sf_sele(SFX_ALARM_CLOCK, 100000L);
                                }
                                od_draw(g_obala[sub_animation_frame_counter & 1],
                                        53, 102);
                        }
                        if (ctrl_a_alarm_pressed_flag == NO) {
                                g_alsts = NO;
                                if (g_sfacf != NO && g_sfpli == SFX_ALARM_CLOCK)
                                        sf_so();
                        }

                        /* Phone ring. */
                        if (phone_call_active_flag != NO) {
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
                                        od_draw(g_obpha[sub_animation_frame_counter & 3],
                                                190, 168);
                                }
                        }
                        if (phone_hangup_flag != NO) {
                                od_draw(OBJ_PHONE_2, 190, 168);
                                phone_hangup_flag = NO;
                                if (g_sfacf != NO && g_sfpli == SFX_PHONE_RING)
                                        sf_so();
                                g_phrc = 0;
                        }

                        if (lcp_record_playing != NO) rp_anim();
                        if (lcp_tv_on != NO)          td_nois();

                        sp_updb();
                        sp_lcha();
                        sp_lchu();

                        if (g_srsdc < 1) {
                                if (disable_key_input_flag == NO &&
                                    intro_sequence_active == NO) {
                                        key = get_pressed_key();
                                        if (key != 0) {
                                                if (key != KEY_CTRL_W_WATER &&
                                                    key != KEY_CTRL_B_BOOK &&
                                                    key != KEY_CTRL_R_RECORD &&
                                                    key != KEY_CTRL_F_FOOD &&
                                                    key != KEY_CTRL_C_CALL &&
                                                    key != KEY_CTRL_D_DOGFOOD &&
                                                    key != KEY_CTRL_A_ALARM &&
                                                    key != KEY_CTRL_P_PATTING) {
                                                        if (text_scroll_timer == 0) {
                                                                fill_top_rect_with_background(27);
                                                                g_cdibp = 0;
                                                        }
                                                        text_scroll_timer = 160;
                                                }
                                                deal_with_keycode(key);
                                        }
                                } else if (game_input_mode_flag != NO) {
                                        key = get_pressed_key();
                                        if (key != 0)
                                                deal_with_keycode(key);
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
        if (lcp_facing_direction == FACING_RIGHT) {
                g_sepex[slot] = lcp_x + 10;
        } else {
                g_sepex[slot] = (lcp_x - g_seacw[slot]) + 16;
                if (g_sepex[slot] < 0)
                        g_sepex[slot] = 0;
        }
        y_off = carry_y_offset(g_lcieo);
        if (y_off != 0x7fff)
                g_sepey[slot] = lcp_y + y_off;
}
