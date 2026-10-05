/* sprglobs.c -- storage for sprite pipeline, LCP animation, dog state. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "sprglobs.h"
#include "sprites.h"

/* ---- LCP animation ----------------------------------------------------- */
/* PLAYER_STATE for the LCP character sprite; drives body_frT +
   body_yof + head-offset lookups.  Starts at 0 (BSS); moveInScene sets
   this to STATE_STAND_SIDE_VIEW (34) before gameLoop starts. */
short   lcp_st;
/* Facing direction (FACING_RIGHT / FACING_LEFT).  Selects the mirror
   path in expandFrame and biases per-state head X offsets. */
short   lcp_face;
/* YES while the LCP is holding a bookshelf item / grocery / game box;
   enables the alternate arms-up carry body-frame table (cy_frT) for
   walking states 0..24. */
short   g_lcyof;
/* Set by sprite selection to remember
   which sprite slot the carried-item overlay came from so the
   depth-compositor can flip it in front/behind on stairs.  -1 =
   nothing carried. */
short   g_lcieo;
/* While YES the compositor forces the
   body/head sprite pointers to NULL so the character disappears
   (used by moveInScene while the LCP is off-screen, and by the study
   door-close cutscene). */
short   g_lssh;
/* Diagnostic-only: while YES, the body and head sprite updates
   (updateBody / updateHead) force the sprite Y to 300 (below the
   visible area) so a developer can look at the empty room. */
short   dbg_hide;

/* ---- Dog --------------------------------------------------------------- */
/* Current screen position of the dog sprite (updated ~8Hz by
   moveDog). */
short   dog_x;
short   dog_y;
/* Final destination the dog is heading to
   (set by the AI when it picks a new wander destination).  When both
   are 0 the dog is considered idle at its current spot. */
short   g_dtx;
short   g_dty;
/* Intermediate stair-transition point
   between dog and target when they're on different floors.  When
   both are 0 no waypoint is active. */
short   g_dyx;
short   g_dyy;
/* 0..7 index into g_dwanf[] that
   rotates the walk-cycle frame on each tick.  Wraps at 8. */
short   g_dwanc;
/* Current SPRITE_DOG_* id pushed into the
   hardware slot each tick (walk frame, lay-down, sit, etc.). */
short   g_dsid;
/* YES while the dog is traversing a
   flight; steers moveDog through the stair-jump table
   instead of the flat-floor step logic. */
short   dg_stair;
/* dg_init: when non-zero setDogSprite skips writing sprite slots
   0/7 (used to hide the dog while off-screen).  Starts at 0 (BSS),
   so the dog is visible from frame one. */
short   dg_init;
short * g_sepim[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_image: image bitmap for next draw */
short * g_sepms[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_mask: 1-bit AND mask for next draw */
short   g_sepex[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_x: X for next draw */
short   g_sepey[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_y: Y for next draw */
short   g_sepeh[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_height: rows for next draw */
short   g_sepew[SPRITE_HW_SLOTS_ALLOC]; /* sprite_pending_width: pixels for next draw */
short * g_seaim[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_image: image currently drawn */
short * g_seams[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_mask: mask currently drawn */
short   g_seacx[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_x: X currently drawn */
short   g_seacy[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_y: Y currently drawn */
short   g_seach[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_height: rows currently drawn */
short   g_seacw[SPRITE_HW_SLOTS_ALLOC]; /* sprite_active_width: pixels currently drawn */

/* ---- Sprite definitions (SPRITE_SLOTS logical slots) -------------------
   Populated once by defineSprite from the SPRITES asset file
   at boot.  Each logical slot holds a sprite's image bitmap, mask,
   height, and width; the hardware-slot pipeline above copies from
   these when a logical sprite is pushed to the screen. */
short * g_sedim[SPRITE_SLOTS];          /* sprite_def_image[SPRITE_ID] */
short * g_sedms[SPRITE_SLOTS];          /* sprite_def_mask[SPRITE_ID] */
short   g_sedeh[SPRITE_SLOTS];          /* sprite_def_height[SPRITE_ID] */
short   g_sedew[SPRITE_SLOTS];          /* sprite_def_width[SPRITE_ID] */

/* The body and shape buffers must be ARRAYS, not pointer variables:
   indexing a real global array is what the original code compiles
   from. */
unsigned char   body_ptr[120][168];     /* LCP_BODY_FRAME_SIZE */
unsigned char   body_shp[98][84];       /* LCP_BODY_SHAPE_SIZE */
/* body_shp buffer (98 * 84 = 8232 bytes):
   destination for buildMasks's 30-bit dilation of the
   raw 168-byte body frames.  84 bytes = 21 rows * 2 words per row.
   BSS-resident so it survives to game end without heap traffic. */
short   g_lsimg[LCP_BODY_DEST_WORDS];    /* expandFrame dest: image plane pair */
short   g_lsmas[LCP_BODY_DEST_WORDS];    /* expandFrame dest: mask plane pair */
/* defineSprite populates g_sedim/g_sedms above; activateSprite/carryBehind/carryInFront/
   setDogSprite all read from the same arrays.  There is no separate "dog only" table -- the
   original binary has one 60-entry sprite pointer table shared by
   every registered sprite. */
/* g_dfimb / g_dfmab: the dog's mirrored image and mask (240 bytes =
   15 rows * 2 word-width * 4 planes * 2 bytes/word), written by flipSprite
   when the dog needs a mirrored frame. */
short   g_dfimb[120];
short   g_dfmab[120];

/* Increments on every 8 Hz
   render tick; used by redrawHands etc. to drive slow overlay
   animations (pendulum, phone-hook rocking, dog tail wag). */
short   subAniC;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   g_hsbuf[LCP_BODY_DEST_WORDS];        /* expandFrame dest: head image */
short   g_hsmas[LCP_BODY_DEST_WORDS];        /* expandFrame dest: head mask */
/* Set by stepHead to select
   the horizontal-flip path in expandFrame when the head faces
   the opposite direction from the body. */
short   g_hsmif;
/* The loaded PEx.LCP frame table and the dilated head silhouettes are
   ARRAYS, not pointers, exactly like body_ptr and body_shp. */
unsigned char   pex_ptr[66][168];       /* LCP_BODY_FRAME_SIZE */
unsigned char   hd_shp[66][84];         /* LCP_BODY_SHAPE_SIZE */

/* ---- Walk-pathfinding state ------------------------------------------ */
/* Intermediate waypoint the LCP walks through to reach `g_wtx`/`g_wty`
   on a different floor; set by nextWaypoint, zeroed on
   arrival.  All-zero = no waypoint active. */
short   g_wyx;
short   g_wyy;
/* Latched YES on walk-cycle frames
   3 and 7 to schedule the next footstep SFX; consumed and cleared by
   playFootstep. */
BOOL16  fs_trg;
/* Snapshot of the head animation
   state from the previous tick; stepHead diffs against this to
   detect direction changes and pick the transition frame. */
short   g_hastl;
