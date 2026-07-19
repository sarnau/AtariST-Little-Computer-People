/*
 * sprhead.c -- LCP head-animation state machine.
 *
 * g_hacur and g_hatas are 8-bit direction
 * codes packed as follows:
 *   bits 0..2: horizontal angle 0..7 (0 = full right, 4 = full left,
 *              in facing-neutral terms; mirrored based on facing dir)
 *   bits 3..4: vertical tilt 0..3 (0 = center, 1 = up, 2 = down)
 *   bit 7 set: "no active target" sentinel (state machine idles)
 *
 * The animation picks a new random target every 2..9 game frames when
 * the current position has caught up.  g_hamod acts as both a
 * per-state bit-mask (which movements are allowed) and a partial
 * override (fixed target values).  hd_mvd[] gives
 * the signed step to take between horizontal frames.
 *
 * addr: sp_lcha()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "globals.h"
#include "random.h"
#include "sprglobs.h"
#include "sprhead.h"


/* Bit-fields inside g_hamod.  These are distinct from the
   HEAD_ANIM_* target-state constants in enums.h that share the Ghidra
   name -- Ghidra collapsed both meanings into one symbol on decompile,
   but the disassembly uses them at different bit widths (mask vs value). */
#define HEAD_MODE_H_AMPLITUDE           0x07
#define HEAD_MODE_H_RANGE               0x08
#define HEAD_MODE_V_RANGE               0x60
#define HEAD_MODE_V_OVERRIDE            0x80

/* sp_lcha: pick or advance a head direction target.
   addr: sp_lcha() */

void
sp_lcha()
{
        unsigned short  random_val;
        short           anim_mode;
        short           movement_mask;
        short           random_seed;
        short           target_frame;
        short           current_pos;

        if (g_hacur != g_hatas ||
            g_hamod < 0)
                goto apply_current;

        g_hadec = g_hadec - 1;
        if (g_hadec > 0)
                goto apply_current;

        /* Pick a fresh target.  Coin-flip between a vertical-only
           adjustment and a horizontal one. */
        g_hadec = rndRng(2, 9);
        random_val = (unsigned short) Random();

        if ((random_val & 0x10) != 0) {
                /* Vertical picker. */
                movement_mask = g_hamod & HEAD_MODE_V_RANGE;
                if (movement_mask == 0) {
                        movement_mask = (short) Random();
                        movement_mask = movement_mask & HEAD_MODE_V_RANGE;
                        if (movement_mask == 0)
                                movement_mask = 0x40;
                }
                if ((g_hamod & (HEAD_MODE_V_OVERRIDE |
                                       HEAD_MODE_V_RANGE)) < 0x81) {
                        random_val = (unsigned short) Random();
                        movement_mask = (((movement_mask >> 5) - 1) & 1) +
                                        ((random_val & 4) >> 2);
                } else {
                        movement_mask = 7 - (g_hamod >> 5);
                }
                g_hatas = (movement_mask << 3) |
                                         (g_hatas & 7);
                goto apply_current;
        }

        /* Horizontal picker. */
        if ((g_hamod & HEAD_MODE_H_AMPLITUDE) == 0) {
                anim_mode = (short) Random();
                anim_mode = (anim_mode & HEAD_MODE_H_AMPLITUDE) | 1;
        } else {
                anim_mode = g_hamod & HEAD_MODE_H_AMPLITUDE;
        }
        random_seed = anim_mode - 1;

        if ((g_hamod & HEAD_MODE_H_RANGE) == 0) {
                random_val = (unsigned short) Random();
                if ((random_val & 8) != 0)
                        random_seed = -random_seed;
        } else if ((g_hamod & HEAD_MODE_H_RANGE) > 7) {
                random_seed = -random_seed;
        }

        random_seed = (random_seed + hd_dang[lcp_st]) & 7;
        if (lcp_face == FACING_LEFT)
                random_seed = (8 - random_seed) & 7;
        g_hatas = random_seed | (g_hatas & 0x18);

apply_current:
        if (g_hatas >= 0) {
                current_pos = (g_hatas & 0x18) -
                              (g_hacur & 0x18);
                if (current_pos > 0)
                        g_hacur = g_hacur + 8;
                else if (current_pos < 0)
                        g_hacur = g_hacur - 8;

                target_frame = hd_mvd[
                        ((g_hatas & 7) -
                         (g_hacur & 7)) + 7];
                if (target_frame == 99) {
                        target_frame = hd_mvd[
                                (((lcp_face * 4 +
                                   hd_dang[lcp_st]) & 7) -
                                 (g_hacur & 7)) + 7];
                }
                if (target_frame == 99)
                        target_frame = -1;

                g_hacur = ((target_frame + g_hacur) & 7) |
                                    (g_hacur & 0x18);
        }

        if (g_hacur >= 0 && g_hacur < 0x80) {
                anim_mode = g_hacur & 7;
                if (anim_mode < 5) {
                        g_hsfra = anim_mode +
                                hd_tilt[
                                        (g_hacur & 0x18) >> 3];
                        g_hsmif = NO;
                } else {
                        g_hsfra = (8 - anim_mode) +
                                hd_tilt[
                                        (g_hacur & 0x18) >> 3];
                        g_hsmif = YES;
                }
        }
}
