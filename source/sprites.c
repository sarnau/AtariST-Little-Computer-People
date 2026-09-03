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

/* sp_updb -> parts/sp_updb.c (STX: 0x148fe object, after gameTick). */
#ifdef FAITHFUL
#include "parts/sp_updb.c"
#endif

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

/* lcp_hwt -> parts/lcp_hwt.c (STX places it immediately
   before gameTick, which is why its call is a short bsr). */
#ifdef FAITHFUL
#include "parts/lcp_hwt.c"
#endif

/* hideLcp -> parts/hideLcp.c (STX: 0xdece object). */
#ifdef FAITHFUL
#include "parts/hideLcp.c"
#endif

/* showLcp -> parts/showLcp.c (STX: 0xdece object). */
#ifdef FAITHFUL
#include "parts/showLcp.c"
#endif

/* sp_ss02 -> parts/sp_ss02.c (STX: 0xdece object, 0x12108 --
   a_kitcc reaches it with a bsr). */
#ifdef FAITHFUL
#include "parts/sp_ss02.c"
#endif

/* sp_lcpf -> parts/sp_lcpf.c (STX places it late in the 0x148fe
   object -- stx_u3.c includes it there). */
#ifdef FAITHFUL
#include "parts/sp_lcpf.c"
#endif


/* sp_flih: mirror sprite in place, preserving width (no expansion).
   The STX revision links it in the alerts object right after
   sp_spud (see alerts.c).
   addr: sp_flih() */

#ifdef FAITHFUL
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

#else   /* STX: link #-10 -- spriteID, index, i; the slot index is
           recomputed at every use instead of being cached. */

void
sp_upds()
{
        short   spriteID;
        short   index;
        short   i;

        if (g_selaf[SPRITE_LCP_BODY_ID] == SPRITE_HIDDEN)
                g_seaim[g_seslm[SPRITE_LCP_BODY_ID]] = NULL;
        if (g_selaf[SPRITE_LCP_HEAD_ID] == SPRITE_HIDDEN)
                g_seaim[g_seslm[SPRITE_LCP_BODY_ID]] = NULL;

        for (spriteID = HW_SLOT_LCP_BODY; spriteID < SPRITE_SLOTS;
             spriteID++) {
                if (g_selaf[spriteID] == SPRITE_HIDDEN) {
                        g_seslm[spriteID] = HW_SLOT_NONE;
                        continue;
                }

                if (g_selaf[spriteID] == SPRITE_IN_FRONT) {
                        i = g_seslm[spriteID];
                        g_seslm[spriteID] = HW_SLOT_FRONT_PRIMARY;

                        for (index = 3; index < spriteID; index++) {
                                if (g_seslm[index] == HW_SLOT_FRONT_PRIMARY) {
                                        g_seslm[spriteID] = HW_SLOT_FRONT_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index++) {
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
                                g_sepex[g_seslm[spriteID]]  = g_sepex[i];
                                g_sepey[g_seslm[spriteID]]  = g_sepey[i];
                                g_seaim[g_seslm[spriteID]]  = g_seaim[i];
                                g_seams[g_seslm[spriteID]]  = g_seams[i];
                                g_seach[g_seslm[spriteID]]  = g_seach[i];
                                g_seacw[g_seslm[spriteID]]  = g_seacw[i];
                                if (g_seslm[spriteID] != i)
                                        g_seaim[i] = NULL;
                        }
                        continue;
                }

                if (g_selaf[spriteID] == SPRITE_BEHIND_LCP) {
                        i = g_seslm[spriteID];
                        g_seslm[spriteID] = HW_SLOT_BEHIND_PRIMARY;

                        for (index = 3; index < spriteID; index++) {
                                if (g_seslm[index] == HW_SLOT_BEHIND_PRIMARY) {
                                        g_seslm[spriteID] = HW_SLOT_BEHIND_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index++) {
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
                                g_sepex[g_seslm[spriteID]]  = g_sepex[i];
                                g_sepey[g_seslm[spriteID]]  = g_sepey[i];
                                g_seaim[g_seslm[spriteID]]  = g_seaim[i];
                                g_seams[g_seslm[spriteID]]  = g_seams[i];
                                g_seach[g_seslm[spriteID]]  = g_seach[i];
                                g_seacw[g_seslm[spriteID]]  = g_seacw[i];
                                if (g_seslm[spriteID] != i)
                                        g_seaim[i] = NULL;
                        }
                }
        }

        /* Second pass: zero any hardware slot not currently claimed by
           a logical sprite (prevents ghosting). */
        for (spriteID = HW_SLOT_BEHIND_OVERFLOW; spriteID < HW_SLOT_DOG_FRONT;
             spriteID++) {
                for (index = 0; index < SPRITE_SLOTS; index++)
                        if (g_seslm[index] == spriteID)
                                break;
                if (index == SPRITE_SLOTS)
                        g_seaim[spriteID] = NULL;
        }
}
#endif

/* sp_lchu -> parts/sp_lchu.c (STX: 0x148fe object, after gameTick). */
#ifdef FAITHFUL
#include "parts/sp_lchu.c"
#endif

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
#ifdef FAITHFUL
        for (i = 0; i < SPRITE_HW_SLOTS; i = i + 1) {
#else
        for (i = 0; i < SPRITE_HW_SLOTS; i++) {
#endif
                sp_iniM(0L, &g_semfi[i],
                                 (void *) g_seaim[i],
                                 g_seacw[i], g_seach[i]);
                sp_iniM(0L, &g_semfm[i],
                                 (void *) g_seams[i],
                                 g_seacw[i], g_seach[i]);
        }
#ifdef FAITHFUL
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
#else
        /* STX has no temporary: the relocatable base is pushed and
           masked in the argument slot (512-align), and both extents
           are 16-bit products. */
        sp_iniM(0L, &g_srmfd,
                (void *) (((long) scrbufA + 0x1FFL) & ~511L),
                scr_scal * 320, scr_scal * 200);
#endif
        sp_drin();
}

/* sp_drin -> parts/sp_drin.c (STX: 0x148fe object, after gameTick). */
#ifdef FAITHFUL
#include "parts/sp_drin.c"
#endif

/* sp_lbbd: dilate 21-row body frame -> shape data.  Source is 168 bytes
   (4 shorts/row); Ghidra packs as
       mask = src[3] | ((src[1] | src[0]) << 16) | src[2];
   Walk bits 30..1: each isolated ON bit smears into 3 (bit-1|bit|bit+1)
   -- 3-px silhouette dilation.  Second pass: vertical dilation, OR each
   row into its predecessor.
   addr: sp_lcp_build_all_body() */


/* sp_lbal -> parts/sp_lbal.c (STX places it late in the 0x148fe
   object -- stx_u3.c includes it there). */
#ifdef FAITHFUL
#include "parts/sp_lbal.c"
#endif


/* sp_lbbd -> parts/sp_lbbd.c (STX places it late in the 0x148fe
   object -- stx_u3.c includes it there). */
#ifdef FAITHFUL
#include "parts/sp_lbbd.c"
#endif


/* sp_lbhd -> parts/sp_lbhd.c (STX places it late in the 0x148fe
   object -- stx_u3.c includes it there). */
#ifdef FAITHFUL
#include "parts/sp_lbhd.c"
#endif

