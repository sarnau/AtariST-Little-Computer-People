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
 * Most of the functions live in parts/ and are included by the unity
 * units at their places in the binary.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "protos.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"

/* sp_updb -> parts/sp_updb.c, included by stx_u3.c after gameTick. */

/* sp_ssco -> parts/sp_ssco.c, included by stx_u2.c. */

/* sp_sprs -> parts/sp_sprs.c, included by stx_u2.c. */

/* lcp_hwt -> parts/lcp_hwt.c, included by stx_u3.c immediately
   before gameTick. */

/* hideLcp -> parts/hideLcp.c, included by stx_u2.c. */

/* showLcp -> parts/showLcp.c, included by stx_u2.c. */

/* sp_ss02 -> parts/sp_ss02.c, included by stx_u2.c. */

/* sp_lcpf -> parts/sp_lcpf.c, included late in stx_u3.c. */


/* sp_flih (mirror a sprite in place, preserving width) lives in
   alerts.c, right after sp_spud. */


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

/* sp_lchu -> parts/sp_lchu.c, included by stx_u3.c after gameTick. */

/* sp_imfs: populate 8 per-slot MFDB pairs, wire compositor MFDB
   (g_srmfd) at scrbufA-aligned, call sp_drin.  Zeroes last_hz so
   the first sc_ren8 frame-gate sees 0->N delta and proceeds. */


void
sp_imfs()
{
        short   i;

        last_hz = 0;
        for (i = 0; i < SPRITE_HW_SLOTS; i++) {
                sp_iniM(0L, &g_semfi[i],
                                 (void *) g_seaim[i],
                                 g_seacw[i], g_seach[i]);
                sp_iniM(0L, &g_semfm[i],
                                 (void *) g_seams[i],
                                 g_seacw[i], g_seach[i]);
        }
        /* No temporary on purpose: the buffer base is aligned to 512
           bytes right in the argument, and both extents are 16-bit
           products. */
        sp_iniM(0L, &g_srmfd,
                (void *) (((long) scrbufA + 0x1FFL) & ~511L),
                scr_scal * 320, scr_scal * 200);
        sp_drin();
}

/* sp_drin -> parts/sp_drin.c, included by stx_u3.c after gameTick. */

/* sp_lbbd: dilate a 21-row body frame into shape data.  The source is
   168 bytes (4 shorts/row), packed per row as
       mask = src[3] | ((src[1] | src[0]) << 16) | src[2];
   Walk bits 30..1: each isolated ON bit smears into 3 (bit-1|bit|bit+1)
   -- 3-px silhouette dilation.  Second pass: vertical dilation, OR each
   row into its predecessor. */


/* sp_lbal -> parts/sp_lbal.c, included late in stx_u3.c. */


/* sp_lbbd -> parts/sp_lbbd.c, included late in stx_u3.c. */


/* sp_lbhd -> parts/sp_lbhd.c, included late in stx_u3.c. */

