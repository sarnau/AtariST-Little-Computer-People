/*
 * sprite_globals.c -- storage for sprite pipeline, LCP animation, and
 *                     dog state.
 *
 * Kept separate from globals.c because the sprite arrays are large and
 * the two units evolve at different cadences.  All variables here are
 * declared extern in globals.h.
 */

#include "types.h"

/* ---- LCP animation ----------------------------------------------------- */
short   lcp_state                       = 0;
short   lcp_facing_direction            = 0;    /* FACING_RIGHT */
short   lcp_carrying_object_flag        = 0;
short   lcp_carried_object              = -1;
short   lcp_sprites_hidden              = 0;
short   debug_hide_lcp_offscreen        = 0;

/* ---- Dog --------------------------------------------------------------- */
short   dog_x                           = 0;
short   dog_y                           = 0;
short   dog_target_x                    = 0;
short   dog_target_y                    = 0;
short   dog_waypoint_x                  = 0;
short   dog_waypoint_y                  = 0;
short   dog_walk_anim_cycle             = 0;
short   dog_sprite_id                   = 0;
short   dog_on_stairs_flag              = 0;
short   dog_initialized                 = 1;    /* until placed */

/* ---- Hardware sprite double-buffer (8 slots) --------------------------- */
short   sprite_pending_flag[8];
short * sprite_pending_image[8];
short * sprite_pending_mask[8];
short   sprite_pending_x[8];
short   sprite_pending_y[8];
short   sprite_pending_height[8];
short   sprite_pending_width[8];
short * sprite_active_image[8];
short * sprite_active_mask[8];
short   sprite_active_x[8];
short   sprite_active_y[8];
short   sprite_active_height[8];
short   sprite_active_width[8];

/* ---- Sprite definitions (60 logical slots) ----------------------------- */
short * sprite_def_image[60];
short * sprite_def_mask[60];
short   sprite_def_height[60];
short   sprite_def_width[60];
short   sprite_layer_flags[60];
short   sprite_slot_map[60];

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
short   lcp_sprite_img[168];    /* 21 rows * 4 words * 2 (image+mask) */
short   lcp_sprite_mask[168];

/* ---- Dog sprite pointers / buffers ------------------------------------- */
short   dog_walk_anim_frames[8];        /* SPRITE_DOG_WALK_1..8 */
short * dog_sprite_pointers[60];
short * dog_mask_pointers[60];
short   dog_flip_image_buffer[64];
short   dog_flip_mask_buffer[64];

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   floor_bottom_y_coords[0] = top floor, [1] = middle floor, [2] = bottom. */
short   floor_bottom_y_coords[3]        = { 77, 140, 202 };
short   floor_center_y_coords[3]        = { 62, 125, 187 };
short   staircase_waypoint_coords[6]    = { 0, 100, 0, 161, 0, 200 };

short   sub_animation_frame_counter     = 0;

/* ---- Head sprite double-buffer + source pointers ---------------------- */
short   head_sprite_buffer[168];        /* 21 rows * 4 words * 2 (image) */
short   head_sprite_mask[168];
short   head_sprite_mirror_flag         = 0;
short * pex_lcp_file;                   /* filled by asset loader */
short * head_shape_data;
short   head_anim_delay_countdown       = 0;

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
short   walk_waypoint_x                 = 0;
short   walk_waypoint_y                 = 0;
short   lcp_on_stairs_flag              = 0;
BOOL16  footstep_trigger_flag           = 0;
short   head_anim_state_last            = 0;
/* Middle-floor staircase-2 landing coordinates (top-of-flight X and Y).
   The middle-floor branch of lcp_calc_floor_waypoint uses these to
   route through the between-floor landing instead of the raw
   staircase_waypoint_coords entries.  Values dumped from Ghidra data. */
short   stair_top_y_threshold           = 130;
short   stair_bottom_y_threshold        = 140;
