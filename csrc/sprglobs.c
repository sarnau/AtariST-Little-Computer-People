/*
 * sprglobs.c -- storage for sprite pipeline, LCP animation, and
 *                     dog state.
 *
 * Kept separate from globals.c because the sprite arrays are large and
 * the two units evolve at different cadences.  All variables here are
 * declared extern in globals.h.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"

/* ---- LCP animation ----------------------------------------------------- */
/* Ghidra BSS = 0.  cutscene_new_lcp_move_in_stub sets lcp_state to
   STATE_STAND_SIDE_VIEW (34) before endless_game_loop starts. */
short   lcp_state                       = 0;
short   lcp_facing_direction            = 0;    /* FACING_RIGHT */
short   g_lcyof        = 0;
short   g_lcieo              = -1;
short   g_lssh              = 0;
short   debug_hide_lcp_offscreen        = 0;

/* ---- Dog --------------------------------------------------------------- */
short   dog_x                           = 0;
short   dog_y                           = 0;
short   g_dtx                    = 0;
short   g_dty                    = 0;
short   g_dyx                  = 0;
short   g_dyy                  = 0;
short   g_dwanc             = 0;
short   g_dsid                   = 0;
short   dog_on_stairs_flag              = 0;
/* dog_initialized: when non-zero sp_spud skips writing sprite slots
   0/7 (used to hide the dog while off-screen).  Ghidra BSS = 0 --
   dog is visible from frame one. */
short   dog_initialized                 = 0;

/* ---- Hardware sprite double-buffer (8 slots) --------------------------- */
short   g_sepef[8];
short * g_sepim[8];
short * g_sepms[8];
short   g_sepex[8];
short   g_sepey[8];
short   g_sepeh[8];
short   g_sepew[8];
short * g_seaim[8];
short * g_seams[8];
short   g_seacx[8];
short   g_seacy[8];
short   g_seach[8];
short   g_seacw[8];

/* ---- Sprite definitions (60 logical slots) ----------------------------- */
short * g_sedim[60];
short * g_sedms[60];
short   g_sedeh[60];
short   g_sedew[60];
short   g_selaf[60];
short   g_seslm[60];

/* ---- Body / carry frame tables (index = PLAYER_STATE) ------------------ */
/* body_sprite_frame_table (Ghidra 0x29BB2, 93 shorts):
   maps lcp_state -> body-frame index into body.lcp / body_shape_data.
   Values dumped via ghidra_scripts/DumpTable.java. */
short   body_sprite_frame_table[93] = {
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
/* carry_body_frame_table (Ghidra 0x29C6C, 25 shorts):
   alternate arms-up frames used while carrying an object in
   walking states 0..24. */
short   carry_body_frame_table[25]      = {
        55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
        13, 14, 15, 16, 17, 18, 19, 18
};
/* (Removed dead head_sprite_frame_table[66] -- it was a mis-transcribed
   duplicate of head_x_offset_per_state and not referenced anywhere.) */
/* body_y_offset_per_state (Ghidra 0x29F8C, 109 shorts):
   Y anchor offset per lcp_state.  Verified against Ghidra dump. */
short   body_y_offset_per_state[109] = {
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

short * body_lcp_file;
short * body_shape_data;
/* body_shape_data buffer (Ghidra 0x3D23C, 98 * 84 = 8232 bytes):
   destination for sprite_lcp_build_all_body's 30-bit dilation of the
   raw 168-byte body frames.  84 bytes = 21 rows * 2 words per row.
   BSS-resident so it survives to game end without heap traffic. */
short   bshdbuf[98 * 42];    /* 42 shorts/frame = 84 bytes */
short   g_lsimg[168];    /* 21 rows * 4 words * 2 (image+mask) */
short   g_lsmas[168];

/* ---- Dog sprite pointers / buffers ------------------------------------- */
/* g_dwanf (Ghidra dog_walk_anim_frames @ 0x2A0E8): 8 sprite ids the
   walk cycle rotates through in dog_move_and_animate. */
short   g_dwanf[8] = { 34, 35, 36, 37, 38, 39, 40, 41 };
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

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   floor_bottom_y_coords[0] = top floor, [1] = middle floor, [2] = bottom. */
/* Ghidra-verified: floor 1 (bottom) .. floor 3 (top).  Dumped from
   0x2A07E (bottom), 0x2A076 (center), 0x2A066 (waypoints). */
short   floor_bottom_y_coords[3]        = { 202, 140, 77 };
short   floor_center_y_coords[3]        = { 198, 135, 71 };
/* Ghidra 0x2A066, actual size = 6 shorts (distance to
   stair_top_y_threshold @ 0x2A072 is 12 bytes).  The last 2 entries
   my earlier port added (124, 137) were `stair_top_y_threshold` and
   `stair_bottom_y_threshold` -- adjacent globals, not part of the
   waypoint table. */
short   staircase_waypoint_coords[6]    = { 170, 185, 133, 124, 182, 72 };

short   sub_animation_frame_counter     = 0;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   g_hsbuf[168];        /* 21 rows * 4 words * 2 (image) */
short   g_hsmas[168];
short   g_hsmif         = 0;
short * pex_lcp_file;                   /* filled by asset loader */
short * head_shape_data;
/* head_shape_data buffer (Ghidra 0x4B9D2, 66 * 84 = 5544 bytes):
   destination for sprite_lcp_build_all_head's dilation of the raw
   168-byte head frames from the PEx.LCP file. */
short   hshdbuf[66 * 42];    /* 42 shorts/frame = 84 bytes */
short   g_hadec       = 0;

/* Per-happiness-level head frame base index (into pex_lcp_file). */
/* Ghidra 0x2BA2C. */
short   happiness_head_frame_offset[3]  = { 44, 0, 22 };

/* Per-PLAYER_STATE horizontal offset for the head anchor.  Ghidra
   0x29C9E, actual size = 93 shorts (distance to next symbol
   head_height_per_state @ 0x29D58 is 186 bytes).  The previous
   [109] port declaration overflowed into adjacent tables, giving
   wrong offsets for lcp_state 93..108. */
short   head_x_offset_per_state[93] = {
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
   Ghidra 0x29D58, 93 shorts (distance to head_default_angle_per_state
   @ 0x29E12). */
short   head_height_per_state[93] = {
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
   room_position_x_table for lcp_state 93..108, producing wrong
   head_sprite_frame values that showed up as broken head phases
   whenever the character entered a state past 92. */
short   head_default_angle_per_state[93] = {
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

/* 15-entry delta table (Ghidra 0x2BA06). */
short   head_movement_delta_table[15]   = {
         1,  1,  1, 99, -1, -1, -1,  0,
         1,  1,  1, 99, -1, -1, -1
};

/* Per-tilt frame-index offset (Ghidra 0x2BA24, 3 shorts -- distance
   to head_anim_delay_countdown @ 0x2BA2A is 6 bytes). */
short   head_tilt_frame_offset[3]       = { 7, 12, 17 };

/* ---- Walk-pathfinding state ------------------------------------------ */
short   g_wyx                 = 0;
short   g_wyy                 = 0;
short   lcp_on_stairs_flag              = 0;
BOOL16  footstep_trigger_flag           = 0;
short   g_hastl            = 0;
/* Middle-floor staircase-2 landing coordinates (top-of-flight X and Y).
   The middle-floor branch of lcp_calc_floor_waypoint uses these to
   route through the between-floor landing instead of the raw
   staircase_waypoint_coords entries.  Values dumped from Ghidra data. */
/* Ghidra 0x2A072 / 0x2A074. */
short   stair_top_y_threshold           = 124;
short   stair_bottom_y_threshold        = 137;
