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
 * All positioning is anchored to the resident's feet (resX, resY);
 * per-frame Y offsets come from bodyYOffset[].
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "protos.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"

/* flipSprite (mirror a sprite in place, preserving width) lives in
   alerts.c, right after setDogSprite. */

void
layoutSlots()
{
        short   spriteID;
        short   index;
        short   i;

        if (spriteLayer[SPRITE_LCP_BODY_ID] == SPRITE_HIDDEN)
                drawnImage[spriteSlot[SPRITE_LCP_BODY_ID]] = NULL;
        if (spriteLayer[SPRITE_LCP_HEAD_ID] == SPRITE_HIDDEN)
                drawnImage[spriteSlot[SPRITE_LCP_BODY_ID]] = NULL;

        for (spriteID = HW_SLOT_LCP_BODY; spriteID < SPRITE_SLOTS;
             spriteID++) {
                if (spriteLayer[spriteID] == SPRITE_HIDDEN) {
                        spriteSlot[spriteID] = HW_SLOT_NONE;
                        continue;
                }

                if (spriteLayer[spriteID] == SPRITE_IN_FRONT) {
                        i = spriteSlot[spriteID];
                        spriteSlot[spriteID] = HW_SLOT_FRONT_PRIMARY;

                        for (index = 3; index < spriteID; index++) {
                                if (spriteSlot[index] == HW_SLOT_FRONT_PRIMARY) {
                                        spriteSlot[spriteID] = HW_SLOT_FRONT_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index++) {
                                if (spriteSlot[index] ==
                                    spriteSlot[spriteID]) {
                                        spriteSlot[index] = HW_SLOT_FRONT_OVERFLOW;
                                        pendX[HW_SLOT_FRONT_OVERFLOW] = pendX[HW_SLOT_FRONT_PRIMARY];
                                        pendY[HW_SLOT_FRONT_OVERFLOW] = pendY[HW_SLOT_FRONT_PRIMARY];
                                        drawnImage[HW_SLOT_FRONT_OVERFLOW] = drawnImage[HW_SLOT_FRONT_PRIMARY];
                                        drawnMask[HW_SLOT_FRONT_OVERFLOW] = drawnMask[HW_SLOT_FRONT_PRIMARY];
                                        drawnHeight[HW_SLOT_FRONT_OVERFLOW] = drawnHeight[HW_SLOT_FRONT_PRIMARY];
                                        drawnWidth[HW_SLOT_FRONT_OVERFLOW] = drawnWidth[HW_SLOT_FRONT_PRIMARY];
                                }
                        }

                        if (i < SPRITE_HW_SLOTS) {
                                pendX[spriteSlot[spriteID]]  = pendX[i];
                                pendY[spriteSlot[spriteID]]  = pendY[i];
                                drawnImage[spriteSlot[spriteID]]  = drawnImage[i];
                                drawnMask[spriteSlot[spriteID]]  = drawnMask[i];
                                drawnHeight[spriteSlot[spriteID]]  = drawnHeight[i];
                                drawnWidth[spriteSlot[spriteID]]  = drawnWidth[i];
                                if (spriteSlot[spriteID] != i)
                                        drawnImage[i] = NULL;
                        }
                        continue;
                }

                if (spriteLayer[spriteID] == SPRITE_BEHIND_LCP) {
                        i = spriteSlot[spriteID];
                        spriteSlot[spriteID] = HW_SLOT_BEHIND_PRIMARY;

                        for (index = 3; index < spriteID; index++) {
                                if (spriteSlot[index] == HW_SLOT_BEHIND_PRIMARY) {
                                        spriteSlot[spriteID] = HW_SLOT_BEHIND_OVERFLOW;
                                        break;
                                }
                        }

                        for (index = spriteID + 1; index < SPRITE_SLOTS;
                             index++) {
                                if (spriteSlot[index] ==
                                    spriteSlot[spriteID]) {
                                        spriteSlot[index] = HW_SLOT_BEHIND_OVERFLOW;
                                        pendX[HW_SLOT_BEHIND_OVERFLOW] = pendX[HW_SLOT_BEHIND_PRIMARY];
                                        pendY[HW_SLOT_BEHIND_OVERFLOW] = pendY[HW_SLOT_BEHIND_PRIMARY];
                                        drawnImage[HW_SLOT_BEHIND_OVERFLOW] = drawnImage[HW_SLOT_BEHIND_PRIMARY];
                                        drawnMask[HW_SLOT_BEHIND_OVERFLOW] = drawnMask[HW_SLOT_BEHIND_PRIMARY];
                                        drawnHeight[HW_SLOT_BEHIND_OVERFLOW] = drawnHeight[HW_SLOT_BEHIND_PRIMARY];
                                        drawnWidth[HW_SLOT_BEHIND_OVERFLOW] = drawnWidth[HW_SLOT_BEHIND_PRIMARY];
                                }
                        }

                        if (i < SPRITE_HW_SLOTS) {
                                pendX[spriteSlot[spriteID]]  = pendX[i];
                                pendY[spriteSlot[spriteID]]  = pendY[i];
                                drawnImage[spriteSlot[spriteID]]  = drawnImage[i];
                                drawnMask[spriteSlot[spriteID]]  = drawnMask[i];
                                drawnHeight[spriteSlot[spriteID]]  = drawnHeight[i];
                                drawnWidth[spriteSlot[spriteID]]  = drawnWidth[i];
                                if (spriteSlot[spriteID] != i)
                                        drawnImage[i] = NULL;
                        }
                }
        }

        /* Second pass: zero any hardware slot not currently claimed by
           a logical sprite (prevents ghosting). */
        for (spriteID = HW_SLOT_BEHIND_OVERFLOW; spriteID < HW_SLOT_DOG_FRONT;
             spriteID++) {
                for (index = 0; index < SPRITE_SLOTS; index++)
                        if (spriteSlot[index] == spriteID)
                                break;
                if (index == SPRITE_SLOTS)
                        drawnImage[spriteID] = NULL;
        }
}

/* initSlots: populate 8 per-slot MFDB pairs, wire compositor MFDB
   (frameMfdb) at altScreen-aligned, call deadHook.  Zeroes last_hz so
   the first renderFrame frame-gate sees 0->N delta and proceeds. */

void
initSlots()
{
        short   i;

        last_hz = 0;
        for (i = 0; i < SPRITE_HW_SLOTS; i++) {
                initMfdb(0L, &slotImgMfdb[i],
                                 (void *) drawnImage[i],
                                 drawnWidth[i], drawnHeight[i]);
                initMfdb(0L, &slotMaskMfdb[i],
                                 (void *) drawnMask[i],
                                 drawnWidth[i], drawnHeight[i]);
        }
        /* No temporary on purpose: the buffer base is aligned to 512
           bytes right in the argument, and both extents are 16-bit
           products. */
        initMfdb(0L, &frameMfdb,
                (void *) (((long) altScreen + 0x1FFL) & ~511L),
                screenScale * 320, screenScale * 200);
        deadHook();
}

/* maskBody: dilate a 21-row body frame into shape data.  The source is
   168 bytes (4 shorts/row), packed per row as
       mask = src[3] | ((src[1] | src[0]) << 16) | src[2];
   Walk bits 30..1: each isolated ON bit smears into 3 (bit-1|bit|bit+1)
   -- 3-px silhouette dilation.  Second pass: vertical dilation, OR each
   row into its predecessor. */

