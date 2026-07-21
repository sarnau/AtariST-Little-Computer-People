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
 * per-frame Y offsets come from body_yof[].
 *
 * addr: sp_updb(), sp_lchu(),
 *       sp_ssco/right(),
 *       update_carried_object_sprite() (carry branch of
 *       gameTick)
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "sprender.h"
#include "sprglobs.h"
#include "sprhead.h"
#include "sprites.h"
#include "tables.h"
#include "tick.h"
/* Forward-decls -- Alcyon skips these silently; modern clang under
   -Werror -std=c89 does not. */

/* sp_updb: select the body pose for the current lcp_st and
   drop it into slot 3.  When carrying an object during a walking state
   (< 25), uses the alternate arms-up frames from cy_frT.
   Positioning: X = lcp_x - 4 (right) or lcp_x - 14 (left);
   Y = lcp_y + body_y_offset[state] - 21.

   addr: sp_updb() */

void
sp_updb()
{
        short   frame;

        /* Wait out any double-buffer race on slot 3. */
        while (g_sepef[HW_SLOT_LCP_BODY] == YES)
                ;

        frame = body_frT[lcp_st];
        if (g_lcyof != NO && lcp_st < STATE_BEND_AND_REACH)
                frame = cy_frT[lcp_st];

        /* Ghidra `body_ptr + frame` / `body_shp + frame`
           is shorthand for `+ frame * stride`: 168 bytes per body
           frame in body.lcp (SOURCE), 84 bytes per frame in
           body_shp (DEST).  The decompile collapses the
           multiplication into type-based scaling; the disassembly
           at Ghidra 0x2669a (`muls.w #0x54, D0`) confirms *84. */
        sp_lcpf((short *) ((char *) body_ptr    + (long) frame * (long) LCP_BODY_FRAME_SIZE),
                (short *) ((char *) body_shp  + (long) frame * (long) LCP_BODY_SHAPE_SIZE),
                (short *) g_lsimg,
                (short *) g_lsmas,
                2, 21, lcp_face, 1);

        if (lcp_face == FACING_RIGHT)
                g_seacx[HW_SLOT_LCP_BODY] = lcp_x - 4;
        else
                g_seacx[HW_SLOT_LCP_BODY] = lcp_x - 14;

        g_seacy[HW_SLOT_LCP_BODY] = lcp_y + body_yof[lcp_st] - 21;
        if (dbg_hide != NO)
                g_seacy[HW_SLOT_LCP_BODY] = 300;

        g_sepeh[HW_SLOT_LCP_BODY] = 21;
        g_sepew[HW_SLOT_LCP_BODY]  = 32;
        g_sepim[HW_SLOT_LCP_BODY]  = g_lsimg;
        g_sepms[HW_SLOT_LCP_BODY]   = g_lsmas;

        if (g_lssh != NO)
                g_sepim[HW_SLOT_LCP_BODY] = NULL;

        g_sepef[HW_SLOT_LCP_BODY] = YES;
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

/* lcp_hwt: spin ticking the animation loop until the
   head's current direction matches its target.
   addr: lcp_hwt() */

void
lcp_hwt()
{
        while (g_hacur != g_hatas)
                gameTick(0);
}

/* hideLcp: stash slot 3 (body) and slot 4 (head) active image
   pointers, nil them out, and raise the hidden flag so the sprite
   update pipeline knows to keep them cleared.  Used during the closet /
   toilet / front-door "resident enters an enclosed sprite" sequences.
   addr: hideLcp() */

void
hideLcp()
{
        sv_bodyP  = g_seaim[HW_SLOT_LCP_BODY];
        sv_headP  = g_seaim[HW_SLOT_LCP_HEAD];
        g_seaim[HW_SLOT_LCP_BODY] = NULL;
        g_seaim[HW_SLOT_LCP_HEAD] = NULL;
        g_lssh     = YES;
}

/* showLcp: restore the pointers hideLcp() stashed.
   addr: showLcp() */

void
showLcp()
{
        g_seaim[HW_SLOT_LCP_BODY] = sv_bodyP;
        g_seaim[HW_SLOT_LCP_HEAD] = sv_headP;
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
   mirror.  flipV selects whether the sprite content sits in the
   left half of the destination row (padding on the right) or the right
   half (padding on the left), so a right-facing frame's 32 pixels land
   at the same screen X as a left-facing one after body_sprite_frame_
   table selection.  Called from sp_updb and
   sp_lchu.

   addr: sp_lcpf() */

void
sp_lcpf(srcImg, srcMask, destImg, destMask,
                width, height, flipH, flipV)
short * srcImg;
short * srcMask;
short * destImg;
short * destMask;
short   width;
short   height;
short   flipH;
short   flipV;
{
        unsigned short  uVar1;
        unsigned short  mask;
        short *         psVar2;
        short           x;
        short           y;
        int             iVar3;

        if (flipH == 0) {
                for (y = 0; y < height; y = y + 1) {
                        for (x = 0; x < width; x = x + 1) {
                                psVar2 = destImg;
                                if (flipV == 0) {
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
                        /* Alcyon C 4.14 miscompiles `(unsigned char) x`:
                           it emits `ext.w` which sign-extends the low byte,
                           so a byte >= 0x80 becomes a negative rev_tab
                           index and reads garbage before the table.
                           `& 0xff` forces unsigned semantics through bit-
                           masking, which the compiler emits correctly. */
                        mask  = rev_tab[(srcImg[iVar3 + iVar3] >> 8) & 0xff] |
                                rev_tab[srcImg[iVar3 + iVar3] & 0xff] << 8;
                        uVar1 = rev_tab[(srcImg[iVar3 + iVar3 + 1] >> 8) & 0xff] |
                                rev_tab[srcImg[iVar3 + iVar3 + 1] & 0xff] << 8;

                        if (flipV == 0) {
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

                        mask = rev_tab[(srcMask[(width - 1) - x] >> 8) & 0xff] |
                               rev_tab[srcMask[(width - 1) - x] & 0xff] << 8;
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
sp_flih(source, dest, pixH, wdWidth)
unsigned short *        source;
unsigned short *        dest;
short                   pixH;
short                   wdWidth;
{
        unsigned short *        img_ptr;
        unsigned short          v;
        short                   planeIndex;
        short                   x;
        short                   y;

        for (y = 0; y < pixH; y = y + 1) {
                for (x = 0; x < wdWidth; x = x + 1) {
                        img_ptr = source + (((wdWidth - 1) - x) << 2);
                        for (planeIndex = 0; planeIndex < 4;
                             planeIndex = planeIndex + 1) {
                                v = *img_ptr;
                                img_ptr = img_ptr + 1;
                                *dest = (rev_tab[v & 0xff] << 8) |
                                         rev_tab[(v >> 8) & 0xff];
                                dest = dest + 1;
                        }
                }
                source = source + (wdWidth << 2);
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

        if (g_selaf[SPRITE_LCP_BODY_ID] == SPRITE_HIDDEN)
                g_seaim[g_seslm[SPRITE_LCP_BODY_ID]] = NULL;
        if (g_selaf[SPRITE_LCP_HEAD_ID] == SPRITE_HIDDEN)
                g_seaim[g_seslm[SPRITE_LCP_BODY_ID]] = NULL;

        for (spriteID = HW_SLOT_LCP_BODY; spriteID < SPRITE_SLOTS; spriteID = spriteID + 1) {
                if (g_selaf[spriteID] == SPRITE_HIDDEN) {
                        g_seslm[spriteID] = HW_SLOT_NONE;
                        continue;
                }

                if (g_selaf[spriteID] == SPRITE_IN_FRONT) {
                        i = g_seslm[spriteID];
                        g_seslm[spriteID] = HW_SLOT_FRONT_PRIMARY;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (g_seslm[index] == HW_SLOT_FRONT_PRIMARY) {
                                        g_seslm[spriteID] = HW_SLOT_FRONT_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index = index + 1) {
                                if (g_seslm[index] ==
                                    g_seslm[spriteID]) {
                                        g_seslm[index] = HW_SLOT_FRONT_OVERFLOW;
                                        g_sepex[HW_SLOT_FRONT_OVERFLOW] = g_sepex[HW_SLOT_FRONT_PRIMARY];
                                        g_sepey[HW_SLOT_FRONT_OVERFLOW] = g_sepey[HW_SLOT_FRONT_PRIMARY];
                                        g_seaim[HW_SLOT_FRONT_OVERFLOW] = g_seaim[HW_SLOT_FRONT_PRIMARY];
                                        g_seams[HW_SLOT_FRONT_OVERFLOW] = g_seams[HW_SLOT_FRONT_PRIMARY];
                                        g_seach[HW_SLOT_FRONT_OVERFLOW] = g_seach[HW_SLOT_FRONT_PRIMARY];
                                        g_seacw[HW_SLOT_FRONT_OVERFLOW] = g_seacw[HW_SLOT_FRONT_PRIMARY];
                                }
                        }

                        if (i < SPRITE_HW_SLOTS) {
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
                        g_seslm[spriteID] = HW_SLOT_BEHIND_PRIMARY;

                        for (index = 3; index < spriteID;
                             index = index + 1) {
                                if (g_seslm[index] == HW_SLOT_BEHIND_PRIMARY) {
                                        g_seslm[spriteID] = HW_SLOT_BEHIND_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index = index + 1) {
                                if (g_seslm[index] ==
                                    g_seslm[spriteID]) {
                                        g_seslm[index] = HW_SLOT_BEHIND_OVERFLOW;
                                        g_sepex[HW_SLOT_BEHIND_OVERFLOW] = g_sepex[HW_SLOT_BEHIND_PRIMARY];
                                        g_sepey[HW_SLOT_BEHIND_OVERFLOW] = g_sepey[HW_SLOT_BEHIND_PRIMARY];
                                        g_seaim[HW_SLOT_BEHIND_OVERFLOW] = g_seaim[HW_SLOT_BEHIND_PRIMARY];
                                        g_seams[HW_SLOT_BEHIND_OVERFLOW] = g_seams[HW_SLOT_BEHIND_PRIMARY];
                                        g_seach[HW_SLOT_BEHIND_OVERFLOW] = g_seach[HW_SLOT_BEHIND_PRIMARY];
                                        g_seacw[HW_SLOT_BEHIND_OVERFLOW] = g_seacw[HW_SLOT_BEHIND_PRIMARY];
                                }
                        }

                        if (i < SPRITE_HW_SLOTS) {
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
        for (spriteID = HW_SLOT_BEHIND_OVERFLOW; spriteID < HW_SLOT_DOG_FRONT; spriteID = spriteID + 1) {
                for (index = 0;
                     index < SPRITE_SLOTS && g_seslm[index] != spriteID;
                     index = index + 1)
                        ;
                if (index == SPRITE_SLOTS)
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

        while (g_sepef[HW_SLOT_LCP_HEAD] == YES)
                ;

        headIndex = mood_hfo[lcp.happiness] +
                    (g_hsfra & 0x7f);

        /* Same stride-scaling shape as sp_updb -- 168 bytes per head
           frame in PEx.LCP, 84 bytes per frame in hd_shp. */
        sp_lcpf((short *) ((char *) pex_ptr    + (long) headIndex * (long) LCP_BODY_FRAME_SIZE),
                (short *) ((char *) hd_shp + (long) headIndex * (long) LCP_BODY_SHAPE_SIZE),
                g_hsbuf, g_hsmas,
                2, 21, g_hsmif, 0);

        if (g_hsmif == NO)
                g_seacx[HW_SLOT_LCP_HEAD] = lcp_x + hd_xoff[lcp_st] - 4;
        else
                g_seacx[HW_SLOT_LCP_HEAD] = lcp_x + hd_xoff[lcp_st] - 14;

        g_seacy[HW_SLOT_LCP_HEAD] = (lcp_y + body_yof[lcp_st]) -
                             (hd_hgt[lcp_st] + 21);
        if (dbg_hide != NO)
                g_seacy[HW_SLOT_LCP_HEAD] = 300;

        if (g_lcyof != NO &&
            lcp_st > STATE_STR_CLIMB_F3S && lcp_st < STATE_STR_DESC_F0)
                g_seacy[HW_SLOT_LCP_HEAD] = g_seacy[HW_SLOT_LCP_HEAD] + 1;

        g_sepeh[HW_SLOT_LCP_HEAD] = 21;
        g_sepew[HW_SLOT_LCP_HEAD]  = 32;
        g_sepim[HW_SLOT_LCP_HEAD]  = g_hsbuf;
        g_sepms[HW_SLOT_LCP_HEAD]   = g_hsmas;

        if (g_lssh != NO)
                g_sepim[HW_SLOT_LCP_HEAD] = NULL;

        g_sepef[HW_SLOT_LCP_HEAD] = YES;
}

/* sp_imfs (Ghidra): populate the 8 per-slot MFDB pairs
   (image + mask) from the active sprite tables, wire the compositing
   MFDB (screen_mfdb == g_srmfd) at scrbufA + 0xCD, and call
   sp_drin.  Also zeroes last_hz so the very first sc_ren8
   frame-rate gate sees a 0->N delta and proceeds.

   Note on the offset 0xCD: same shape as stpScrB's 0x12F,
   just a different header size, and NOT rounded up here (unlike
   stpScrB's align-up-to-512).  The 1985 code left this
   compositing target unaligned; VDI raster ops don't require it and
   the shifter is pointed elsewhere via the page-flip in sc_ren8.

   addr: sp_imfs() */


void
sp_imfs()
{
        short   i;

        last_hz = 0;
        for (i = 0; i < SPRITE_HW_SLOTS; i = i + 1) {
                sp_iniM(0L, &g_semfi[i],
                                 (void *) g_seaim[i],
                                 g_seacw[i], g_seach[i]);
                sp_iniM(0L, &g_semfm[i],
                                 (void *) g_seams[i],
                                 g_seacw[i], g_seach[i]);
        }
        {
                /* Compositor screen = scrbufA rounded UP to next 512.

                   PORT DIVERGENCE FROM GHIDRA.  Raw disasm at 0x25110
                   is `move.l #0x2ca66, -(SP); andi.l #-0x200, (SP)` --
                   an align-DOWN of the compile-time constant
                   `SCREEN_BUFFER_A + 0xCD`.  That works in the 1985
                   binary only because SCREEN_BUFFER_A landed at
                   0x2c999 (low 9 bits = 0x199), which happens to make
                   (SCREEN_BUFFER_A + 0xCD) & ~0x1FF round UP to
                   0x2ca00 (still inside the buffer).  Our linker
                   places scrbufA elsewhere; a literal
                   `((long)scrbufA + 0xCD) & ~0x1FF` can round DOWN
                   to an address BEFORE scrbufA.  So we use the
                   safer align-UP pattern from stpScrB
                   (0x165ae/0x165b4).  Result: same shape (a 512-
                   aligned screen inside scrbufA), non-literal instr
                   sequence. */
                long    buf;
                buf = ((long) scrbufA + 0x200L) & ~0x1FFL;
                sp_iniM(0L, &g_srmfd, (void *) buf,
                        (short) (scr_scal * 320),
                        (short) (scr_scal * 200));
        }
        sp_drin();
}

/* sp_drin (Ghidra): empty stub.  Present in the 1985 code
   as a hook -- probably a wired-up function pointer table entry that
   was reduced to a no-op in the final build.
   addr: sp_drin() */

void
sp_drin()
{
}

/* sp_lbbd (Ghidra sprite_lcp_build_all_body):
   dilate one 21-row body frame into shape data.  The 168-byte source
   is 4 shorts per row (a 64-bit sprite word split across four shorts,
   read in the order src[0]|src[1] << 16 | src[2] << 16 (?)) -- Ghidra
   decompiled the pack as
       mask = src[3] | ((src[1] | src[0]) << 16) | src[2];
   Each row's mask is walked bit-by-bit from bit 30 down to bit 1,
   and every isolated ON bit is smeared into its neighbours (bit-1,
   bit, bit+1 all set) -- a 3-pixel-wide dilation that produces the
   silhouette outline.  A second pass then bit-ORs each row into its
   predecessor for vertical dilation.
   addr: sp_lcp_build_all_body() */


void
sp_lbbd(src, dest, height)
unsigned short *src;
unsigned short *dest;
short           height;
{
        long   img;
        long   mask;
        short           bit;
        short           h;
        short           flag;
        unsigned short *dp;

        dp = dest;
        for (h = 0; h < height; h = h + 1) {
                img  = 0L;
                mask = ((long) (src[3] | src[2]))
                     | (((long) (src[1] | src[0])) << 16);
                src = src + 4;
                flag = 0;
                for (bit = 30; bit > 0; bit = bit - 1) {
                        if (flag) {
                                img = img | bm32or[bit];
                                if ((bm32or[bit] & mask) == 0L)
                                        flag = 0;
                        } else if ((bm32or[bit] & mask) != 0L) {
                                img = img
                                      | bm32or[bit - 1]
                                      | bm32or[bit]
                                      | bm32or[bit + 1];
                                flag = 1;
                        }
                }
                dest[0] = (unsigned short) (img >> 16);
                dest[1] = (unsigned short) img;
                dest = dest + 2;
        }
        /* Vertical dilation: OR each row into the row above. */
        dest = dest - 1;
        for (h = 0; h < (short) (height - 1); h = h + 1) {
                dest[0]  = dest[-2] | dest[0];
                dest[-1] = dest[-3] | dest[-1];
                dest = dest - 2;
        }
        (void) dp;
}

/* sp_lbhd (Ghidra sprite_lcp_build_all_head):
   dilate one 21-row head frame.  Same source packing as sp_lbbd but a
   different bit-smearing rule:
     - start with mask = 0xFFFFFFFF and img = the packed source pixels
     - shrink mask from bit 31 downwards until bit_or[bit-1] & img != 0
     - shrink mask from bit 0 upwards   until bit_or[bit+1] & img != 0
   The result is a mask that covers exactly the horizontal extent of
   the ON pixels (plus 1 bit of slack on each side) -- the head's
   convex-hull outline instead of a pixel-precise dilation.
   Then the same vertical-OR merge as sp_lbbd.
   addr: sp_lcp_build_all_head() */


void
sp_lbhd(src, dest, height)
unsigned short *src;
unsigned short *dest;
short           height;
{
        long   img;
        long   mask;
        short           bit;
        short           h;
        unsigned short *dp;

        dp = dest;
        for (h = 0; h < height; h = h + 1) {
                mask = -1L;
                img  = ((long) (src[3] | src[2]))
                     | (((long) (src[1] | src[0])) << 16);
                src = src + 4;
                bit = 31;
                while (bit > 0 &&
                       (bm32or[bit - 1] & img) == 0L) {
                        mask = bm32and[bit] & mask;
                        bit = bit - 1;
                }
                bit = 0;
                while (bit < 31 &&
                       (bm32or[bit + 1] & img) == 0L) {
                        mask = bm32and[bit] & mask;
                        bit = bit + 1;
                }
                dest[0] = (unsigned short) (mask >> 16);
                dest[1] = (unsigned short) mask;
                dest = dest + 2;
        }
        /* Vertical dilation: OR each row into the row BELOW (opposite
           direction to sp_lbbd). */
        dest = dp;
        for (h = 0; h < (short) (height - 1); h = h + 1) {
                dest[0] = dest[2] | dest[0];
                dest[1] = dest[3] | dest[1];
                dest = dest + 2;
        }
}

/* sp_lbal (Ghidra sprite_lcp_build_all):
   dispatcher.  Iterate 98 body frames (168 source bytes each ->
   84 dest bytes each) then 66 head frames, calling sp_lbbd /
   sp_lbhd.  Sources come from body_ptr / pex_ptr loaded
   by fLoad; destinations are the BSS arrays body_shape_data_buf
   / head_shape_data_buf we allocate in sprglobs.c.
   addr: sp_lcp_build_all() */

void
sp_lbal()
{
        short   index;
        char *  src_ptr;
        char *  dst_ptr;

        /* Precompute walking pointers instead of the more obvious
           `body_ptr + index * 168` per iteration -- the Alcyon
           codegen for the (short * ) + (int * short) expression
           crashed on the very first iteration in testing; walking
           two char* accumulators is byte-for-byte equivalent and
           compiles cleanly. */
        src_ptr = (char *) body_ptr;
        dst_ptr = (char *) body_shp;
        for (index = 0; index < 98; index = index + 1) {
                sp_lbbd((unsigned short *) src_ptr,
                        (unsigned short *) dst_ptr, 21);
                src_ptr = src_ptr + LCP_BODY_FRAME_SIZE;
                dst_ptr = dst_ptr + LCP_BODY_SHAPE_SIZE;
        }
        src_ptr = (char *) pex_ptr;
        dst_ptr = (char *) hd_shp;
        for (index = 0; index < 66; index = index + 1) {
                sp_lbhd((unsigned short *) src_ptr,
                        (unsigned short *) dst_ptr, 21);
                src_ptr = src_ptr + LCP_BODY_FRAME_SIZE;
                dst_ptr = dst_ptr + LCP_BODY_SHAPE_SIZE;
        }
}
