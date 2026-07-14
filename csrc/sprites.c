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
 * addr: sp_updb(), sp_lchu(),
 *       sp_ssco/right(),
 *       update_carried_object_sprite() (carry branch of
 *       game_tick_and_animate)
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hsfra;
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short *  saved_body_sprite_ptr;
extern short *  saved_head_sprite_ptr;
extern short    g_hsbuf[];
extern short    g_hsmas[];
extern short    g_hsmif;
extern short *  pex_lcp_file;                   /* source head sheet */
extern short *  head_shape_data;                /* source head masks */
extern short    happiness_head_frame_offset[];
extern short    head_x_offset_per_state[];
extern short    head_height_per_state[];
extern unsigned short   revert_table[];
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_lcieo;
extern short    g_lssh;
extern short    debug_hide_lcp_offscreen;
extern short    g_sepef[];
extern short *  g_sepim[];
extern short *  g_sepms[];
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_sepeh[];
extern short    g_sepew[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seacx[];
extern short    g_seacy[];
extern short    g_seach[];
extern short    g_seacw[];
extern short *  g_sedim[];
extern short *  g_sedms[];
extern short    g_sedeh[];
extern short    g_sedew[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    body_sprite_frame_table[];
extern short    carry_body_frame_table[];
extern short    body_y_offset_per_state[];
extern short *  body_lcp_file;
extern short *  body_shape_data;
extern short    g_lsimg[];
extern short    g_lsmas[];
/* Forward-decls -- Alcyon skips these silently; modern clang under
   -Werror -std=c89 does not. */
extern void     sp_lcha();
extern void     sp_lcpf();
extern void     sp_upds();

/* sp_updb: select the body pose for the current lcp_state and
   drop it into slot 3.  When carrying an object during a walking state
   (< 25), uses the alternate arms-up frames from carry_body_frame_table.
   Positioning: X = lcp_x - 4 (right) or lcp_x - 14 (left);
   Y = lcp_y + body_y_offset[state] - 21.

   addr: sp_updb() */

void
sp_updb()
{
        short   frame;

        /* Wait out any double-buffer race on slot 3. */
        while (g_sepef[3] == YES)
                ;

        frame = body_sprite_frame_table[lcp_state];
        if (g_lcyof != NO && lcp_state < 25)
                frame = carry_body_frame_table[lcp_state];

        sp_lcpf((short *) (body_lcp_file + frame),
                        (short *) (body_shape_data + frame),
                        (short *) g_lsimg,
                        (short *) g_lsmas,
                        2, 21, lcp_facing_direction, 1);

        if (lcp_facing_direction == FACING_RIGHT)
                g_seacx[3] = lcp_x - 4;
        else
                g_seacx[3] = lcp_x - 14;

        g_seacy[3] = lcp_y + body_y_offset_per_state[lcp_state] - 21;
        if (debug_hide_lcp_offscreen != NO)
                g_seacy[3] = 300;

        g_sepeh[3] = 21;
        g_sepew[3]  = 32;
        g_sepim[3]  = g_lsimg;
        g_sepms[3]   = g_lsmas;

        if (g_lssh != NO)
                g_sepim[3] = NULL;

        g_sepef[3] = YES;
}

/* sp_ssco: activate a sprite as a carried
   object in the behind-LCP layer.  Called from action code when the
   resident picks something up.  The per-frame X/Y update happens in
   update_carried_object_sprite() below.

   addr: sp_ssco() */

void
sp_ssco(g_seix)
short   g_seix;
{
        short   slot;

        g_selaf[g_seix] = SPRITE_BEHIND_LCP;
        sp_upds();
        slot = g_seslm[g_seix];
        g_seaim[slot]  = g_sedim[g_seix];
        g_seams[slot]   = g_sedms[g_seix];
        g_seach[slot] = g_sedeh[g_seix];
        g_seacw[slot]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}

/* sp_sprs: the "generic" sprite activator used by save.c and
   the pet/petting animations.  Recomputes the 8-slot layout then copies
   the definition's image / mask / dimensions into the active-slot
   arrays.  Bypasses the pending double-buffer.

   addr: sp_sprs() */

void
sp_sprs(g_seix)
short   g_seix;
{
        short   slot;

        sp_upds();
        slot = g_seslm[g_seix];
        g_seaim[slot]  = g_sedim[g_seix];
        g_seams[slot]   = g_sedms[g_seix];
        g_seach[slot] = g_sedeh[g_seix];
        g_seacw[slot]  = g_sedew[g_seix];
}

/* lcp_wait_head_reach_target: spin ticking the animation loop until the
   head's current direction matches its target.
   addr: lcp_wait_head_reach_target() */

void
lcp_wait_head_reach_target()
{
        while (g_hacur != g_hatas)
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
        saved_body_sprite_ptr  = g_seaim[3];
        saved_head_sprite_ptr  = g_seaim[4];
        g_seaim[3] = NULL;
        g_seaim[4] = NULL;
        g_lssh     = YES;
}

/* show_lcp_sprites: restore the pointers hide_lcp_sprites() stashed.
   addr: show_lcp_sprites() */

void
show_lcp_sprites()
{
        g_seaim[3] = saved_body_sprite_ptr;
        g_seaim[4] = saved_head_sprite_ptr;
        g_lssh     = NO;
}

/* sp_ss02: as above, but places the
   carried sprite in the in-front-of-LCP layer.
   addr: sp_ss02() */

void
sp_ss02(g_seix)
short   g_seix;
{
        short   slot;

        g_selaf[g_seix] = SPRITE_IN_FRONT;
        sp_upds();
        slot = g_seslm[g_seix];
        g_seaim[slot]  = g_sedim[g_seix];
        g_seams[slot]   = g_sedms[g_seix];
        g_seach[slot] = g_sedeh[g_seix];
        g_seacw[slot]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}

/* ---- Sprite compositing pipeline -------------------------------------- */

/* sp_lcpf: expand a 2-word (32-pixel)-wide LCP source frame into
   a 4-word (64-pixel)-wide destination row, with optional horizontal
   mirror.  flipVertical selects whether the sprite content sits in the
   left half of the destination row (padding on the right) or the right
   half (padding on the left), so a right-facing frame's 32 pixels land
   at the same screen X as a left-facing one after body_sprite_frame_
   table selection.  Called from sp_updb and
   sp_lchu.

   addr: sp_lcpf() */

void
sp_lcpf(srcImg, srcMask, destImg, destMask,
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

/* sp_flih: mirror a general sprite in place.  Unlike
   sp_lcpf this preserves the source width (no row expansion);
   the caller supplies pre-sized destination buffers.
   addr: sp_flih() */

void
sp_flih(source, dest, pixelHeight, wordsWidth)
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

/* sp_upds: allocate the 60 logical sprites onto 8 hardware
   slots by layer.  Slot 3 is body, slot 4 is head (both reserved).
   Layer -1 (SPRITE_BEHIND_LCP) uses slots 1..2, layer +1 (SPRITE_IN_FRONT)
   uses slots 5..6, layer 0 (SPRITE_HIDDEN) maps to slot 9 (off-screen).
   Slot 0 and 7 are reserved for the dog.  When two logical sprites
   compete for the same slot, the older one is bumped to the alternate
   slot and its render state (image, mask, pending X/Y) is copied along.

   addr: sp_upds() */

void
sp_upds()
{
        short   spriteID;
        short   sVar1;
        short   index;
        short   i;

        if (g_selaf[0] == SPRITE_HIDDEN)
                g_seaim[g_seslm[0]] = NULL;
        if (g_selaf[1] == SPRITE_HIDDEN)
                g_seaim[g_seslm[0]] = NULL;

        for (spriteID = 3; spriteID < 60; spriteID = spriteID + 1) {
                if (g_selaf[spriteID] == SPRITE_HIDDEN) {
                        g_seslm[spriteID] = 9;
                        continue;
                }

                if (g_selaf[spriteID] == SPRITE_IN_FRONT) {
                        i = g_seslm[spriteID];
                        g_seslm[spriteID] = 6;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (g_seslm[index] == 6) {
                                        g_seslm[spriteID] = 5;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < 60;
                             index = index + 1) {
                                if (g_seslm[index] ==
                                    g_seslm[spriteID]) {
                                        g_seslm[index] = 5;
                                        g_sepex[5]     = g_sepex[6];
                                        g_sepey[5]     = g_sepey[6];
                                        g_seaim[5]  = g_seaim[6];
                                        g_seams[5]   = g_seams[6];
                                        g_seach[5] = g_seach[6];
                                        g_seacw[5]  = g_seacw[6];
                                }
                        }

                        if (i < 8) {
                                sVar1 = g_seslm[spriteID];
                                g_sepex[sVar1]     = g_sepex[i];
                                g_sepey[sVar1]     = g_sepey[i];
                                g_seaim[sVar1]  = g_seaim[i];
                                g_seams[sVar1]   = g_seams[i];
                                g_seach[sVar1] = g_seach[i];
                                g_seacw[sVar1]  = g_seacw[i];
                                if (sVar1 != i)
                                        g_seaim[i] = NULL;
                        }
                        continue;
                }

                if (g_selaf[spriteID] == SPRITE_BEHIND_LCP) {
                        i = g_seslm[spriteID];
                        g_seslm[spriteID] = 2;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (g_seslm[index] == 2) {
                                        g_seslm[spriteID] = 1;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < 60;
                             index = index + 1) {
                                if (g_seslm[index] ==
                                    g_seslm[spriteID]) {
                                        g_seslm[index] = 1;
                                        g_sepex[1]     = g_sepex[2];
                                        g_sepey[1]     = g_sepey[2];
                                        g_seaim[1]  = g_seaim[2];
                                        g_seams[1]   = g_seams[2];
                                        g_seach[1] = g_seach[2];
                                        g_seacw[1]  = g_seacw[2];
                                }
                        }

                        if (i < 8) {
                                sVar1 = g_seslm[spriteID];
                                g_sepex[sVar1]     = g_sepex[i];
                                g_sepey[sVar1]     = g_sepey[i];
                                g_seaim[sVar1]  = g_seaim[i];
                                g_seams[sVar1]   = g_seams[i];
                                g_seach[sVar1] = g_seach[i];
                                g_seacw[sVar1]  = g_seacw[i];
                                if (sVar1 != i)
                                        g_seaim[i] = NULL;
                        }
                }
        }

        /* Second pass: zero any hardware slot not currently claimed by
           a logical sprite (prevents ghosting). */
        for (spriteID = 1; spriteID < 7; spriteID = spriteID + 1) {
                for (index = 0;
                     index < 60 && g_seslm[index] != spriteID;
                     index = index + 1)
                        ;
                if (index == 60)
                        g_seaim[spriteID] = NULL;
        }
}

/* ---- LCP head sprite (slot 4) ----------------------------------------- */

/* sp_lchu: pick the current head frame from PEx.LCP based
   on happiness + g_hsfra, expand into the double-buffer via
   sp_lcpf, and drop it into slot 4.  Positioning tracks the body:
     X = lcp_x + head_x_offset[state] + (-4 or -14)
     Y = lcp_y + body_y_offset[state] - head_height[state] - 21
   Special case: while carrying an object on stairs (states 13..16), the
   head is lowered 1 px to sync with the carry animation bob.

   addr: sp_lchu() */

void
sp_lchu()
{
        short   headIndex;

        while (g_sepef[4] == YES)
                ;

        headIndex = happiness_head_frame_offset[lcp.happiness] +
                    (g_hsfra & 0x7f);

        sp_lcpf((short *) (pex_lcp_file  + headIndex),
                        (short *) (head_shape_data + headIndex),
                        g_hsbuf, g_hsmas,
                        2, 21, g_hsmif, 0);

        if (g_hsmif == NO)
                g_seacx[4] = lcp_x + head_x_offset_per_state[lcp_state] - 4;
        else
                g_seacx[4] = lcp_x + head_x_offset_per_state[lcp_state] - 14;

        g_seacy[4] = (lcp_y + body_y_offset_per_state[lcp_state]) -
                             (head_height_per_state[lcp_state] + 21);
        if (debug_hide_lcp_offscreen != NO)
                g_seacy[4] = 300;

        if (g_lcyof != NO &&
            lcp_state > 12 && lcp_state < 17)
                g_seacy[4] = g_seacy[4] + 1;

        g_sepeh[4] = 21;
        g_sepew[4]  = 32;
        g_sepim[4]  = g_hsbuf;
        g_sepms[4]   = g_hsmas;

        if (g_lssh != NO)
                g_sepim[4] = NULL;

        g_sepef[4] = YES;
}
