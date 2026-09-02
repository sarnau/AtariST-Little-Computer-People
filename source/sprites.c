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

/* sp_updb: select body pose for lcp_st -> slot 3.  When carrying an
   object during walk states (< 25), uses arms-up frames from cy_frT.
   X = lcp_x - 4 (right) or lcp_x - 14 (left); Y = lcp_y + body_yof[st] - 21.
   addr: sp_updb() */

void
sp_updb()
{
        short   frame;

        while (g_sepef[HW_SLOT_LCP_BODY] == YES)
                ;

        frame = body_frT[lcp_st];
        /* STX spells the bound inclusively on the previous state
           (cmpi #24/bgt) where LCP_ORG uses < 25 (cmpi #25/bge). */
#ifdef FAITHFUL
        if (g_lcyof != NO && lcp_st < STATE_BEND_AND_REACH)
#else
        if (g_lcyof != NO && lcp_st <= STATE_STR_BTM_F3)
#endif
                frame = cy_frT[lcp_st];

        /* Ghidra 0x2669a `muls.w #0x54, D0`: stride is 168 src, 84 dest. */
        /* STX multiplies in word width (muls.w) -- no (long) casts,
           so no call to the long-multiply helper. */
#ifdef FAITHFUL
        sp_lcpf((short *) ((char *) body_ptr    + (long) frame * (long) LCP_BODY_FRAME_SIZE),
                (short *) ((char *) body_shp  + (long) frame * (long) LCP_BODY_SHAPE_SIZE),
#else
        sp_lcpf((short *) ((char *) body_ptr    + frame * LCP_BODY_FRAME_SIZE),
                (short *) ((char *) body_shp  + frame * LCP_BODY_SHAPE_SIZE),
#endif
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

/* sp_ssco -> parts/sp_ssco.c (STX puts it in the 0xdece object;
   FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/sp_ssco.c"
#endif

/* sp_sprs -> parts/sp_sprs.c (STX puts it in the 0xdece object;
   FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/sp_sprs.c"
#endif

/* lcp_hwt: tick until g_hacur == g_hatas.
   addr: lcp_hwt() */

void
lcp_hwt()
{
        while (g_hacur != g_hatas)
                gameTick(0);
}

/* hideLcp -> parts/hideLcp.c (STX: 0xdece object). */
#ifdef FAITHFUL
#include "parts/hideLcp.c"
#endif

/* showLcp -> parts/showLcp.c (STX: 0xdece object). */
#ifdef FAITHFUL
#include "parts/showLcp.c"
#endif

/* sp_ss02: same as sp_ssco but in the in-front-of-LCP layer.
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

/* sp_lcpf: expand 2-word (32-px) LCP source frame into 4-word (64-px)
   dest row, with optional horizontal mirror.  flipV picks left- vs
   right-half so mirrored frames land at the same screen X.
   Called from sp_updb and sp_lchu.
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
                        /* Alcyon 4.14 miscompiles (unsigned char) with
                           ext.w -- and the ROM SHIPPED that way (asr.w
                           #8 + ext.w at 0xbd66): bytes >= 0x80 index
                           rev_tab negatively.  Frames never set those
                           bits, so it is benign; keep the ROM bytes. */
                        mask  = rev_tab[(unsigned char) (srcImg[iVar3 + iVar3] >> 8)] |
                                rev_tab[(unsigned char) srcImg[iVar3 + iVar3]] << 8;
                        uVar1 = rev_tab[(unsigned char) (srcImg[iVar3 + iVar3 + 1] >> 8)] |
                                rev_tab[(unsigned char) srcImg[iVar3 + iVar3 + 1]] << 8;

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

                        mask = rev_tab[(unsigned char) (srcMask[(width - 1) - x] >> 8)] |
                               rev_tab[(unsigned char) srcMask[(width - 1) - x]] << 8;
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

/* sp_flih: mirror sprite in place, preserving width (no expansion).
   The STX revision links it in the alerts object right after
   sp_spud (see alerts.c).
   addr: sp_flih() */

#ifdef FAITHFUL
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
#endif  /* FAITHFUL */

/* sp_upds: allocate 60 logical sprites onto 8 hardware slots by layer.
   Slots: 3=body, 4=head (reserved), 0/7=dog, 1..2=behind, 5..6=front,
   9=off-screen (hidden).  On collision, older sprite bumps to overflow
   slot with its render state copied.
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

/* sp_lchu: pick head frame from PEx.LCP by happiness + g_hsfra,
   expand via sp_lcpf into slot 4.  Tracks body position; head lowers
   1 px while carrying on stair states 13..16.
   addr: sp_lchu() */

void
sp_lchu()
{
        short   headIndex;

        while (g_sepef[HW_SLOT_LCP_HEAD] == YES)
                ;

        headIndex = mood_hfo[lcp.happiness] +
                    (g_hsfra & 0x7f);

        /* Same 168-src/84-dest stride as sp_updb. */
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

/* sp_imfs: populate 8 per-slot MFDB pairs, wire compositor MFDB
   (g_srmfd) at scrbufA-aligned, call sp_drin.  Zeroes last_hz so
   the first sc_ren8 frame-gate sees 0->N delta and proceeds.
   Note: 1985 code left this compositing target unaligned; VDI doesn't
   require alignment and the shifter is pointed elsewhere via sc_ren8.
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
                /* PORT DIVERGENCE FROM GHIDRA (align-UP vs align-DOWN).
                   Ghidra 0x25110 does align-DOWN of the compile-time
                   constant scrbufA+0xCD; this only worked because 1985
                   scrbufA landed at 0x2c999 so the result rounded UP to
                   0x2ca00 anyway.  Our linker places scrbufA elsewhere;
                   literal align-DOWN can produce an address BEFORE
                   scrbufA.  Use safer align-UP from stpScrB
                   (0x165ae/0x165b4) -- same shape, different instrs. */
                long    buf;
                buf = ((long) scrbufA + 0xFFL) & ~0xFFL;        /* ROM: 256-align */
                sp_iniM(0L, &g_srmfd, (void *) buf,
                        (short) (scr_scal * 320),
                        (short) (scr_scal * 200));
        }
        sp_drin();
}

/* sp_drin: empty in the 1985 code (dead hook).
   addr: sp_drin() */

void
sp_drin()
{
}

/* sp_lbbd: dilate 21-row body frame -> shape data.  Source is 168 bytes
   (4 shorts/row); Ghidra packs as
       mask = src[3] | ((src[1] | src[0]) << 16) | src[2];
   Walk bits 30..1: each isolated ON bit smears into 3 (bit-1|bit|bit+1)
   -- 3-px silhouette dilation.  Second pass: vertical dilation, OR each
   row into its predecessor.
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

/* sp_lbhd: dilate 21-row head frame.  Same packing as sp_lbbd but:
   start with mask = 0xFFFFFFFF, shrink from bit 31 down and bit 0 up
   until the next bit hits set img pixels -- convex-hull outline plus
   1 bit slack.  Then vertical-OR merge (opposite direction to sp_lbbd).
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
        /* Vertical dilation: OR each row into the row BELOW. */
        dest = dp;
        for (h = 0; h < (short) (height - 1); h = h + 1) {
                dest[0] = dest[2] | dest[0];
                dest[1] = dest[3] | dest[1];
                dest = dest + 2;
        }
}

/* sp_lbal: dispatch 98 body + 66 head frames through sp_lbbd/sp_lbhd.
   addr: sp_lcp_build_all() */

void
sp_lbal()
{
        short   index;
        char *  src_ptr;
        char *  dst_ptr;

        /* Walking char* accumulators (not body_ptr + index * 168):
           Alcyon miscompiled the (short*) + (int * short) form. */
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
