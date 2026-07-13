/*
 * tick.c -- main frame driver.
 *
 * game_tick_and_animate(counter) is the workhorse of endless_game_loop.
 * Two entry paths:
 *
 *   g_lcyof == NO:
 *       Loop (counter + 1) frames.  Each frame:
 *         wait for the next 8 Hz render tick
 *         advance sub_animation_frame_counter
 *         draw the wall-clock pendulum
 *         call game_simulate_one_second
 *         advance dog-bowl / fire / alarm / phone / TV animations
 *         update sp_updb / sp_lchu
 *         poll keyboard, dispatch shortcut keys
 *         sc_ren8
 *
 *   g_lcyof != NO:
 *       Position the carried-object sprite:
 *           x = lcp_x + 10                       (facing right)
 *           x = lcp_x - g_seacw + 16 (facing left, clamped)
 *       Then dispatch a per-object handler that sets g_sepey
 *       (all currently listed handlers set y = lcp_y - 20).
 *
 * addr: game_tick_and_animate()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    animation_tick_counter;
extern short    lcp_x;
extern short    lcp_y;
extern void     game_tick_and_animate();
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_lcieo;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_seacw[];
extern short    g_seslm[];
extern short    sub_animation_frame_counter;
extern void     sp_updb();
extern void     sp_lcha();
extern void     sp_lchu();
extern void     sc_ren8();
extern void     clock_redraw_hands();
extern void     object_draw();
extern void     game_simulate_one_second();
extern void     sf_sele();
extern void     sf_so();
extern void     sp_sprs();
extern void     sp_upds();
extern void     record_player_animate_needle();
extern void     td_nois();
extern void     fill_top_rect_with_background();
extern void     sc_sctd();
extern short    get_pressed_key();
extern void     deal_with_keycode();

/* Carried-object per-frame Y offset (Ghidra dispatch table at
   0x257c6..0x258b0).  Every listed sprite uses -20; the default (no
   entry) leaves g_sepey untouched. */
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
        return 0x7fff;          /* sentinel = "no update" */
}

/* game_tick_and_animate: drive counter+1 frames of animation, or update
   the carried-object sprite if the resident is currently carrying.
   addr: game_tick_and_animate() */

void
game_tick_and_animate(counter)
short   counter;
{
        short   count;
        short   index;
        short   slot;
        short   y_off;

        if (g_lcyof != NO) {
                slot = g_seslm[g_lcieo];
                if (lcp_facing_direction == FACING_RIGHT) {
                        g_sepex[slot] = lcp_x + 10;
                } else {
                        g_sepex[slot] =
                                (lcp_x - g_seacw[slot]) + 16;
                        if (g_sepex[slot] < 0)
                                g_sepex[slot] = 0;
                }
                y_off = carry_y_offset(g_lcieo);
                if (y_off != 0x7fff)
                        g_sepey[slot] = lcp_y + y_off;
                return;
        }

        count = animation_tick_counter;
        for (index = 0; index < counter + 1; index = index + 1) {
                while (count == animation_tick_counter)
                        sc_ren8();
                count = animation_tick_counter;

                sub_animation_frame_counter =
                        sub_animation_frame_counter + 1;

                object_draw(0, 271, 92);         /* TODO: clock frame */
                game_simulate_one_second();
                clock_redraw_hands();

                sp_updb();
                sp_lcha();
                sp_lchu();

                sc_ren8();
        }
}
