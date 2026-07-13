/*
 * render_frame.c -- screen_render_8hz, the main 8Hz compositor.
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
 *      with a non-NULL active image gets drawn via sprite_draw.
 *   6. Vsync + Setscreen to page-flip.
 *   7. Play any queued SFX via soundeffect_irq_play.
 *   8. Toggle the compositing target for the next frame between the
 *      original physbase and the alternate buffer at 0x2CA00.
 *   9. Bump animation_tick_counter.
 *
 * addr: screen_render_8hz()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern short    randomRange();
extern void     dog_move_and_animate();
extern void     spritedata_update_dog();
extern void     soundeffects_off();
extern void     soundeffect_select();
extern void     soundeffect_irq_play();
extern void     blkcopy32();
extern void     sprite_draw();

/* Read the 200 Hz clock via GEMDOS Super mode.  On the host we return
   0 -- the render skip-gate then always fires immediately (which is
   fine; the game state is unchanged, and rendering isn't visible under
   host builds anyway). */

static short
read_hz_200()
{
        void *  saveSSP;
        short   hi;

        saveSSP = (void *) _gemdos(GEMDOS_Super, 0L, 0L, 0L);
        hi = _hz_200_lo;                /* low word of _hz_200 (200 Hz) */
        _gemdos(GEMDOS_Super, (long) saveSSP, 0L, 0L);
        return hi;
}

static long
read_vbclock()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) _gemdos(GEMDOS_Super, 0L, 0L, 0L);
        v = _vbclock;
        _gemdos(GEMDOS_Super, (long) saveSSP, 0L, 0L);
        return v;
}

/* dog_pick_new_wander_target: choose the next dog destination from a
   9-entry table.  Extracted from screen_render_8hz for readability.
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

        dest_position = dog_destination_position_table[pick];
        house_get_position_xy(dest_position,
                              &dog_target_x, &dog_target_y);
        dog_target_y = dog_dest_y_offset_table[pick] + dog_target_y;
        dog_target_x = dog_dest_x_offset_table[pick] + dog_target_x;

        if (dest_position == POS_BTM_STAIR_LANDING)
                dog_near_food_bowl = YES;

        dog_last_target_index = pick;
        dog_idle_countdown    = randomRange(20, 200);
}

/* screen_render_8hz: the frame driver.
   addr: screen_render_8hz() */

void
screen_render_8hz()
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
        if (dog_target_x == 0 && dog_target_y == 0 &&
            lcp_dog_bowl_status != BOWL_EMPTY &&
            dog_near_food_bowl != NO &&
            dog_eating_active == NO &&
            dog_x < 0x14 && dog_y > 0xa0) {
                dog_eating_active    = YES;
                dog_eating_countdown = randomRange(0x52, 100);
        }

        /* Idle countdown while waiting for a target. */
        if (dog_target_x == 0 && dog_target_y == 0 &&
            dog_idle_countdown != 0 && dog_eating_active == NO)
                dog_idle_countdown = dog_idle_countdown - 1;

        if (dog_target_x == 0 && dog_target_y == 0 &&
            dog_idle_countdown == 0 && dog_eating_active == NO)
                dog_pick_new_wander_target();

        /* Eating animation cycle. */
        if (dog_eating_active != NO) {
                dog_eating_countdown = dog_eating_countdown - 1;
                if (dog_eating_countdown == 0) {
                        dog_eating_active    = NO;
                        dog_near_food_bowl   = NO;
                        dog_food_bowl_change = -1;
                } else {
                        if (dog_eating_countdown == 60 ||
                            dog_eating_countdown == 30 ||
                            dog_eating_countdown == 4)
                                dog_food_bowl_change = -1;
                        else
                                dog_food_bowl_change = 0;
                        dog_sprite_id = dog_sprite_eating_anim_tab[
                                dog_eating_countdown % 3];
                        spritedata_update_dog(dog_sprite_id, 1, NO);
                }
        }

        /* --- SFX chaining --- */
        if (soundeffect_remaining_ticks > 0) {
                soundeffect_remaining_ticks =
                        soundeffect_remaining_ticks - 1;
                if (soundeffect_remaining_ticks == 0) {
                        soundeffects_off();
                        if (soundeffect_playing_id == SFX_DOORBELL)
                                soundeffect_select(SFX_DOORBELL_ECHO, 5L);
                        if (soundeffect_playing_id == SFX_TOILET_FLUSH)
                                soundeffect_select(SFX_TOILET_REFILL, 15L);
                }
        }

        /* --- Background copy --- */
        if (text_scroll_timer < 1) {
                if (text_scroll_timer < 0) {
                        /* Partial (top-strip only). */
                        blkcopy32(dest_screenbase_ptr,
                                  screen_mfdb.fd_addr, 385);
                        blkcopy32((char *) MFDB_screen_ptr.fd_addr + 12320,
                                  (char *) screen_mfdb.fd_addr + 12320,
                                  615);
                } else {
                        /* Full-screen. */
                        blkcopy32(MFDB_screen_ptr.fd_addr,
                                  screen_mfdb.fd_addr, 1000);
                }
        } else {
                /* Split copy for letter scroll. */
                blkcopy32(dest_screenbase_ptr,
                          screen_mfdb.fd_addr, 135);
                blkcopy32((char *) MFDB_screen_ptr.fd_addr + 4320,
                          (char *) screen_mfdb.fd_addr + 4320, 865);
                text_scroll_timer = text_scroll_timer - 1;
        }

        /* --- Sprite compositing --- */
        for (index = 0; index < 8; index = index + 1) {
                if (sprite_pending_flag[index] == YES) {
                        sprite_pending_flag[index]  = NO;
                        sprite_pending_x[index]     = sprite_active_x[index];
                        sprite_pending_y[index]     = sprite_active_y[index];
                        sprite_active_image[index]  = sprite_pending_image[index];
                        sprite_active_mask[index]   = sprite_pending_mask[index];
                        sprite_active_height[index] = sprite_pending_height[index];
                        sprite_active_width[index]  = sprite_pending_width[index];
                }
                if (sprite_active_image[index] != NULL)
                        sprite_draw(index);
        }

        /* --- Page flip --- */
        current_screen_mfdb = &screen_mfdb;
        _xbios(XBIOS_Vsync, 0L, 0L, 0L);
        _xbios(XBIOS_Setscreen,
               -1L, (long) current_screen_mfdb->fd_addr, -1L);

        if (soundeffect_active_flag != NO) {
                soundeffect_irq_play();
                soundeffect_active_flag = NO;
        }

        /* Toggle compositing buffer between the physbase we started
           with and the alternate at 0x2CA00. */
        if (current_screen_mfdb->fd_addr == save_physbase)
                current_screen_mfdb->fd_addr = (void *) 0x2ca00L;
        else
                current_screen_mfdb->fd_addr = save_physbase;

        animation_tick_counter = animation_tick_counter + 1;
        last_vbclock = read_vbclock();
}
