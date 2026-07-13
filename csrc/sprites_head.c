/*
 * sprites_head.c -- LCP head-animation state machine.
 *
 * head_anim_current and head_anim_target_state are 8-bit direction
 * codes packed as follows:
 *   bits 0..2: horizontal angle 0..7 (0 = full right, 4 = full left,
 *              in facing-neutral terms; mirrored based on facing dir)
 *   bits 3..4: vertical tilt 0..3 (0 = center, 1 = up, 2 = down)
 *   bit 7 set: "no active target" sentinel (state machine idles)
 *
 * The animation picks a new random target every 2..9 game frames when
 * the current position has caught up.  head_anim_mode acts as both a
 * per-state bit-mask (which movements are allowed) and a partial
 * override (fixed target values).  head_movement_delta_table[] gives
 * the signed step to take between horizontal frames.
 *
 * addr: sprite_lcp_head_animate()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>             /* Random() */

extern short    randomRange();

/* Bit-fields inside head_anim_mode.  These are distinct from the
   HEAD_ANIM_* target-state constants in enums.h that share the Ghidra
   name -- Ghidra collapsed both meanings into one symbol on decompile,
   but the disassembly uses them at different bit widths (mask vs value). */
#define HEAD_MODE_H_AMPLITUDE           0x07
#define HEAD_MODE_H_RANGE               0x08
#define HEAD_MODE_V_RANGE               0x60
#define HEAD_MODE_V_OVERRIDE            0x80

/* sprite_lcp_head_animate: pick or advance a head direction target.
   addr: sprite_lcp_head_animate() */

void
sprite_lcp_head_animate()
{
        unsigned short  random_val;
        short           anim_mode;
        short           movement_mask;
        short           random_seed;
        short           target_frame;
        short           current_pos;

        if (head_anim_current != head_anim_target_state ||
            head_anim_mode < 0)
                goto apply_current;

        head_anim_delay_countdown = head_anim_delay_countdown - 1;
        if (head_anim_delay_countdown > 0)
                goto apply_current;

        /* Pick a fresh target.  Coin-flip between a vertical-only
           adjustment and a horizontal one. */
        head_anim_delay_countdown = randomRange(2, 9);
        random_val = (unsigned short) Random();

        if ((random_val & 0x10) != 0) {
                /* Vertical picker. */
                movement_mask = head_anim_mode & HEAD_MODE_V_RANGE;
                if (movement_mask == 0) {
                        movement_mask = (short) Random();
                        movement_mask = movement_mask & HEAD_MODE_V_RANGE;
                        if (movement_mask == 0)
                                movement_mask = 0x40;
                }
                if ((head_anim_mode & (HEAD_MODE_V_OVERRIDE |
                                       HEAD_MODE_V_RANGE)) < 0x81) {
                        random_val = (unsigned short) Random();
                        movement_mask = (((movement_mask >> 5) - 1) & 1) +
                                        ((random_val & 4) >> 2);
                } else {
                        movement_mask = 7 - (head_anim_mode >> 5);
                }
                head_anim_target_state = (movement_mask << 3) |
                                         (head_anim_target_state & 7);
                goto apply_current;
        }

        /* Horizontal picker. */
        if ((head_anim_mode & HEAD_MODE_H_AMPLITUDE) == 0) {
                anim_mode = (short) Random();
                anim_mode = (anim_mode & HEAD_MODE_H_AMPLITUDE) | 1;
        } else {
                anim_mode = head_anim_mode & HEAD_MODE_H_AMPLITUDE;
        }
        random_seed = anim_mode - 1;

        if ((head_anim_mode & HEAD_MODE_H_RANGE) == 0) {
                random_val = (unsigned short) Random();
                if ((random_val & 8) != 0)
                        random_seed = -random_seed;
        } else if ((head_anim_mode & HEAD_MODE_H_RANGE) > 7) {
                random_seed = -random_seed;
        }

        random_seed = (random_seed + head_default_angle_per_state[lcp_state]) & 7;
        if (lcp_facing_direction == FACING_LEFT)
                random_seed = (8 - random_seed) & 7;
        head_anim_target_state = random_seed | (head_anim_target_state & 0x18);

apply_current:
        if (head_anim_target_state >= 0) {
                current_pos = (head_anim_target_state & 0x18) -
                              (head_anim_current & 0x18);
                if (current_pos > 0)
                        head_anim_current = head_anim_current + 8;
                else if (current_pos < 0)
                        head_anim_current = head_anim_current - 8;

                target_frame = head_movement_delta_table[
                        ((head_anim_target_state & 7) -
                         (head_anim_current & 7)) + 7];
                if (target_frame == 99) {
                        target_frame = head_movement_delta_table[
                                (((lcp_facing_direction * 4 +
                                   head_default_angle_per_state[lcp_state]) & 7) -
                                 (head_anim_current & 7)) + 7];
                }
                if (target_frame == 99)
                        target_frame = -1;

                head_anim_current = ((target_frame + head_anim_current) & 7) |
                                    (head_anim_current & 0x18);
        }

        if (head_anim_current >= 0 && head_anim_current < 0x80) {
                anim_mode = head_anim_current & 7;
                if (anim_mode < 5) {
                        head_sprite_frame = anim_mode +
                                head_tilt_frame_offset[
                                        (head_anim_current & 0x18) >> 3];
                        head_sprite_mirror_flag = NO;
                } else {
                        head_sprite_frame = (8 - anim_mode) +
                                head_tilt_frame_offset[
                                        (head_anim_current & 0x18) >> 3];
                        head_sprite_mirror_flag = YES;
                }
        }
}
