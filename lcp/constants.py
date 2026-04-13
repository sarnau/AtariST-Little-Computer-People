"""
ROM data tables for Little Computer People (Atari ST).
All values extracted from the DATA segment (0x0296DC–0x02C6BF) via Ghidra.

These are read-only at runtime — they mirror the original binary tables.
"""

from .enums import ACTION_ID, HOUSE_POS, SPRITE_ID, SOUND_EFFECT_ID


# ---------------------------------------------------------------------------
# House position coordinate tables
# addr: room_position_x_table, room_position_height_table
# house_get_position_xy(pos) → (x, y) pixel coordinates
#
# Formula (from decompiled house_get_position_xy):
#   x = room_position_x_table[pos] << 1       (stored as half-pixels)
#   y = floor_baseline_y[floor] - room_position_height_table[pos + 1]
#
# Floor baselines (Y pixels from top of 320×200 screen):
#   Floor 3 (top):    Y = 77   (positions  0–15)
#   Floor 2 (middle): Y = 140  (positions 16–31)
#   Floor 1 (bottom): Y = 202  (positions 32–47)
# ---------------------------------------------------------------------------

FLOOR_BASELINE_Y = [77, 140, 202]   # index 0=top, 1=middle, 2=bottom

# X coordinates (half-pixels — double to get screen X)
# 48 entries, one per HOUSE_POS
# Extracted from DATA segment at 0x19ecc
ROOM_POSITION_X_TABLE = [
    # Floor 3 — top (positions 0–15)
      22,   36,   49,   55,   60,   56,   73,   96,
     106,  118,  113,  110,  131,   47,  133,  146,
    # Floor 2 — middle (positions 16–31)
      16,   40,   27,   31,   45,   55,   84,  100,
     111,  100,  109,  124,  134,  135,  144,   67,
    # Floor 1 — bottom (positions 32–47)
       8,    8,   12,   19,   40,   25,   54,   49,
      67,   70,  106,  110,  123,  132,  147,  140,
]

# Height offsets from floor baseline (Y subtracted from baseline)
# 49 entries (pos 0 uses index 1, so table has a leading dummy at index 0)
# Extracted from DATA segment at 0x19f2c
ROOM_POSITION_HEIGHT_TABLE = [
    # dummy[0]
    9,
    # Floor 3 — top (indices 1–16)
      14,    9,   10,   11,   14,   12,   13,   12,
      12,   12,    6,   15,   10,   14,    3,    3,
    # Floor 2 — middle (indices 17–32)
       3,    8,   15,   13,   13,   12,   13,   14,
      12,    8,   14,   13,   14,   13,    5,    8,
    # Floor 1 — bottom (indices 33–48)
       3,   10,   13,   13,   14,   10,   14,   14,
      12,   13,    7,   14,   12,   13,    2,   -2,
]


def house_get_position_xy(pos: int) -> tuple[int, int]:
    """
    Convert a HOUSE_POS index (0–47) to screen pixel coordinates.
    addr: house_get_position_xy at 0x01635E
    """
    x = ROOM_POSITION_X_TABLE[pos] << 1
    floor_index = pos >> 4       # floor = position // 16
    y_base = FLOOR_BASELINE_Y[floor_index] if floor_index < 3 else FLOOR_BASELINE_Y[2]
    y = y_base - ROOM_POSITION_HEIGHT_TABLE[pos + 1]
    return x, y


# ---------------------------------------------------------------------------
# Staircase waypoint coordinates
# addr: staircase_waypoint_coords[] used in lcp_pathfind_one_step()
# Waypoints guide the LCP through the stairwell between floors
# X=160 is the staircase horizontal centre
# ---------------------------------------------------------------------------
STAIR_X = 160   # staircase horizontal centre pixel

# Staircase waypoint coordinates — 3 (X, Y) pairs, one per floor
# Extracted from DATA segment at 0x1a066
# Indexed as staircase_waypoint_coords[2*(floor_num-1)] = X,
#            staircase_waypoint_coords[2*(floor_num-1)+1] = Y
# addr: staircase_waypoint_coords[6]
STAIRCASE_WAYPOINT_COORDS = [170, 185, 133, 124, 182, 72]
#                             floor1_x, floor1_y, floor2_x, floor2_y, floor3_x, floor3_y


# ---------------------------------------------------------------------------
# Action tables — lists of ACTION_IDs used by the random activity selector
# addr: action_table_active, action_table_moderate, action_table_relaxed
# Selected by activity_schedule_table based on time-of-day and activity_level
# ---------------------------------------------------------------------------
ACTION_TABLE_ACTIVE = [
    ACTION_ID.ACTION_PLAY_COMPUTER,
    ACTION_ID.ACTION_WRITE_LETTER,
    ACTION_ID.ACTION_PLAY_PIANO,
    ACTION_ID.ACTION_DANCE,
    ACTION_ID.ACTION_SIT_AND_EXERCISE,
    ACTION_ID.ACTION_LISTEN_SONG,
    ACTION_ID.ACTION_PLAY_A_GAME,
    ACTION_ID.ACTION_BRUSH_TEETH,
    ACTION_ID.ACTION_PLAY_WITH_RECORD,
    ACTION_ID.ACTION_LIGHT_FIREPLACE,
    ACTION_ID.ACTION_TAKE_SHOWER,
    ACTION_ID.ACTION_WRITE_LETTER,
    ACTION_ID.ACTION_PLAY_COMPUTER,
    ACTION_ID.ACTION_DANCE,
    ACTION_ID.ACTION_SIT_AND_EXERCISE,
    ACTION_ID.ACTION_PLAY_PIANO,
]

ACTION_TABLE_MODERATE = [
    ACTION_ID.ACTION_READ_NEWSPAPER,
    ACTION_ID.ACTION_PLAY_A_GAME,
    ACTION_ID.ACTION_LISTEN_SONG,
    ACTION_ID.ACTION_BRUSH_TEETH,
    ACTION_ID.ACTION_CLEAN_UP,
    ACTION_ID.ACTION_TIDY_HOUSE,
    ACTION_ID.ACTION_DRINK,
    ACTION_ID.ACTION_NOD_HEAD,
    ACTION_ID.ACTION_PEEK_AROUND,
    ACTION_ID.ACTION_PLAY_COMPUTER,
    ACTION_ID.ACTION_GET_SNACK_FROM_FRIDGE,
    ACTION_ID.ACTION_TAKE_SHOWER,
    ACTION_ID.ACTION_READ_NEWSPAPER,
    ACTION_ID.ACTION_PLAY_A_GAME,
    ACTION_ID.ACTION_LISTEN_SONG,
    ACTION_ID.ACTION_PLAY_PIANO,
]

ACTION_TABLE_RELAXED = [
    ACTION_ID.ACTION_SIT_ON_COUCH_WITH_DOG,
    ACTION_ID.ACTION_YAWN_AND_STRETCH,
    ACTION_ID.ACTION_WANDER_IDLY,
    ACTION_ID.ACTION_PACE_NERVOUSLY,
    ACTION_ID.ACTION_NOD_HEAD,
    ACTION_ID.ACTION_PEEK_AROUND,
    ACTION_ID.ACTION_SLEEP,
    ACTION_ID.ACTION_DRINK,
    ACTION_ID.ACTION_TOGGLE_TV,
    ACTION_ID.ACTION_READ_NEWSPAPER,
    ACTION_ID.ACTION_SIT_ON_COUCH_WITH_DOG,
    ACTION_ID.ACTION_YAWN_AND_STRETCH,
    ACTION_ID.ACTION_WANDER_IDLY,
    ACTION_ID.ACTION_PACE_NERVOUSLY,
    ACTION_ID.ACTION_NOD_HEAD,
    ACTION_ID.ACTION_SLEEP,
]

ACTION_TABLES = [ACTION_TABLE_ACTIVE, ACTION_TABLE_MODERATE, ACTION_TABLE_RELAXED]

# activity_schedule_table[3][8]
# Selects which ACTION_TABLE to use (0=active, 1=moderate, 2=relaxed)
# Index: [time_period 0–2][activity_level 0–7]
# time_period = (hours_since_wake // 2) % 3, roughly
# Saturday forces moderate (1), Sunday forces relaxed (2)
# TODO: verify exact values from Ghidra
ACTIVITY_SCHEDULE_TABLE = [
    # Time period 0 (morning)
    [2, 2, 2, 2, 1, 1, 0, 0],
    # Time period 1 (midday)
    [0, 0, 0, 0, 0, 1, 1, 2],
    # Time period 2 (evening)
    [2, 2, 2, 2, 1, 1, 1, 0],
]


# ---------------------------------------------------------------------------
# Footstep sound selection by surface type
# addr: lcp_pathfind_one_step() footstep trigger logic
# Surface determined by X position (staircase vs floor) and floor number
# ---------------------------------------------------------------------------
FOOTSTEP_BY_SURFACE = {
    'stairs':  SOUND_EFFECT_ID.SFX_FOOTSTEP_STAIRS,
    'carpet':  SOUND_EFFECT_ID.SFX_FOOTSTEP_CARPET,
    'wood':    SOUND_EFFECT_ID.SFX_FOOTSTEP_WOOD,
}

# Walk-cycle frames that trigger a footstep sound
FOOTSTEP_TRIGGER_FRAMES = {3, 7}   # STATE_WALK_FRAME_3, STATE_WALK_FRAME_7
STAIR_FOOTSTEP_FRAMES   = {12}     # STATE_STAIR_UP_3


# ---------------------------------------------------------------------------
# Static object positions — initialised in main()
# addr: main() object coordinate assignments
# ---------------------------------------------------------------------------
STATIC_OBJECT_POSITIONS = {
    'cabinet':      (46,  140),
    'front_door':   (294, 151),
    'dresser':      (97,  115),
    'closet':       (75,   87),
    'study_door':   (178,  23),
    'toilet_door':  (187,  87),
    'filing_cabinet': (258, 47),
    'dog_bowl':     (8,   190),
    'fridge':       (24,  153),
}


# ---------------------------------------------------------------------------
# Dog wandering destinations (9 positions)
# addr: dog_destination_position_table[9] at DATA+0x2202 (Ghidra 0x2b8de)
# HOUSE_POS indices used by dog wander AI. Indices 0-2 are top-floor only
# (used when dog_visible==0), indices 3-8 span all floors.
# ---------------------------------------------------------------------------
DOG_DESTINATION_POSITION_TABLE = [
    HOUSE_POS.POS_TOP_0,           # 0
    HOUSE_POS.POS_TOP_LOG_AREA,    # 5
    HOUSE_POS.POS_TOP_11,          # 11
    HOUSE_POS.POS_MID_CLOSET,      # 19
    HOUSE_POS.POS_MID_COMPUTER,    # 29
    HOUSE_POS.POS_BTM_0,           # 32 — triggers dog_near_food_bowl
    HOUSE_POS.POS_BTM_SINK,        # 33
    HOUSE_POS.POS_BTM_41,          # 41
    HOUSE_POS.POS_BTM_47,          # 47
]

# addr: dog_dest_y_offset_table[9] at DATA+0x2216 (Ghidra 0x2b8f2)
DOG_DEST_Y_OFFSET_TABLE = [3, 9, 2, 10, 6, 0, 0, 11, 3]

# addr: dog_dest_x_offset_table[9] at DATA+0x222a (Ghidra 0x2b906)
DOG_DEST_X_OFFSET_TABLE = [0, 0, 0, 0, 10, 0, 0, 0, 0]

# Stair thresholds used by dog_calc_walk_path
# addr: stair_top_y_threshold at Ghidra 0x2a072, stair_bottom_y_threshold at Ghidra 0x2a074
DOG_STAIR_TOP_Y_THRESHOLD = 124
DOG_STAIR_BOTTOM_Y_THRESHOLD = 137

# Dog walk animation cycle: 8 sprite IDs
DOG_WALK_ANIM_FRAMES = [
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_1,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_2,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_3,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_4,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_5,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_6,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_7,
    SPRITE_ID.SPRITE_DOG_WALK_RIGHT_8,
]

# Dog eating animation cycle: 3 sprite IDs
DOG_EATING_ANIM_FRAMES = [
    SPRITE_ID.SPRITE_DOG_EATING_1,
    SPRITE_ID.SPRITE_DOG_EATING_2,
    SPRITE_ID.SPRITE_DOG_EATING_3,
]


# ---------------------------------------------------------------------------
# Sprite data index remapping table
# addr: spritedata_index_table[50] in readFiles.py / spritedata_load()
# Maps sequential file order to logical SPRITE_ID slots
# ---------------------------------------------------------------------------
SPRITEDATA_INDEX_TABLE = [
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
    0x2C, 0x09, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x03,
    0x04, 0x32, 0x07, 0x06, 0x33, 0x34, 0x35, 0x36,
    0x08, 0x37,
]


# ---------------------------------------------------------------------------
# Timing constants
# addr: game_tick_and_animate(), game_simulate_one_second()
# ---------------------------------------------------------------------------
GAME_TICKS_PER_SECOND  = 8     # animation frames per game-second (~8 Hz)
FRAMES_PER_GAME_SECOND = 8

# Sickness timers (game-minutes)
SICKNESS_WORSEN_MINUTES   = 60   # minutes between worsening steps
SICKNESS_RECOVER_MINUTES  = 5    # minutes between recovery steps

# Phone call probability per game-second (2%, active 8am–10pm)
PHONE_CALL_PROBABILITY    = 0.02
PHONE_CALL_HOUR_START     = 8
PHONE_CALL_HOUR_END       = 22


# ---------------------------------------------------------------------------
# Days-in-month table
# addr: days_in_month() helper used in game_simulate_one_second()
# ---------------------------------------------------------------------------
DAYS_IN_MONTH = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]


# ---------------------------------------------------------------------------
# Atari ST colour palette (from readFiles.py COLORS list)
# 16 palette entries in ST 0x0RGB format → RGB tuples
# addr: main_colorpalette[16]
# ---------------------------------------------------------------------------
ATARI_PALETTE_RGB = []
for _color_str in '0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split():
    ATARI_PALETTE_RGB.append((
        int(_color_str[1], 16) * 32,
        int(_color_str[2], 16) * 32,
        int(_color_str[3], 16) * 32,
    ))

# Clothing colour pairs (primary, secondary) for palette entries 1 and 2
# addr: clothing_color_primary[16] at DATA+0xC08, clothing_color_secondary[16] at DATA+0xBE8
# Extracted from LCP.PRG DATA segment. Atari ST 0x0RGB format.
# palette_apply_clothing_colors() sets main_colorpalette[1]=primary, [2]=secondary
CLOTHING_COLORS_PRIMARY = [
    0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0676, 0x0676, 0x0500,
    0x0500, 0x0735, 0x0140, 0x0641, 0x0623, 0x0036, 0x0242, 0x0442,
]
CLOTHING_COLORS_SECONDARY = [
    0x0060, 0x0760, 0x0606, 0x0066, 0x0767, 0x0007, 0x0700, 0x0030,
    0x0767, 0x0465, 0x0314, 0x0255, 0x0662, 0x0406, 0x0156, 0x0514,
]

# Skin tone colours for palette entry 6
# addr: skin_color_palette[8] at DATA+0xC28
# Extracted from LCP.PRG DATA segment. Atari ST 0x0RGB format.
# palette_apply_skin_colors() sets main_colorpalette[1]=main_colorpalette[2]=skin_color
SKIN_COLOR_PALETTE = [
    0x0512, 0x0742, 0x0567, 0x0762, 0x0745, 0x0145, 0x0160, 0x0565,
]


# ---------------------------------------------------------------------------
# Revert (bit-reversal) table — 256 entries
# addr: revert_table[256], used by lcp_flip_sprite_horizontal()
# Reverses bit order within a byte for horizontal sprite mirroring
# ---------------------------------------------------------------------------
def _build_revert_table() -> list[int]:
    table = []
    for i in range(256):
        rev = 0
        for bit in range(8):
            if i & (1 << bit):
                rev |= (1 << (7 - bit))
        table.append(rev)
    return table

REVERT_TABLE = _build_revert_table()


# ---------------------------------------------------------------------------
# LCP body sprite frame tables
# addr: body_sprite_frame_table[93] at 0x29bb2
# Maps PLAYER_STATE → frame index into BODY.LCP file
# ---------------------------------------------------------------------------
BODY_SPRITE_FRAME_TABLE = [
    0, 1, 2, 3, 4, 1, 6, 7, 43, 9, 10, 11, 12, 20, 21, 22, 21, 13, 14,
    15, 16, 17, 18, 19, 18, 23, 24, 25, 24, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 27, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 67, 68, 32, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
    95, 96, 97, 26, 5, 8,
]

# addr: carry_body_frame_table[25] at 0x29c6c
# Alternate body frames for states 0..24 when LCP is carrying an object
CARRY_BODY_FRAME_TABLE = [
    55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
    13, 14, 15, 16, 17, 18, 19, 18,
]

# addr: body_y_offset_per_state[93] at 0x29f8c
# Vertical pixel offset for body sprite positioning per PLAYER_STATE
BODY_Y_OFFSET_PER_STATE = [
    -2, -2, -2, -1, -2, -2, -2, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, -2, -2, -2, -2, 0, 0, 0, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, 0, -6, -6, -6, -2, -6, -6, 2, 1, 7, -7, -5, -5, -5,
    -5, -5, -4, -1, 0, -2, -2, -2, 11, 11, 11, 11, 11, -1, -1, -7, -7,
    -4, -7, -2, -2, -4, -2, -1, -2, -2, 0, 0, -2, -2, -2, -2, -3, -2,
    -3, -2, -2,
]


# ---------------------------------------------------------------------------
# LCP head sprite positioning tables
# addr: head_x_offset_per_state[93] at 0x29c9e
# Horizontal pixel offset for head sprite relative to body
# ---------------------------------------------------------------------------
HEAD_X_OFFSET_PER_STATE = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 6, 6, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
]

# addr: head_height_per_state[93] at 0x29d58
# Head height (pixels above body) per PLAYER_STATE
HEAD_HEIGHT_PER_STATE = [
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 18, 18, 18, 18, 17, 17, 17, 21,
    21, 18, 18, 18, 18, 18, 18, 18, 17, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 21, 21, 21, 21, 21, 21, 21, 18, 21, 21, 21, 21, 5, 5, 5, 5, 5,
    19, 19, 21, 21, 21, 21, 21, 21, 21, 21, 20, 21, 21, 20, 20, 21, 21,
    21, 21, 20, 21, 20, 21, 21,
]

# addr: head_default_angle_per_state[93] at 0x29e12
# Default horizontal head angle per PLAYER_STATE (0–7)
HEAD_DEFAULT_ANGLE_PER_STATE = [
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 2, 2, 2, 2, 0,
    0, 0, 0, 3, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0,
    0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 2, 2,
    2, 2, 2, 1, 4, 0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 4, 3, 1,
]


# ---------------------------------------------------------------------------
# Head animation tables
# addr: happiness_head_frame_offset[3] at 0x2ba2c
# Maps happiness (0=happy, 1=content, 2=sad) → head sprite row offset
# ---------------------------------------------------------------------------
HAPPINESS_HEAD_FRAME_OFFSET = [44, 0, 22]

# addr: head_tilt_frame_offset[4] at 0x2ba24
# Maps vertical tilt index (0–3) → frame offset within head row
HEAD_TILT_FRAME_OFFSET = [7, 12, 17, 1]

# addr: head_movement_delta_table[15] at 0x2ba06
# Delta lookup for head horizontal animation; indexed by (target - current + 7)
# Value 99 = overflow sentinel → fall back to default angle
HEAD_MOVEMENT_DELTA_TABLE = [1, 1, 1, 99, -1, -1, -1, 0, 1, 1, 1, 99, -1, -1, -1]

# Atari ST palette colour constants for skin
ST_PEACH = 0x0742       # healthy skin colour (palette entry 6)
ST_SICK_GREEN = 0x0450  # sick skin colour (palette entry 6)
