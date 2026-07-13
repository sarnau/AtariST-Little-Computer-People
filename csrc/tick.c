/*
 * tick.c -- main frame driver.
 *
 * game_tick_and_animate(counter) is the workhorse of endless_game_loop.
 * Two entry paths:
 *
 *   lcp_carrying_object_flag == NO:
 *       Loop (counter + 1) frames.  Each frame:
 *         wait for the next 8 Hz render tick
 *         advance sub_animation_frame_counter
 *         draw the wall-clock pendulum
 *         call game_simulate_one_second
 *         advance dog-bowl / fire / alarm / phone / TV animations
 *         update sprite_update_body / sprite_lcp_head_update
 *         poll keyboard, dispatch shortcut keys
 *         screen_render_8hz
 *
 *   lcp_carrying_object_flag != NO:
 *       Position the carried-object sprite:
 *           x = lcp_x + 10                       (facing right)
 *           x = lcp_x - sprite_active_width + 16 (facing left, clamped)
 *       Then dispatch a per-object handler that sets sprite_pending_y
 *       (all currently listed handlers set y = lcp_y - 20).
 *
 * addr: game_tick_and_animate()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern void     sprite_update_body();
extern void     sprite_lcp_head_animate();
extern void     sprite_lcp_head_update();
extern void     screen_render_8hz();
extern void     clock_redraw_hands();
extern void     object_draw();
extern void     game_simulate_one_second();
extern void     soundeffect_select();
extern void     soundeffects_off();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     record_player_animate_needle();
extern void     tv_draw_static_noise();
extern void     fill_top_rect_with_background();
extern void     screen_scroll_text_down();
extern short    get_pressed_key();
extern void     deal_with_keycode();

/* Carried-object per-frame Y offset (Ghidra dispatch table at
   0x257c6..0x258b0).  Every listed sprite uses -20; the default (no
   entry) leaves sprite_pending_y untouched. */
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

        if (lcp_carrying_object_flag != NO) {
                slot = sprite_slot_map[lcp_carried_object];
                if (lcp_facing_direction == FACING_RIGHT) {
                        sprite_pending_x[slot] = lcp_x + 10;
                } else {
                        sprite_pending_x[slot] =
                                (lcp_x - sprite_active_width[slot]) + 16;
                        if (sprite_pending_x[slot] < 0)
                                sprite_pending_x[slot] = 0;
                }
                y_off = carry_y_offset(lcp_carried_object);
                if (y_off != 0x7fff)
                        sprite_pending_y[slot] = lcp_y + y_off;
                return;
        }

        count = animation_tick_counter;
        for (index = 0; index < counter + 1; index = index + 1) {
                while (count == animation_tick_counter)
                        screen_render_8hz();
                count = animation_tick_counter;

                sub_animation_frame_counter =
                        sub_animation_frame_counter + 1;

                object_draw(0, 271, 92);         /* TODO: clock frame */
                game_simulate_one_second();
                clock_redraw_hands();

                sprite_update_body();
                sprite_lcp_head_animate();
                sprite_lcp_head_update();

                screen_render_8hz();
        }
}
