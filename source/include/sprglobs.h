/* sprglobs.h -- extern declarations for sprglobs.c. */

#ifndef SPRGLOBS_H
#define SPRGLOBS_H

#include "enums.h"        /* LCP_BODY_FRAME_SIZE / LCP_BODY_SHAPE_SIZE */

#include "types.h"

/* Number of logical sprite-definition slots: one shared 60-entry
   table; slot IDs are
   SPRITE_* enum values in include/enums.h.  The sprite-slot map
   `spriteSlot[SPRITE_SLOTS]` picks which of the SPRITE_HW_SLOTS
   hardware slots each logical sprite lands on. */
#define SPRITE_SLOTS    60

/* Number of hardware sprite slots the compositor maintains.  Slot
   roles are named individually below. */
#define SPRITE_HW_SLOTS         8

/* Hardware-slot IDs (indices into pendImage/pendMask/drawnImage/drawnMask/
   drawnX/drawnY/etc.).  Z-order runs low-to-high back-to-front:
   slot 0 draws first, slot 7 draws last on top of everything.

     0  DOG_BACK        dog when depth puts it behind the LCP
     1  BEHIND_OVERFLOW behind-LCP overlay, secondary
     2  BEHIND_PRIMARY  behind-LCP overlay, primary (compositor
                        assigns here first for SPRITE_BEHIND_LCP;
                        overflow falls back to slot 1)
     3  LCP_BODY        the character body sprite
     4  LCP_HEAD        the character head sprite (drawn over body)
     5  FRONT_OVERFLOW  in-front overlay, secondary
     6  FRONT_PRIMARY   in-front overlay, primary (compositor
                        assigns here first for SPRITE_IN_FRONT;
                        overflow falls back to slot 5)
     7  DOG_FRONT       dog when depth puts it in front of the LCP

   `HW_SLOT_NONE` is a sentinel value stored in spriteSlot[] meaning
   "this logical sprite is not currently mapped to any hardware
   slot" (i.e. not drawn). */
#define HW_SLOT_DOG_BACK        0
#define HW_SLOT_BEHIND_OVERFLOW 1
#define HW_SLOT_BEHIND_PRIMARY  2
#define HW_SLOT_LCP_BODY        3
#define HW_SLOT_LCP_HEAD        4
#define HW_SLOT_FRONT_OVERFLOW  5
#define HW_SLOT_FRONT_PRIMARY   6
#define HW_SLOT_DOG_FRONT       7
#define HW_SLOT_NONE            9

/* Allocation size for the hardware-slot pending/active arrays below.
   Logical render slots are 0..7 (SPRITE_HW_SLOTS); HW_SLOT_NONE (9) is
   the "disabled" slot that layoutSlots parks HIDDEN sprites in.  gameTick's
   carrying path writes pendX/pendY[spriteSlot[carriedSprite]] every frame,
   and carryBehind/carryInFront write drawnImage/drawnMask/drawnHeight/drawnWidth the same
   way -- so any of these arrays can be indexed at HW_SLOT_NONE when a
   carried sprite is momentarily hidden.  The original's 8-entry
   arrays tolerate the [9] write because it overflows into the
   ADJACENT array (pendY[9] == drawnWidth[1], a harmless short) -- which
   is only true while the BSS layout is the original's.  So the arrays
   stay at 8 and the safety rests on that adjacency; widening them
   would change the layout.  Loops and bounds checks use
   SPRITE_HW_SLOTS (8). */
#define SPRITE_HW_SLOTS_ALLOC   SPRITE_HW_SLOTS

extern short animState;
extern short resFacing;
extern short isCarrying;
extern short carriedSprite;
extern short lcpHidden;
extern short debugHideLcp;
extern short dogX;
extern short dogY;
extern short dogXTarget;
extern short dogYTarget;
extern short dogXWaypt;
extern short dogYWaypt;
extern short dogStepIdx;
extern short dogSpriteId;
extern short dogOnStairs;
extern short dogHidden;
extern short pendReady[];
extern short* pendImage[];
extern short* pendMask[];
extern short pendX[];
extern short pendY[];
extern short pendHeight[];
extern short pendWidth[];
extern short* drawnImage[];
extern short* drawnMask[];
extern short drawnX[];
extern short drawnY[];
extern short drawnHeight[];
extern short drawnWidth[];
extern short* spriteBitmap[];
extern short* spriteMask[];
extern short spriteHeight[];
extern short spriteWidth[];
extern short spriteLayer[];
extern short spriteSlot[];
extern short bodyIndex[];
extern short carryFrames[];
extern short bodyYOffset[];
extern unsigned char bodyFrames[][LCP_BODY_FRAME_SIZE];
extern unsigned char bodyShapes[][LCP_BODY_SHAPE_SIZE];
extern short bodyImage[];
extern short bodyMask[];
extern short dogWalkSprites[];
extern short dogMirImage[];
extern short dogMirMask[];
extern short floorBottomY[];
extern short floorWalkY[];
extern short stairWaypts[];
extern short tickCount;
extern short headImage[];
extern short headMask[];
extern BOOL16 headMirror;
extern unsigned char pexFrames[][LCP_BODY_FRAME_SIZE];
extern unsigned char headShapes[][LCP_BODY_SHAPE_SIZE];
extern short headDelay;
extern short moodHeadBase[];
extern short headXOffset[];
extern short headYOffset[];
extern short headRestDir[];
extern short headTurnStep[];
extern short headTiltFrame[];
extern short xWaypoint;
extern short yWaypoint;
extern short onStairs;
extern BOOL16 footstepDue;
extern short headLastWalk;
extern short xLanding;
extern short yLanding;

#endif /* SPRGLOBS_H */
