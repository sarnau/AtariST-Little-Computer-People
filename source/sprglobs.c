/* sprglobs.c -- storage for sprite pipeline, LCP animation, dog state. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "sprglobs.h"
#include "sprites.h"

/* ---- LCP animation ----------------------------------------------------- */
/* PLAYER_STATE for the LCP character sprite; drives bodyIndex +
   bodyYOffset + head-offset lookups.  Starts at 0 (BSS); moveInScene sets
   this to STATE_STAND_SIDE_VIEW (34) before gameLoop starts. */
short   animState;
/* Facing direction (FACING_RIGHT / FACING_LEFT).  Selects the mirror
   path in expandFrame and biases per-state head X offsets. */
short   resFacing;
/* YES while the LCP is holding a bookshelf item / grocery / game box;
   enables the alternate arms-up carry body-frame table (carryFrames) for
   walking states 0..24. */
short   isCarrying;
/* Set by sprite selection to remember
   which sprite slot the carried-item overlay came from so the
   depth-compositor can flip it in front/behind on stairs.  -1 =
   nothing carried. */
short   carriedSprite;
/* While YES the compositor forces the
   body/head sprite pointers to NULL so the character disappears
   (used by moveInScene while the LCP is off-screen, and by the study
   door-close cutscene). */
short   lcpHidden;
/* Diagnostic-only: while YES, the body and head sprite updates
   (updateBody / updateHead) force the sprite Y to 300 (below the
   visible area) so a developer can look at the empty room. */
short   debugHideLcp;

/* ---- Dog --------------------------------------------------------------- */
/* Current screen position of the dog sprite (updated ~8Hz by
   moveDog). */
short   dogX;
short   dogY;
/* Final destination the dog is heading to
   (set by the AI when it picks a new wander destination).  When both
   are 0 the dog is considered idle at its current spot. */
short   dogXTarget;
short   dogYTarget;
/* Intermediate stair-transition point
   between dog and target when they're on different floors.  When
   both are 0 no waypoint is active. */
short   dogXWaypt;
short   dogYWaypt;
/* 0..7 index into dogWalkSprites[] that
   rotates the walk-cycle frame on each tick.  Wraps at 8. */
short   dogStepIdx;
/* Current SPRITE_DOG_* id pushed into the
   hardware slot each tick (walk frame, lay-down, sit, etc.). */
short   dogSpriteId;
/* YES while the dog is traversing a
   flight; steers moveDog through the stair-jump table
   instead of the flat-floor step logic. */
short   dogOnStairs;
/* dogHidden: when non-zero setDogSprite skips writing sprite slots
   0/7 (used to hide the dog while off-screen).  Starts at 0 (BSS),
   so the dog is visible from frame one. */
short   dogHidden;
short * pendImage[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_image: image bitmap for next draw */
short * pendMask[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_mask: 1-bit AND mask for next draw */
short   pendX[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_x: X for next draw */
short   pendY[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_y: Y for next draw */
short   pendHeight[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_height: rows for next draw */
short   pendWidth[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_width: pixels for next draw */
short * drawnImage[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_image: image currently drawn */
short * drawnMask[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_mask: mask currently drawn */
short   drawnX[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_x: X currently drawn */
short   drawnY[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_y: Y currently drawn */
short   drawnHeight[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_height: rows currently drawn */
short   drawnWidth[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_width: pixels currently drawn */

/* ---- Sprite definitions (SPRITE_SLOTS logical slots) -------------------
   Populated once by defineSprite from the SPRITES asset file
   at boot.  Each logical slot holds a sprite's image bitmap, mask,
   height, and width; the hardware-slot pipeline above copies from
   these when a logical sprite is pushed to the screen. */
short * spriteBitmap[SPRITE_SLOTS];          /* sprite_def_image[SPRITE_ID] */
short * spriteMask[SPRITE_SLOTS];          /* sprite_def_mask[SPRITE_ID] */
short   spriteHeight[SPRITE_SLOTS];          /* sprite_def_height[SPRITE_ID] */
short   spriteWidth[SPRITE_SLOTS];          /* sprite_def_width[SPRITE_ID] */

/* The body and shape buffers must be ARRAYS, not pointer variables:
   indexing a real global array is what the original code compiles
   from. */
unsigned char   bodyFrames[BODY_FRAME_SLOTS][LCP_BODY_FRAME_SIZE];
unsigned char   bodyShapes[BODY_FRAMES][LCP_BODY_SHAPE_SIZE];
/* bodyShapes buffer (BODY_FRAMES * LCP_BODY_SHAPE_SIZE = 8232 bytes):
   destination for buildMasks's 30-bit dilation of the
   raw 168-byte body frames.  84 bytes = 21 rows * 2 words per row.
   BSS-resident so it survives to game end without heap traffic. */
short   bodyImage[LCP_BODY_DEST_WORDS];    /* expandFrame dest: image plane pair */
short   bodyMask[LCP_BODY_DEST_WORDS];    /* expandFrame dest: mask plane pair */
/* defineSprite populates spriteBitmap/spriteMask above; activateSprite/carryBehind/carryInFront/
   setDogSprite all read from the same arrays.  There is no separate "dog only" table -- the
   original binary has one 60-entry sprite pointer table shared by
   every registered sprite. */
/* dogMirImage / dogMirMask: the dog's mirrored image and mask (240 bytes =
   15 rows * 2 word-width * 4 planes * 2 bytes/word), written by flipSprite
   when the dog needs a mirrored frame. */
short   dogMirImage[120];
short   dogMirMask[120];

/* Increments on every 8 Hz
   render tick; used by redrawHands etc. to drive slow overlay
   animations (pendulum, phone-hook rocking, dog tail wag). */
short   tickCount;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   headImage[LCP_BODY_DEST_WORDS];        /* expandFrame dest: head image */
short   headMask[LCP_BODY_DEST_WORDS];        /* expandFrame dest: head mask */
/* Set by stepHead to select
   the horizontal-flip path in expandFrame when the head faces
   the opposite direction from the body. */
short   headMirror;
/* The loaded PEx.LCP frame table and the dilated head silhouettes are
   ARRAYS, not pointers, exactly like bodyFrames and bodyShapes. */
unsigned char   pexFrames[HEAD_FRAMES][LCP_BODY_FRAME_SIZE];
unsigned char   headShapes[HEAD_FRAMES][LCP_BODY_SHAPE_SIZE];

/* ---- Walk-pathfinding state ------------------------------------------ */
/* Intermediate waypoint the LCP walks through to reach `walkXTarget`/`walkYTarget`
   on a different floor; set by nextWaypoint, zeroed on
   arrival.  All-zero = no waypoint active. */
short   xWaypoint;
short   yWaypoint;
/* Latched YES on walk-cycle frames
   3 and 7 to schedule the next footstep SFX; consumed and cleared by
   playFootstep. */
BOOL16  footstepDue;
/* Snapshot of the head animation
   state from the previous tick; stepHead diffs against this to
   detect direction changes and pick the transition frame. */
short   headLastWalk;

/* Shared mask buffer (14 KB): defineSprite writes each sprite's
   generated transparency mask here, parallel to its image bytes in
   sprFileBuf, and spriteMask[id] points at that slice. */
unsigned char   genMaskBuf[14000];

/* 8-bit bit-reversal table used to mirror sprites.  Not shipped as
   data: initMirror builds it at boot from mirrorSrcBit/mirrorDstBit, so
   it lives in BSS. */
short           mirrorTable[256];
