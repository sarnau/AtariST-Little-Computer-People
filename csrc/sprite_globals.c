/*
 * sprite_globals.c -- storage for sprite pipeline, LCP animation, and
 *                     dog state.
 *
 * Kept separate from globals.c because the sprite arrays are large and
 * the two units evolve at different cadences.  All variables here are
 * declared extern in globals.h.
 */

#include "types.h"
#include "structs.h"

/* ---- LCP animation ----------------------------------------------------- */
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
short   dog_initialized                 = 1;    /* until placed */

/* ---- Hardware sprite double-buffer (8 slots) --------------------------- */
short   g_sepef[8];
short * sprite_pending_image[8];
short * sprite_pending_mask[8];
short   g_sepex[8];
short   g_sepey[8];
short   g_sepeh[8];
short   g_sepew[8];
short * sprite_active_image[8];
short * sprite_active_mask[8];
short   g_seacx[8];
short   g_seacy[8];
short   g_seach[8];
short   g_seacw[8];

/* ---- Sprite definitions (60 logical slots) ----------------------------- */
short * sprite_def_image[60];
short * sprite_def_mask[60];
short   g_sedeh[60];
short   g_sedew[60];
short   g_selaf[60];
short   g_seslm[60];

/* ---- Body / carry frame tables (index = PLAYER_STATE) ------------------ */
/* Verified against Ghidra body_sprite_frame_table[93] at 0x29bd0 and
   carry_body_frame_table[25] at 0x29c6c.  Extended with zeros beyond the
   Ghidra length so out-of-range states are safe. */
short   body_sprite_frame_table[100];
short   carry_body_frame_table[25]      = {
        55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
        13, 14, 15, 16, 17, 18, 19, 18
};
short   body_y_offset_per_state[100];

short * body_lcp_file;
short * body_shape_data;
short   g_lsimg[168];    /* 21 rows * 4 words * 2 (image+mask) */
short   g_lsmas[168];

/* ---- Dog sprite pointers / buffers ------------------------------------- */
short   g_dwanf[8];        /* SPRITE_DOG_WALK_1..8 */
short * dog_sprite_pointers[60];
short * dog_mask_pointers[60];
short   g_dfimb[64];
short   g_dfmab[64];

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   floor_bottom_y_coords[0] = top floor, [1] = middle floor, [2] = bottom. */
short   floor_bottom_y_coords[3]        = { 77, 140, 202 };
short   floor_center_y_coords[3]        = { 62, 125, 187 };
short   staircase_waypoint_coords[6]    = { 0, 100, 0, 161, 0, 200 };

short   sub_animation_frame_counter     = 0;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   g_hsbuf[168];        /* 21 rows * 4 words * 2 (image) */
short   g_hsmas[168];
short   g_hsmif         = 0;
short * pex_lcp_file;                   /* filled by asset loader */
short * head_shape_data;
short   g_hadec       = 0;

/* Per-happiness-level head frame base index (into pex_lcp_file). */
short   happiness_head_frame_offset[3]  = { 0, 66, 132 };

/* Per-PLAYER_STATE horizontal offset for the head anchor. */
short   head_x_offset_per_state[100];

/* Per-PLAYER_STATE head Y contribution (subtracted from the body top). */
short   head_height_per_state[100];

/* Neutral head-facing angle per PLAYER_STATE (used by head_animate to
   pick the "resting" horizontal direction the head drifts toward). */
short   head_default_angle_per_state[100];

/* 15-entry delta table indexed by (target_frame - current_frame + 7),
   returning the signed step count between frames.  Value 99 = "no
   direct path, use default". */
short   head_movement_delta_table[15]   = {
        99, 99, -1, -1, -1, -1, -1, 0, 1, 1, 1, 1, 1, 99, 99
};

/* Per-tilt frame-index offset into the 8-frame head sprite row.
   3 tilts (up / centre / down) * 8 frames each. */
short   head_tilt_frame_offset[3]       = { 0, 8, 16 };

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
short   stair_top_y_threshold           = 130;
short   stair_bottom_y_threshold        = 140;
