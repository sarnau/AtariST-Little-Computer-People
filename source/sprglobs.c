/* sprglobs.c -- storage for sprite pipeline, LCP animation, dog state. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "sprglobs.h"
#include "sprites.h"

/* ---- LCP animation ----------------------------------------------------- */
/* PLAYER_STATE for the LCP character sprite; drives body_frT +
   body_yof + head-offset lookups.  Ghidra BSS = 0.  cs_mvIn sets
   this to STATE_STAND_SIDE_VIEW (34) before gameLoop starts. */
short   lcp_st;
/* Facing direction (FACING_RIGHT / FACING_LEFT).  Selects the mirror
   path in sprite_lcp_flip and biases per-state head X offsets. */
short   lcp_face;
/* Ghidra `lcp_carrying_object_flag`.  YES while the LCP is holding
   a bookshelf item / grocery / game box; enables the alternate
   arms-up carry_body_frame_table for walking states 0..24. */
short   g_lcyof;
/* Ghidra `lcp_carried_object`.  Set by sprite selection to remember
   which sprite slot the carried-item overlay came from so the
   depth-compositor can flip it in front/behind on stairs.  -1 =
   nothing carried. */
short   g_lcieo;
/* Ghidra `lcp_sprites_hidden`.  While YES the compositor forces the
   body/head sprite pointers to NULL so the character disappears
   (used by cs_mvIn while the LCP is off-screen, and by the study
   door-close cutscene). */
short   g_lssh;
/* Ghidra `debug_hide_lcp_offscreen`.  Diagnostic-only: while YES,
   sprite_update_body / _head force the sprite Y to 300 (below the
   visible area) so a developer can look at the empty room. */
short   dbg_hide;

/* ---- Dog --------------------------------------------------------------- */
/* Current screen position of the dog sprite (updated ~8Hz by
   dog_move_and_animate). */
short   dog_x;
short   dog_y;
/* Ghidra `dog_target_x/y`.  Final destination the dog is heading to
   (set by the AI when it picks a new wander destination).  When both
   are 0 the dog is considered idle at its current spot. */
short   g_dtx;
short   g_dty;
/* Ghidra `dog_waypoint_x/y`.  Intermediate stair-transition point
   between dog and target when they're on different floors.  When
   both are 0 no waypoint is active. */
short   g_dyx;
short   g_dyy;
/* Ghidra `dog_walk_anim_cycle`.  0..7 index into g_dwanf[] that
   rotates the walk-cycle frame on each tick.  Wraps at 8. */
short   g_dwanc;
/* Ghidra `dog_sprite_id`.  Current SPRITE_DOG_* id pushed into the
   hardware slot each tick (walk frame, lay-down, sit, etc.). */
short   g_dsid;
/* Ghidra `dog_on_stairs_flag`.  YES while the dog is traversing a
   flight; steers dog_move_and_animate through the stair-jump table
   instead of the flat-floor step logic. */
short   dg_stair;
/* dg_init: when non-zero sp_spud skips writing sprite slots
   0/7 (used to hide the dog while off-screen).  Ghidra BSS = 0 --
   dog is visible from frame one. */
short   dg_init;

/* ---- Hardware sprite double-buffer (SPRITE_HW_SLOTS) -------------------
   Two parallel state sets per hardware slot: `pe` = pending (what game
   logic queued for the next 8 Hz compositor tick) and `ac` = active
   (currently drawn on the visible frame).  Slot layout: 0/7 = dog
   (behind/in-front of LCP by Y depth), 3 = LCP body, 4 = LCP head,
   1..2 and 5..6 = door/object overlay slots. */
/* Sized SPRITE_HW_SLOTS_ALLOC (= HW_SLOT_NONE + 1), not SPRITE_HW_SLOTS:
   sp_upds parks HIDDEN sprites in the disabled slot HW_SLOT_NONE (9),
   and gameTick's carrying path / sp_ssco then index these arrays at [9]
   for a momentarily-hidden carried sprite.  Allocating the scratch slot
   keeps that write in-bounds; see the SPRITE_HW_SLOTS_ALLOC note in
   sprglobs.h.  Real-slot loops still bound by SPRITE_HW_SLOTS. */
/* Explicitly initialized, so it lands in DATA (all zeros) rather than
   as a .comm -- that is where LCP_STX has it. */
short   g_sepef[SPRITE_HW_SLOTS_ALLOC] = { 0 }; /* sprite_pending_flag */
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
   Populated once by sprite_register_loop from the SPRITES asset file
   at boot.  Each logical slot holds a sprite's image bitmap, mask,
   height, and width; the hardware-slot pipeline above copies from
   these when a logical sprite is pushed to the screen. */
short * g_sedim[SPRITE_SLOTS];          /* sprite_def_image[SPRITE_ID] */
short * g_sedms[SPRITE_SLOTS];          /* sprite_def_mask[SPRITE_ID] */
short   g_sedeh[SPRITE_SLOTS];          /* sprite_def_height[SPRITE_ID] */
short   g_sedew[SPRITE_SLOTS];          /* sprite_def_width[SPRITE_ID] */

/* ---- Body / carry frame tables (index = PLAYER_STATE) ------------------ */
/* body_frT (Ghidra 0x29BB2, 93 shorts):
   maps lcp_st -> body-frame index into body.lcp / body_shp.
   Values dumped via ghidra_scripts/DumpTable.java. */
short   body_frT[93] = {
         0,  1,  2,  3,  4,  1,  6,  7,     /*  0..7  */
        43,  9, 10, 11, 12, 20, 21, 22,     /*  8..15 */
        21, 13, 14, 15, 16, 17, 18, 19,     /* 16..23 */
        18, 23, 24, 25, 24, 27, 28, 29,     /* 24..31 */
        30, 31, 32, 33, 34, 35, 36, 37,     /* 32..39 */
        27, 38, 39, 40, 41, 42, 43, 44,     /* 40..47 */
        45, 46, 47, 48, 49, 50, 51, 52,     /* 48..55 */
        53, 54, 67, 68, 32, 69, 70, 71,     /* 56..63 */
        72, 73, 74, 75, 76, 77, 78, 79,     /* 64..71 */
        80, 81, 82, 83, 84, 85, 86, 87,     /* 72..79 */
        88, 89, 90, 91, 92, 93, 94, 95,     /* 80..87 */
        96, 97, 26,  5,  8                  /* 88..92 */
};
/* cy_frT (Ghidra 0x29C6C, 25 shorts):
   alternate arms-up frames used while carrying an object in
   walking states 0..24. */
short   cy_frT[25]      = {
        55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
        13, 14, 15, 16, 17, 18, 19, 18
};

/* Per-PLAYER_STATE horizontal offset for the head anchor.  Ghidra
   0x29C9E, actual size = 93 shorts (distance to next symbol
   hd_hgt @ 0x29D58 is 186 bytes).  The previous
   [109] port declaration overflowed into adjacent tables, giving
   wrong offsets for lcp_st 93..108. */
short   hd_xoff[93] = {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  6,
         6,  0, -1,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0
};

/* Per-PLAYER_STATE head Y contribution (subtracted from body top).
   Ghidra 0x29D58, 93 shorts (distance to hd_dang
   @ 0x29E12). */
short   hd_hgt[93] = {
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 18, 18, 18, 18, 17, 17,
        17, 21, 21, 18, 18, 18, 18, 18,
        18, 18, 17, 21, 21, 21, 21, 21,
        21, 21, 21, 20, 21, 21, 21, 21,
        21, 21, 21, 18, 21, 21, 21, 21,
         5,  5,  5,  5,  5, 19, 19, 21,
        21, 21, 21, 21, 21, 21, 21, 20,
        21, 21, 20, 20, 21, 21, 21, 21,
        20, 21, 20, 21, 21
};

/* Neutral head-facing angle per PLAYER_STATE (used by head_animate to
   pick the "resting" horizontal direction the head drifts toward).
   Ghidra 0x29E12, 93 shorts (distance to room_position_x_table @
   0x29ECC).  The previous [109] port declaration read into
   room_position_x_table for lcp_st 93..108, producing wrong
   head_sprite_frame values that showed up as broken head phases
   whenever the character entered a state past 92. */
short   hd_dang[93] = {
         2,  2,  2,  2,  2,  2,  2,  2,
         2,  2,  2,  2,  2,  4,  4,  4,
         4,  2,  2,  2,  2,  0,  0,  0,
         0,  3,  4,  4,  4,  4,  4,  4,
         4,  4,  0,  0,  0,  0,  4,  4,
         4,  4,  4,  0,  0,  0,  2,  2,
         2,  2,  2,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  4,  4,  4,
         2,  2,  2,  2,  2,  1,  4,  0,
         0,  0,  0,  4,  4,  4,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  4,  3,  1
};

/* LCP_STX indexes the body and shape buffers as ARRAYS (muls.w
   stride plus an immediate base, no ext.l); the other revision goes through
   pointer variables that al_locs wires to the buffers.  See the
   sp_updb note in CLAUDE.md. */
unsigned char   body_ptr[120][168];     /* LCP_BODY_FRAME_SIZE */
unsigned char   body_shp[98][84];       /* LCP_BODY_SHAPE_SIZE */
/* body_shp buffer (Ghidra 0x3D23C, 98 * 84 = 8232 bytes):
   destination for sprite_lcp_build_all_body's 30-bit dilation of the
   raw 168-byte body frames.  84 bytes = 21 rows * 2 words per row.
   BSS-resident so it survives to game end without heap traffic. */
short   g_lsimg[LCP_BODY_DEST_WORDS];    /* sp_lcpf dest: image plane pair */
short   g_lsmas[LCP_BODY_DEST_WORDS];    /* sp_lcpf dest: mask plane pair */
/* body_yof (Ghidra 0x29F8C, 109 shorts):
   Y anchor offset per lcp_st.  Verified against Ghidra dump. */
short   body_yof[109] = {
        -2, -2, -2, -1, -2, -2, -2, -1,     /*   0..7  */
        -2,  0,  0,  0,  0,  0,  0,  0,     /*   8..15 */
         0,  0,  0,  0,  0,  0,  0,  0,     /*  16..23 */
         0, -2, -2, -2, -2, -2,  0,  0,     /*  24..31 */
         0, -2, -2, -2, -2, -2, -2, -2,     /*  32..39 */
        -2, -2,  0, -6, -6, -6, -2, -6,     /*  40..47 */
        -6,  2,  1,  7, -7, -5, -5, -5,     /*  48..55 */
        -5, -5, -4, -1,  0, -2, -2, -2,     /*  56..63 */
        11, 11, 11, 11, 11, -1, -1, -7,     /*  64..71 */
        -7, -4, -7, -2, -2, -4, -2, -1,     /*  72..79 */
        -2, -2,  0,  0, -2, -2, -2, -2,     /*  80..87 */
        -3, -2, -3, -2, -2,  1,  2,  6,     /*  88..95 */
        11, 17, 20, 22, 26, 30, 33, 35,     /*  96..103 */
        46,  1, 11, 26, 35                  /* 104..108 */
};
/* PTR_ARRAY_0005a156/0x54016 are declared as g_sedim/g_sedms above.
   sp_reglp populates them; sp_sprs/sp_ssco/sp_ss02/sp_spud all read
   from the same arrays.  There is no separate "dog only" table -- the
   original binary has one 60-entry sprite pointer table shared by
   every registered sprite. */
/* g_dfimb / g_dfmab (Ghidra dog_flip_image_buffer / dog_flip_mask_buffer,
   240 bytes = 15 rows * 2 word-width * 4 planes * 2 bytes/word).
   Written by sp_flih when the dog needs a mirrored frame.
   The original port sized these at 64 shorts (128 bytes), which is
   half the amount sp_flih actually writes -- the extra 112 bytes
   spilled into adjacent BSS globals. */
short   g_dfimb[120];
short   g_dfmab[120];
/* Ghidra 0x2A066, actual size = 6 shorts (distance to
   stair_ty @ 0x2A072 is 12 bytes).  The last 2 entries
   my earlier port added (124, 137) were `stair_ty` and
   `stair_by` -- adjacent globals, not part of the
   waypoint table. */
short   stair_wp[6]    = { 170, 185, 133, 124, 182, 72 };
/* Middle-floor staircase-2 landing coordinates (top-of-flight X and Y).
   The middle-floor branch of lcp_flwp uses these to
   route through the between-floor landing instead of the raw
   stair_wp entries.  Values dumped from Ghidra data. */
/* Ghidra 0x2A072 / 0x2A074. */
short   stair_ty           = 124;
short   stair_by        = 137;

/* Ghidra `sub_animation_frame_counter`.  Increments on every 8 Hz
   render tick; used by cl_redrH etc. to drive slow overlay
   animations (pendulum, phone-hook rocking, dog tail wag). */
short   subAniC;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   g_hsbuf[LCP_BODY_DEST_WORDS];        /* sp_lcpf dest: head image */
short   g_hsmas[LCP_BODY_DEST_WORDS];        /* sp_lcpf dest: head mask */
/* Ghidra `head_sprite_mirror_flag`.  Set by head_animate to select
   the horizontal-flip path in sprite_lcp_flip when the head faces
   the opposite direction from the body. */
short   g_hsmif;
/* The loaded PEx.LCP frame table and the dilated head silhouettes are
   ARRAYS, not pointers: LCP_STX indexes them with an immediate base
   (muls #168 / muls #84 then add.l #base), exactly like body_ptr and
   body_shp. */
unsigned char   pex_ptr[66][168];       /* LCP_BODY_FRAME_SIZE */
unsigned char   hd_shp[66][84];         /* LCP_BODY_SHAPE_SIZE */
short   flr_cy[3]        = { 198, 135, 71 };
/* Ghidra `lcp_on_stairs_flag` (short, YES/NO).  YES while
   lcp_pathfind_one_step is inside a stair-traversal path; drives the
   stair-specific sprite-state sequence 9..24 and the wood-stairs SFX
   selection. */
short   lcp_stR              = 0;

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   flr_by[0] = top floor, [1] = middle floor, [2] = bottom. */
/* Ghidra-verified: floor 1 (bottom) .. floor 3 (top).  Dumped from
   0x2A07E (bottom), 0x2A076 (center), 0x2A066 (waypoints). */
short   flr_by[3]        = { 202, 140, 77 };

/* ---- Dog sprite pointers / buffers ------------------------------------- */
/* g_dwanf (Ghidra dog_walk_anim_frames @ 0x2A0E8): 8 sprite ids the
   walk cycle rotates through in dg_mvAni. */
short   g_dwanf[8] = {
        SPRITE_DOG_WLK_R1, SPRITE_DOG_WLK_R2,
        SPRITE_DOG_WLK_R3, SPRITE_DOG_WLK_R4,
        SPRITE_DOG_WLK_R5, SPRITE_DOG_WLK_R7,
        SPRITE_DOG_WLK_R8, SPRITE_DOG_WLK_R9
};
/* Ghidra sprite_layer_flags @ 0x2b6ee: entries 0,1 = SPRITE_IN_FRONT (1),
   rest = SPRITE_HIDDEN (0).  These are the two dog slot flags (slots
   0 and 7 in the hardware layout, per sp_upds). */
short   g_selaf[SPRITE_SLOTS] = { 1, 1 };
/* Ghidra sprite_slot_map @ 0x2b766: which hardware slot each logical
   sprite is currently mapped to.  Entries 0..1 pin the LCP body/head
   to their dedicated slots; the rest default to HW_SLOT_NONE (=9,
   the compositor's off-screen sentinel) and get assigned dynamically
   by sprite_update_slots when the sprite is queued. */
short   g_seslm[SPRITE_SLOTS] = {
        /* 0..9   */ HW_SLOT_LCP_BODY, HW_SLOT_LCP_HEAD,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
        /* 10..19 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 20..29 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 30..39 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 40..49 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 50..59 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE
};

/* 15-entry delta table (Ghidra 0x2BA06). */
short   hd_mvd[15]   = {
         1,  1,  1, 99, -1, -1, -1,  0,
         1,  1,  1, 99, -1, -1, -1
};

/* ---- Walk-pathfinding state ------------------------------------------ */
/* Intermediate waypoint the LCP walks through to reach `g_wtx`/`g_wty`
   on a different floor; set by lcp_calc_floor_waypoint, zeroed on
   arrival.  All-zero = no waypoint active. */
short   g_wyx;
short   g_wyy;

/* Per-tilt frame-index offset (Ghidra 0x2BA24, 3 shorts -- distance
   to head_anim_delay_countdown @ 0x2BA2A is 6 bytes). */
short   hd_tilt[3]       = { 7, 12, 17 };
/* Ghidra `footstep_trigger_flag`.  Latched YES on walk-cycle frames
   3 and 7 to schedule the next footstep SFX; consumed and cleared by
   lcp_play_footstep_sound. */
BOOL16  fs_trg;
/* Ghidra `head_anim_state_last`.  Snapshot of the head animation
   state from the previous tick; head_animate diffs against this to
   detect direction changes and pick the transition frame. */
short   g_hastl;
/* Ghidra head_anim_delay_countdown @ 0x2ba2a = 1. */
short   g_hadec                         = 1;

/* Per-happiness-level head frame base index (into pex_ptr). */
/* Ghidra 0x2BA2C. */
short   mood_hfo[3]  = { 44, 0, 22 };
