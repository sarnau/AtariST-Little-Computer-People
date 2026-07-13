/*
 * sprites.c -- sprite slot management for the resident (body + head)
 *              and the carried-object rider.
 *
 * The Atari ST sprite pipeline runs a pending -> active double buffer
 * over 8 hardware sprite slots.  Slot allocation:
 *
 *   slot 0   dog (behind LCP layer)
 *   slot 1-2 general objects / carried items
 *   slot 3   LCP body
 *   slot 4   LCP head
 *   slot 5-6 general objects / pet-hand animation
 *   slot 7   dog (in-front-of-LCP layer)
 *
 * All positioning is anchored to the resident's feet (lcp_x, lcp_y);
 * per-frame Y offsets come from body_y_offset_per_state[].
 *
 * addr: sprite_update_body(), sprite_lcp_head_update(),
 *       spritedata_select_carried_object_left/right(),
 *       update_carried_object_sprite() (carry branch of
 *       game_tick_and_animate)
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

/* Forward-decls -- Alcyon skips these silently; modern clang under
   -Werror -std=c89 does not. */
extern void     sprite_lcp_head_animate();
extern void     sprite_lcp_flip();
extern void     sprite_update_slots();

/* sprite_update_body: select the body pose for the current lcp_state and
   drop it into slot 3.  When carrying an object during a walking state
   (< 25), uses the alternate arms-up frames from carry_body_frame_table.
   Positioning: X = lcp_x - 4 (right) or lcp_x - 14 (left);
   Y = lcp_y + body_y_offset[state] - 21.

   addr: sprite_update_body() */

void
sprite_update_body()
{
        short   frame;

        /* Wait out any double-buffer race on slot 3. */
        while (sprite_pending_flag[3] == YES)
                ;

        frame = body_sprite_frame_table[lcp_state];
        if (lcp_carrying_object_flag != NO && lcp_state < 25)
                frame = carry_body_frame_table[lcp_state];

        sprite_lcp_flip((short *) (body_lcp_file + frame),
                        (short *) (body_shape_data + frame),
                        (short *) lcp_sprite_img,
                        (short *) lcp_sprite_mask,
                        2, 21, lcp_facing_direction, 1);

        if (lcp_facing_direction == FACING_RIGHT)
                sprite_active_x[3] = lcp_x - 4;
        else
                sprite_active_x[3] = lcp_x - 14;

        sprite_active_y[3] = lcp_y + body_y_offset_per_state[lcp_state] - 21;
        if (debug_hide_lcp_offscreen != NO)
                sprite_active_y[3] = 300;

        sprite_pending_height[3] = 21;
        sprite_pending_width[3]  = 32;
        sprite_pending_image[3]  = lcp_sprite_img;
        sprite_pending_mask[3]   = lcp_sprite_mask;

        if (lcp_sprites_hidden != NO)
                sprite_pending_image[3] = NULL;

        sprite_pending_flag[3] = YES;
}

/* spritedata_select_carried_object_left: activate a sprite as a carried
   object in the behind-LCP layer.  Called from action code when the
   resident picks something up.  The per-frame X/Y update happens in
   update_carried_object_sprite() below.

   addr: spritedata_select_carried_object_left() */

void
spritedata_select_carried_object_left(sprite_index)
short   sprite_index;
{
        short   slot;

        sprite_layer_flags[sprite_index] = SPRITE_BEHIND_LCP;
        sprite_update_slots();
        slot = sprite_slot_map[sprite_index];
        sprite_active_image[slot]  = sprite_def_image[sprite_index];
        sprite_active_mask[slot]   = sprite_def_mask[sprite_index];
        sprite_active_height[slot] = sprite_def_height[sprite_index];
        sprite_active_width[slot]  = sprite_def_width[sprite_index];
        lcp_carrying_object_flag = YES;
        lcp_carried_object       = sprite_index;
}

/* spritedata_select: the "generic" sprite activator used by save.c and
   the pet/petting animations.  Recomputes the 8-slot layout then copies
   the definition's image / mask / dimensions into the active-slot
   arrays.  Bypasses the pending double-buffer.

   addr: spritedata_select() */

void
spritedata_select(sprite_index)
short   sprite_index;
{
        short   slot;

        sprite_update_slots();
        slot = sprite_slot_map[sprite_index];
        sprite_active_image[slot]  = sprite_def_image[sprite_index];
        sprite_active_mask[slot]   = sprite_def_mask[sprite_index];
        sprite_active_height[slot] = sprite_def_height[sprite_index];
        sprite_active_width[slot]  = sprite_def_width[sprite_index];
}

/* lcp_wait_head_reach_target: spin ticking the animation loop until the
   head's current direction matches its target.
   addr: lcp_wait_head_reach_target() */

void
lcp_wait_head_reach_target()
{
        while (head_anim_current != head_anim_target_state)
                game_tick_and_animate(0);
}

/* hide_lcp_sprites: stash slot 3 (body) and slot 4 (head) active image
   pointers, nil them out, and raise the hidden flag so the sprite
   update pipeline knows to keep them cleared.  Used during the closet /
   toilet / front-door "resident enters an enclosed sprite" sequences.
   addr: hide_lcp_sprites() */

void
hide_lcp_sprites()
{
        saved_body_sprite_ptr  = sprite_active_image[3];
        saved_head_sprite_ptr  = sprite_active_image[4];
        sprite_active_image[3] = NULL;
        sprite_active_image[4] = NULL;
        lcp_sprites_hidden     = YES;
}

/* show_lcp_sprites: restore the pointers hide_lcp_sprites() stashed.
   addr: show_lcp_sprites() */

void
show_lcp_sprites()
{
        sprite_active_image[3] = saved_body_sprite_ptr;
        sprite_active_image[4] = saved_head_sprite_ptr;
        lcp_sprites_hidden     = NO;
}

/* spritedata_select_carried_object_right: as above, but places the
   carried sprite in the in-front-of-LCP layer.
   addr: spritedata_select_carried_object_right() */

void
spritedata_select_carried_object_right(sprite_index)
short   sprite_index;
{
        short   slot;

        sprite_layer_flags[sprite_index] = SPRITE_IN_FRONT;
        sprite_update_slots();
        slot = sprite_slot_map[sprite_index];
        sprite_active_image[slot]  = sprite_def_image[sprite_index];
        sprite_active_mask[slot]   = sprite_def_mask[sprite_index];
        sprite_active_height[slot] = sprite_def_height[sprite_index];
        sprite_active_width[slot]  = sprite_def_width[sprite_index];
        lcp_carrying_object_flag = YES;
        lcp_carried_object       = sprite_index;
}

/* ---- Sprite compositing pipeline -------------------------------------- */

/* sprite_lcp_flip: expand a 2-word (32-pixel)-wide LCP source frame into
   a 4-word (64-pixel)-wide destination row, with optional horizontal
   mirror.  flipVertical selects whether the sprite content sits in the
   left half of the destination row (padding on the right) or the right
   half (padding on the left), so a right-facing frame's 32 pixels land
   at the same screen X as a left-facing one after body_sprite_frame_
   table selection.  Called from sprite_update_body and
   sprite_lcp_head_update.

   addr: sprite_lcp_flip() */

void
sprite_lcp_flip(srcImg, srcMask, destImg, destMask,
                width, height, flipHorizontal, flipVertical)
short * srcImg;
short * srcMask;
short * destImg;
short * destMask;
short   width;
short   height;
short   flipHorizontal;
short   flipVertical;
{
        unsigned short  uVar1;
        unsigned short  mask;
        short *         psVar2;
        short           x;
        short           y;
        int             iVar3;

        if (flipHorizontal == 0) {
                for (y = 0; y < height; y = y + 1) {
                        for (x = 0; x < width; x = x + 1) {
                                psVar2 = destImg;
                                if (flipVertical == 0) {
                                        destImg[0] = 0;
                                        destImg[1] = srcImg[0];
                                        destImg[2] = srcImg[1];
                                } else {
                                        destImg[0] = srcImg[0];
                                        destImg[1] = srcImg[1];
                                        destImg[2] = 0;
                                }
                                destImg = destImg + 3;
                                srcImg  = srcImg  + 2;
                                destImg[0] = 0;
                                destImg    = psVar2 + 4;

                                destMask[0] = srcMask[0];
                                destMask[1] = srcMask[0];
                                destMask[2] = srcMask[0];
                                destMask[3] = srcMask[0];
                                srcMask  = srcMask  + 1;
                                destMask = destMask + 4;
                        }
                }
                return;
        }

        for (y = 0; y < height; y = y + 1) {
                for (x = 0; x < width; x = x + 1) {
                        psVar2 = destImg;

                        iVar3 = (width - 1) - x;
                        mask  = revert_table[(unsigned char) (srcImg[iVar3 + iVar3] >> 8)] |
                                revert_table[(unsigned char) (srcImg[iVar3 + iVar3])] << 8;
                        uVar1 = revert_table[(unsigned char) (srcImg[iVar3 + iVar3 + 1] >> 8)] |
                                revert_table[(unsigned char) (srcImg[iVar3 + iVar3 + 1])] << 8;

                        if (flipVertical == 0) {
                                destImg[0] = 0;
                                destImg[1] = mask;
                                destImg[2] = uVar1;
                        } else {
                                destImg[0] = mask;
                                destImg[1] = uVar1;
                                destImg[2] = 0;
                        }
                        destImg    = destImg + 3;
                        destImg[0] = 0;
                        destImg    = psVar2 + 4;

                        mask = revert_table[(unsigned char) (srcMask[(width - 1) - x] >> 8)] |
                               revert_table[(unsigned char) (srcMask[(width - 1) - x])] << 8;
                        destMask[0] = mask;
                        destMask[1] = mask;
                        destMask[2] = mask;
                        destMask[3] = mask;
                        destMask = destMask + 4;
                }
                srcImg  = (short *) ((char *) srcImg  + (width << 2));
                srcMask = (short *) ((char *) srcMask + (width << 1));
        }
}

/* sprite_flip_horizontal: mirror a general sprite in place.  Unlike
   sprite_lcp_flip this preserves the source width (no row expansion);
   the caller supplies pre-sized destination buffers.
   addr: sprite_flip_horizontal() */

void
sprite_flip_horizontal(source, dest, pixelHeight, wordsWidth)
unsigned short *        source;
unsigned short *        dest;
short                   pixelHeight;
short                   wordsWidth;
{
        unsigned short *        img_ptr;
        unsigned short          v;
        short                   planeIndex;
        short                   x;
        short                   y;

        for (y = 0; y < pixelHeight; y = y + 1) {
                for (x = 0; x < wordsWidth; x = x + 1) {
                        img_ptr = source + (((wordsWidth - 1) - x) << 2);
                        for (planeIndex = 0; planeIndex < 4;
                             planeIndex = planeIndex + 1) {
                                v = *img_ptr;
                                img_ptr = img_ptr + 1;
                                *dest = (revert_table[v & 0xff] << 8) |
                                         revert_table[(v >> 8) & 0xff];
                                dest = dest + 1;
                        }
                }
                source = source + (wordsWidth << 2);
        }
}

/* sprite_update_slots: allocate the 60 logical sprites onto 8 hardware
   slots by layer.  Slot 3 is body, slot 4 is head (both reserved).
   Layer -1 (SPRITE_BEHIND_LCP) uses slots 1..2, layer +1 (SPRITE_IN_FRONT)
   uses slots 5..6, layer 0 (SPRITE_HIDDEN) maps to slot 9 (off-screen).
   Slot 0 and 7 are reserved for the dog.  When two logical sprites
   compete for the same slot, the older one is bumped to the alternate
   slot and its render state (image, mask, pending X/Y) is copied along.

   addr: sprite_update_slots() */

void
sprite_update_slots()
{
        short   spriteID;
        short   sVar1;
        short   index;
        short   i;

        if (sprite_layer_flags[0] == SPRITE_HIDDEN)
                sprite_active_image[sprite_slot_map[0]] = NULL;
        if (sprite_layer_flags[1] == SPRITE_HIDDEN)
                sprite_active_image[sprite_slot_map[0]] = NULL;

        for (spriteID = 3; spriteID < 60; spriteID = spriteID + 1) {
                if (sprite_layer_flags[spriteID] == SPRITE_HIDDEN) {
                        sprite_slot_map[spriteID] = 9;
                        continue;
                }

                if (sprite_layer_flags[spriteID] == SPRITE_IN_FRONT) {
                        i = sprite_slot_map[spriteID];
                        sprite_slot_map[spriteID] = 6;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (sprite_slot_map[index] == 6) {
                                        sprite_slot_map[spriteID] = 5;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < 60;
                             index = index + 1) {
                                if (sprite_slot_map[index] ==
                                    sprite_slot_map[spriteID]) {
                                        sprite_slot_map[index] = 5;
                                        sprite_pending_x[5]     = sprite_pending_x[6];
                                        sprite_pending_y[5]     = sprite_pending_y[6];
                                        sprite_active_image[5]  = sprite_active_image[6];
                                        sprite_active_mask[5]   = sprite_active_mask[6];
                                        sprite_active_height[5] = sprite_active_height[6];
                                        sprite_active_width[5]  = sprite_active_width[6];
                                }
                        }

                        if (i < 8) {
                                sVar1 = sprite_slot_map[spriteID];
                                sprite_pending_x[sVar1]     = sprite_pending_x[i];
                                sprite_pending_y[sVar1]     = sprite_pending_y[i];
                                sprite_active_image[sVar1]  = sprite_active_image[i];
                                sprite_active_mask[sVar1]   = sprite_active_mask[i];
                                sprite_active_height[sVar1] = sprite_active_height[i];
                                sprite_active_width[sVar1]  = sprite_active_width[i];
                                if (sVar1 != i)
                                        sprite_active_image[i] = NULL;
                        }
                        continue;
                }

                if (sprite_layer_flags[spriteID] == SPRITE_BEHIND_LCP) {
                        i = sprite_slot_map[spriteID];
                        sprite_slot_map[spriteID] = 2;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (sprite_slot_map[index] == 2) {
                                        sprite_slot_map[spriteID] = 1;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < 60;
                             index = index + 1) {
                                if (sprite_slot_map[index] ==
                                    sprite_slot_map[spriteID]) {
                                        sprite_slot_map[index] = 1;
                                        sprite_pending_x[1]     = sprite_pending_x[2];
                                        sprite_pending_y[1]     = sprite_pending_y[2];
                                        sprite_active_image[1]  = sprite_active_image[2];
                                        sprite_active_mask[1]   = sprite_active_mask[2];
                                        sprite_active_height[1] = sprite_active_height[2];
                                        sprite_active_width[1]  = sprite_active_width[2];
                                }
                        }

                        if (i < 8) {
                                sVar1 = sprite_slot_map[spriteID];
                                sprite_pending_x[sVar1]     = sprite_pending_x[i];
                                sprite_pending_y[sVar1]     = sprite_pending_y[i];
                                sprite_active_image[sVar1]  = sprite_active_image[i];
                                sprite_active_mask[sVar1]   = sprite_active_mask[i];
                                sprite_active_height[sVar1] = sprite_active_height[i];
                                sprite_active_width[sVar1]  = sprite_active_width[i];
                                if (sVar1 != i)
                                        sprite_active_image[i] = NULL;
                        }
                }
        }

        /* Second pass: zero any hardware slot not currently claimed by
           a logical sprite (prevents ghosting). */
        for (spriteID = 1; spriteID < 7; spriteID = spriteID + 1) {
                for (index = 0;
                     index < 60 && sprite_slot_map[index] != spriteID;
                     index = index + 1)
                        ;
                if (index == 60)
                        sprite_active_image[spriteID] = NULL;
        }
}

/* ---- LCP head sprite (slot 4) ----------------------------------------- */

/* sprite_lcp_head_update: pick the current head frame from PEx.LCP based
   on happiness + head_sprite_frame, expand into the double-buffer via
   sprite_lcp_flip, and drop it into slot 4.  Positioning tracks the body:
     X = lcp_x + head_x_offset[state] + (-4 or -14)
     Y = lcp_y + body_y_offset[state] - head_height[state] - 21
   Special case: while carrying an object on stairs (states 13..16), the
   head is lowered 1 px to sync with the carry animation bob.

   addr: sprite_lcp_head_update() */

void
sprite_lcp_head_update()
{
        short   headIndex;

        while (sprite_pending_flag[4] == YES)
                ;

        headIndex = happiness_head_frame_offset[lcp.happiness] +
                    (head_sprite_frame & 0x7f);

        sprite_lcp_flip((short *) (pex_lcp_file  + headIndex),
                        (short *) (head_shape_data + headIndex),
                        head_sprite_buffer, head_sprite_mask,
                        2, 21, head_sprite_mirror_flag, 0);

        if (head_sprite_mirror_flag == NO)
                sprite_active_x[4] = lcp_x + head_x_offset_per_state[lcp_state] - 4;
        else
                sprite_active_x[4] = lcp_x + head_x_offset_per_state[lcp_state] - 14;

        sprite_active_y[4] = (lcp_y + body_y_offset_per_state[lcp_state]) -
                             (head_height_per_state[lcp_state] + 21);
        if (debug_hide_lcp_offscreen != NO)
                sprite_active_y[4] = 300;

        if (lcp_carrying_object_flag != NO &&
            lcp_state > 12 && lcp_state < 17)
                sprite_active_y[4] = sprite_active_y[4] + 1;

        sprite_pending_height[4] = 21;
        sprite_pending_width[4]  = 32;
        sprite_pending_image[4]  = head_sprite_buffer;
        sprite_pending_mask[4]   = head_sprite_mask;

        if (lcp_sprites_hidden != NO)
                sprite_pending_image[4] = NULL;

        sprite_pending_flag[4] = YES;
}
