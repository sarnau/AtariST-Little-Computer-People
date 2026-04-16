"""
All enumerations for Little Computer People (Atari ST).
Translated from Ghidra-applied enum types on LCP.PRG.
Original compiler: Alcyon C (Digital Research CP/M-68K toolchain).
All integer values are 16-bit (short) on the original platform.
addr: 0x0296DC  (DATA segment — enum values applied from Ghidra analysis)
"""

from enum import IntEnum


# ---------------------------------------------------------------------------
# ACTION_ID — 40 player behaviour actions dispatched by do_action()
# addr: do_action switch table (verified: cases 0-39 only)
# ---------------------------------------------------------------------------
class ACTION_ID(IntEnum):
    ACTION_SIT_AND_EXERCISE      = 0
    ACTION_READ_NEWSPAPER        = 1
    ACTION_PLAY_COMPUTER         = 2
    ACTION_WASH_HANDS            = 3
    ACTION_GET_IN_OUT_OF_BED     = 4
    ACTION_LISTEN_SONG           = 5
    ACTION_PLAY_PIANO            = 6
    ACTION_WRITE_LETTER          = 7
    ACTION_DANCE                 = 8
    ACTION_YAWN_AND_STRETCH      = 9
    ACTION_PACE_NERVOUSLY        = 10
    ACTION_WANDER_IDLY           = 11
    ACTION_SLEEP                 = 12
    ACTION_DRINK                 = 13
    ACTION_NOD_HEAD              = 14
    ACTION_PEEK_AROUND           = 15
    ACTION_PLAY_A_GAME           = 16
    ACTION_BRUSH_TEETH           = 17
    ACTION_KITCHEN_CABINET       = 18
    ACTION_SIT_ON_COUCH_WITH_DOG = 19
    ACTION_LIGHT_FIREPLACE       = 20
    ACTION_USE_TOILET            = 21
    ACTION_TAKE_SHOWER           = 22
    ACTION_FEED_DOG              = 23
    ACTION_HELLO                 = 24
    ACTION_EAT_MEAL              = 25
    ACTION_PLAY_WITH_RECORD      = 26
    ACTION_OPEN_UPSTAIRS_CLOSET  = 27
    ACTION_GET_SNACK_FROM_FRIDGE = 28
    ACTION_OPEN_BEDROOM_CLOSET   = 29
    ACTION_GET_DRESSED           = 30
    ACTION_CLEAN_UP              = 31
    ACTION_TIDY_HOUSE            = 32
    ACTION_CHECK_FRONT_DOOR      = 33
    ACTION_TOGGLE_TV             = 34
    ACTION_CALL_DOG              = 35
    ACTION_WAKE_FROM_ALARM       = 36
    ACTION_PET_DOG               = 37
    ACTION_WAKE_UP_MORNING       = 38
    ACTION_GO_TO_BED_NIGHT       = 39
    # Event IDs (used in triggered_event_list, not in do_action switch)
    ACTION_EVENT_PHONE_CALL      = 40
    ACTION_EVENT_BOOK_DELIVERY   = 41
    ACTION_EVENT_FOOD_DELIVERY   = 42
    ACTION_EVENT_DOG_FOOD        = 43
    ACTION_EVENT_RECORD_DELIVERY = 44
    # Sentinel — no action pending
    ACTION_NONE                  = 0xFFFF


# ---------------------------------------------------------------------------
# PLAYER_STATE — LCP body sprite animation frame indices
# 91 states total (body.lcp contains 91 × 21-line frames)
# addr: lcp_state global, used in lcp_pathfind_one_step / sprite_lcp_body_update
# ---------------------------------------------------------------------------
class PLAYER_STATE(IntEnum):
    # Flat-floor walking cycle (8 frames, 0–7)
    STATE_WALK_FRAME_0     = 0
    STATE_WALK_FRAME_1     = 1
    STATE_WALK_FRAME_2     = 2
    STATE_WALK_FRAME_3     = 3   # footstep trigger
    STATE_WALK_FRAME_4     = 4
    STATE_WALK_FRAME_5     = 5
    STATE_WALK_FRAME_6     = 6
    STATE_WALK_FRAME_7     = 7   # footstep trigger
    # State 8 = stand idle (used as walk-complete sentinel)
    STATE_STAND_IDLE       = 8
    # Stair climbing up — 4-frame cycle (9–12)
    STATE_STAIR_UP_0       = 9
    STATE_STAIR_UP_1       = 10
    STATE_STAIR_UP_2       = 11
    STATE_STAIR_UP_3       = 12  # footstep trigger
    # Stair top landing — 4-frame cycle (13–16)
    STATE_STAIR_TOP_0      = 13
    STATE_STAIR_TOP_1      = 14
    STATE_STAIR_TOP_2      = 15
    STATE_STAIR_TOP_3      = 16  # footstep trigger
    # Stair descending — 4-frame cycle (17–20)
    STATE_STAIR_DOWN_0     = 17
    STATE_STAIR_DOWN_1     = 18  # footstep trigger
    STATE_STAIR_DOWN_2     = 19
    STATE_STAIR_DOWN_3     = 20  # footstep trigger
    # Stair bottom landing — 4-frame cycle (21–24)
    STATE_STAIR_BTM_0      = 21
    STATE_STAIR_BTM_1      = 22
    STATE_STAIR_BTM_2      = 23
    STATE_STAIR_BTM_3      = 24
    # Standing / idle poses (25+)
    STATE_STAND_FACING_SCREEN = 25
    STATE_STAND_SIDE_VIEW     = 26
    STATE_SIT_CHAIR           = 27
    STATE_SIT_COUCH           = 28
    STATE_SIT_DESK            = 29
    STATE_TYPE_LEFT           = 30
    STATE_TYPE_RIGHT          = 31
    STATE_EAT_BITE            = 32
    STATE_DRINK_GLASS         = 33
    STATE_EXERCISE_ARMS_UP    = 34
    STATE_EXERCISE_CROUCH     = 35
    STATE_SLEEP_IN_BED        = 36   # Ghidra: STATE_LIE_DOWN_IN_BED
    STATE_SLEEP_LYING         = 37   # Ghidra: STATE_LIE_DOWN_GETTING_IN
    STATE_SHOWER_1            = 38
    STATE_SHOWER_2            = 39
    STATE_SHOWER_3            = 40
    STATE_SHOWER_4            = 41
    STATE_SHOWER_5            = 42
    STATE_BRUSH_TEETH         = 43
    STATE_WASH_HANDS          = 44
    STATE_USE_TOILET          = 45
    STATE_PLAY_PIANO_1        = 46
    STATE_PLAY_PIANO_2        = 47
    STATE_DANCE_LEFT          = 48
    STATE_DANCE_RIGHT         = 49
    STATE_READ_NEWSPAPER      = 50
    STATE_WRITE_LETTER        = 51
    STATE_SIT_EXERCISE_1      = 52
    STATE_SIT_EXERCISE_2      = 53
    STATE_YAWN                = 54
    STATE_STRETCH             = 55
    STATE_WANDER_LOOK         = 56
    STATE_PLAY_COMPUTER       = 57
    STATE_PEEK_AROUND         = 58
    STATE_NOD_HEAD            = 59
    STATE_HELLO               = 60
    STATE_GET_IN_BED          = 61   # Ghidra: STATE_UNDRESS_AT_BED
    STATE_GET_OUT_BED         = 62   # Ghidra alias (may share sprite with 61)
    STATE_OPEN_CLOSET         = 63
    STATE_CLOSE_CLOSET        = 64
    STATE_CARRY_OBJECT        = 65
    STATE_PUT_DOWN_OBJECT     = 66
    STATE_PICK_UP_OBJECT      = 67
    STATE_FEED_DOG            = 68
    STATE_PET_DOG_1           = 69
    STATE_PET_DOG_2           = 70
    STATE_SIT_ON_COUCH_DOG    = 71
    STATE_LIGHT_FIRE_1        = 72
    STATE_LIGHT_FIRE_2        = 73
    STATE_PHONE_ANSWER        = 74
    STATE_PHONE_TALK          = 75
    STATE_PHONE_HANG_UP       = 76
    STATE_PLAY_RECORD_1       = 77
    STATE_PLAY_RECORD_2       = 78
    STATE_WATCH_TV            = 79
    STATE_PLAY_GAME_SIT       = 80
    STATE_PACE_1              = 81
    STATE_PACE_2              = 82
    STATE_WAKE_FROM_ALARM     = 83
    STATE_STRETCH_WAKE        = 84
    STATE_SNIFF               = 85
    STATE_DRESSED_STAND       = 86
    STATE_TIDY_1              = 87
    STATE_TIDY_2              = 88
    STATE_CLEAN_1             = 89
    STATE_CLEAN_2             = 90


# ---------------------------------------------------------------------------
# HOUSE_POS — 48 navigable positions across 3 floors
# addr: house_get_position_xy, lcp_walk_to_destination
# Floor 3 (top):    positions  0–15, Y baseline = 77
# Floor 2 (middle): positions 16–31, Y baseline = 140
# Floor 1 (bottom): positions 32–47, Y baseline = 202
# ---------------------------------------------------------------------------
class HOUSE_POS(IntEnum):
    # Floor 3 — Living Room / Study (top floor)
    POS_TOP_0           = 0
    POS_TOP_ARMCHAIR    = 2   # Ghidra: POS_TOP_ARMCHAIR (verified: disasm passes #0x2 to house_get_position_xy in action_read_newspaper @ 0x22d8e)
    POS_TOP_1           = 1   # waypoint (formerly misnamed POS_TOP_ARMCHAIR)
    POS_TOP_DANCE_FLOOR = 3   # Ghidra: POS_TOP_DANCE_FLOOR
    POS_TOP_FIREPLACE   = 4
    POS_TOP_LOG_AREA    = 5
    POS_TOP_6           = 6
    POS_TOP_STUDY_DOOR  = 7
    POS_TOP_8           = 8
    POS_TOP_9           = 9
    POS_TOP_DESK_CHAIR  = 10  # Ghidra: POS_TOP_DESK_CHAIR (verified: disasm passes #0xa to house_get_position_xy in action_write_letter)
    POS_TOP_DESK_LAMP   = 10  # legacy alias — same slot
    POS_TOP_11          = 11
    POS_TOP_FILING_CAB      = 12  # Ghidra: POS_TOP_FILING_CABINET (verified: disasm passes #0xc to house_get_position_xy in action_write_letter @ 0x23cf2)
    POS_TOP_FILING_CABINET  = 12  # Ghidra alias
    POS_TOP_RECORD_SHELF    = 12  # TODO verify separately — previously at 12, kept as alias until proven distinct
    POS_TOP_13          = 13
    POS_TOP_14          = 14
    POS_TOP_15          = 15
    # Floor 2 — Bedroom / Bathroom (middle floor)
    POS_MID_0              = 16
    POS_MID_BED            = 17
    POS_MID_DRESSER        = 18
    POS_MID_CLOSET         = 19
    POS_MID_COUCH          = 20
    POS_MID_SINK           = 21
    POS_MID_BATHROOM_SINK  = 21   # Ghidra alias
    POS_MID_TOILET         = 22
    POS_MID_TOILET_DOOR    = 22   # Ghidra alias
    POS_MID_SHOWER         = 23
    POS_MID_SHOWER_DOOR    = 23   # Ghidra alias
    POS_MID_SHOWER_EXIT    = 24   # walk-out position after shower
    POS_MID_SHOWER_INSIDE  = 25   # verified from binary: position 25
    POS_MID_26             = 26
    POS_MID_27             = 27
    POS_MID_28             = 28
    POS_MID_COMPUTER       = 29   # Ghidra: POS_MID_COMPUTER_DESK
    POS_MID_30             = 30
    POS_MID_31             = 31
    # Floor 1 — Kitchen / Entrance (bottom floor)
    POS_BTM_0              = 32
    POS_BTM_SINK           = 33
    POS_BTM_KITCHEN_SINK   = 33   # Ghidra alias
    POS_BTM_STOVE          = 34
    POS_BTM_WATER_TAP      = 34   # Ghidra alias (near stove/sink)
    POS_BTM_FRIDGE         = 35
    POS_BTM_CABINET        = 36
    POS_BTM_KITCHEN_CABINET = 36  # Ghidra alias
    POS_BTM_TABLE       = 37
    POS_BTM_DOG_BOWL    = 38
    POS_BTM_39          = 39
    POS_BTM_40          = 40
    POS_BTM_41          = 41
    POS_BTM_42          = 42
    POS_BTM_DOG_FOOD    = 43  # Ghidra: POS_BTM_DOG_FOOD — also phone area
    POS_BTM_43          = 43
    POS_BTM_44          = 44
    POS_BTM_45          = 45
    POS_BTM_FRONT_DOOR  = 46   # Ghidra: POS_BTM_FRONT_DOOR (X=294, front door at right edge)
    POS_BTM_SCREEN_EDGE = 47   # Ghidra: POS_BTM_SCREEN_EDGE (out-of-bounds fallback)


# ---------------------------------------------------------------------------
# SPRITE_ID — 56 logical sprite slots
# addr: sprite_def_image[60], ARCHITECTURE.md sprite_id enum
# ---------------------------------------------------------------------------
class SPRITE_ID(IntEnum):
    SPRITE_0                  = 0
    SPRITE_1                  = 1
    SPRITE_2                  = 2
    SPRITE_GLASS              = 3
    SPRITE_GAME_BOX           = 4
    SPRITE_5                  = 5
    SPRITE_6                  = 6
    SPRITE_VINYL_RECORD       = 7
    SPRITE_TYPEWRITER         = 8
    SPRITE_FOOD_PACKAGE       = 9
    SPRITE_10                 = 10
    SPRITE_11                 = 11
    SPRITE_TABLE_SETTING      = 12
    SPRITE_DOOR_ANIM_1        = 13
    SPRITE_DOOR_ANIM_2        = 14
    SPRITE_DOOR_ANIM_3        = 15
    SPRITE_CLOSET_LCP_INSIDE  = 16
    SPRITE_CLOSET_AJAR        = 17
    SPRITE_CLOSET_WIDE_OPEN   = 18
    SPRITE_PET_DOG_1          = 19
    SPRITE_PET_DOG_2          = 20
    SPRITE_DOG_SIT            = 21
    SPRITE_FIREWOOD           = 22
    SPRITE_COOKING_POT        = 23
    SPRITE_DOOR_STUDY_1       = 24
    SPRITE_DOOR_STUDY_AJAR    = 25
    SPRITE_DOOR_STUDY_OPEN    = 26
    SPRITE_PET_HAND_1         = 27
    SPRITE_PET_HAND_2         = 28
    SPRITE_PET_HAND_3         = 29
    SPRITE_PET_HAND_4         = 30
    SPRITE_PET_HAND_5         = 31
    SPRITE_PET_HAND_6         = 32
    SPRITE_DOG_LAY_DOWN       = 33
    SPRITE_DOG_WALK_RIGHT_1   = 34
    SPRITE_DOG_WALK_RIGHT_2   = 35
    SPRITE_DOG_WALK_RIGHT_3   = 36
    SPRITE_DOG_WALK_RIGHT_4   = 37
    SPRITE_DOG_WALK_RIGHT_5   = 38
    SPRITE_DOG_WALK_RIGHT_6   = 39
    SPRITE_DOG_WALK_RIGHT_7   = 40
    SPRITE_DOG_WALK_RIGHT_8   = 41
    SPRITE_DOG_EATING_1       = 42
    SPRITE_DOG_EATING_2       = 43
    SPRITE_DOG_EATING_3       = 44
    SPRITE_READING_1          = 45
    SPRITE_READING_2          = 46
    SPRITE_READING_3          = 47
    SPRITE_SUITCASE           = 48
    SPRITE_BOOK               = 49
    SPRITE_VINYL_CARRY        = 50
    SPRITE_PET_HAND_7         = 51
    SPRITE_DESK_LAMP          = 52
    SPRITE_TYPING_1           = 51  # shares slot with PET_HAND_7 per ARCHITECTURE.md
    SPRITE_TYPING_2           = 52
    SPRITE_TYPING_3           = 53
    SPRITE_TYPING_4           = 54
    SPRITE_55                 = 55


# ---------------------------------------------------------------------------
# SPRITE_LAYER — visibility / depth layer for sprite slots
# addr: sprite_layer_flags[60], sprite_update_slots()
# ---------------------------------------------------------------------------
class SPRITE_LAYER(IntEnum):
    SPRITE_HIDDEN      = 0
    SPRITE_BEHIND_LCP  = 1
    SPRITE_IN_FRONT    = 2


# ---------------------------------------------------------------------------
# SICKNESS_LEVEL — health degradation levels
# addr: LCP.sickness_level, game_simulate_one_second()
# ---------------------------------------------------------------------------
class SICKNESS_LEVEL(IntEnum):
    SICKNESS_HEALTHY  = 0
    SICKNESS_LEVEL_1  = 1
    SICKNESS_LEVEL_2  = 2   # forces MOOD_SAD, skin turns green
    SICKNESS_LEVEL_3  = 3
    SICKNESS_CRITICAL = 4


# ---------------------------------------------------------------------------
# HAPPINESS_LEVEL — current mood
# addr: LCP.happiness, game_simulate_one_second()
# ---------------------------------------------------------------------------
class HAPPINESS_LEVEL(IntEnum):
    MOOD_HAPPY   = 0
    MOOD_CONTENT = 1
    MOOD_SAD     = 2


# ---------------------------------------------------------------------------
# NEED_LEVEL — thirst / hunger satiation level (0=satisfied, 3=critical)
# addr: LCP.thirst_level, LCP.hunger_level
# ---------------------------------------------------------------------------
class NEED_LEVEL(IntEnum):
    NEED_SATISFIED = 0
    NEED_MILD      = 1
    NEED_ELEVATED  = 2
    NEED_CRITICAL  = 3   # triggers sickness


# ---------------------------------------------------------------------------
# FACING_DIR — horizontal direction the LCP character faces
# addr: lcp_facing_direction, lcp_flip_sprite_horizontal()
# ---------------------------------------------------------------------------
class FACING_DIR(IntEnum):
    FACING_RIGHT = 0
    FACING_LEFT  = 1


# ---------------------------------------------------------------------------
# PERSONALITY_TYPE — 4 personality archetypes affecting action weighting
# addr: LCP.personality_type, activity_schedule_table
# ---------------------------------------------------------------------------
class PERSONALITY_TYPE(IntEnum):
    PERSONALITY_0 = 0
    PERSONALITY_1 = 1
    PERSONALITY_2 = 2
    PERSONALITY_3 = 3


# ---------------------------------------------------------------------------
# CLOTHING_COLOR_ID — 16 outfit colour combinations (palette entries 1–2)
# addr: LCP.clothing_color, clothing_color_primary/secondary[16]
# ---------------------------------------------------------------------------
class CLOTHING_COLOR_ID(IntEnum):
    OUTFIT_0  = 0
    OUTFIT_1  = 1
    OUTFIT_2  = 2
    OUTFIT_3  = 3
    OUTFIT_4  = 4
    OUTFIT_5  = 5
    OUTFIT_6  = 6
    OUTFIT_7  = 7
    OUTFIT_8  = 8
    OUTFIT_9  = 9
    OUTFIT_10 = 10
    OUTFIT_11 = 11
    OUTFIT_12 = 12
    OUTFIT_13 = 13
    OUTFIT_14 = 14
    OUTFIT_15 = 15


# ---------------------------------------------------------------------------
# SKIN_COLOR_ID — 8 skin tone options (palette entry 6)
# addr: LCP.skin_tone, skin_color_palette[8]
# ---------------------------------------------------------------------------
class SKIN_COLOR_ID(IntEnum):
    SKIN_0 = 0
    SKIN_1 = 1
    SKIN_2 = 2
    SKIN_3 = 3
    SKIN_4 = 4
    SKIN_5 = 5
    SKIN_6 = 6
    SKIN_7 = 7


# ---------------------------------------------------------------------------
# ST_COLOR — Atari ST hardware colour format (0x0RGB, 3 bits per channel)
# addr: main_colorpalette[16], clothing_color_primary/secondary[16]
# Values used in palette initialisation in main()
# ---------------------------------------------------------------------------
class ST_COLOR(IntEnum):
    ST_BLACK        = 0x0000
    ST_DARK_BLUE    = 0x0007
    ST_DARK_GREEN   = 0x0070
    ST_DARK_CYAN    = 0x0077
    ST_DARK_RED     = 0x0700
    ST_DARK_MAGENTA = 0x0707
    ST_BROWN        = 0x0740
    ST_LIGHT_GRAY   = 0x0555
    ST_DARK_GRAY    = 0x0333
    ST_BLUE         = 0x000F
    ST_GREEN        = 0x00F0
    ST_CYAN         = 0x00FF
    ST_RED          = 0x0F00
    ST_MAGENTA      = 0x0F0F
    ST_YELLOW       = 0x0FF0
    ST_WHITE        = 0x0FFF
    # Special semantic colours
    ST_PEACH        = 0x0742   # healthy skin colour (palette entry 6)
    ST_SICK_GREEN   = 0x0070   # sick skin colour (palette entry 6)


# ---------------------------------------------------------------------------
# DOG_BOWL_STATUS — current food bowl fill level
# addr: dog_bowl_status, dog_move_and_animate()
# ---------------------------------------------------------------------------
class DOG_BOWL_STATUS(IntEnum):
    BOWL_EMPTY = 0
    BOWL_HALF  = 1
    BOWL_FULL  = 2


# ---------------------------------------------------------------------------
# HEAD_ANIM_MODE — flags controlling LCP head sprite movement
# addr: head_anim_mode, sprite_lcp_head_update()
# ---------------------------------------------------------------------------
class HEAD_ANIM_MODE(IntEnum):
    HEAD_ANIM_ALL_RANDOM = 0     # all random movement (no constraints)
    # Bitmask constants for head_anim_mode bitfield:
    HEAD_ANIM_HORIZ_AMP_MASK   = 0x03  # bits 0–1: horizontal amplitude (0=random, 1-3=fixed)
    HEAD_ANIM_HORIZ_DIR_MASK   = 0x0C  # bits 2–3: horizontal direction (0=random, ≥8=negate)
    HEAD_ANIM_VERT_SELECT_MASK = 0x60  # bits 5–6: vertical tilt selection (0=random)
    HEAD_ANIM_VERT_OVERRIDE    = 0x80  # bit 7: force specific vertical tilt
    HEAD_ANIM_VERT_RANGE_MASK  = 0xE0  # bits 5–7: full vertical range (with override)
    # Sign bit (bit 15): negative value = animation frozen/disabled


# ---------------------------------------------------------------------------
# MIDI_SEQ_PHASE — MIDI sequencer state machine phases
# addr: midi_seq_phase, midi_seq_tick_handler()
# ---------------------------------------------------------------------------
class MIDI_SEQ_PHASE(IntEnum):
    SEQ_WAIT_NOTE_EXPIRE  = 0
    SEQ_PARSE_NEXT_EVENT  = 1
    SEQ_SONG_ENDING       = 2


# ---------------------------------------------------------------------------
# ENV_PHASE — PSG software ADSR envelope phases
# addr: PSG_ENVELOPE.phase, psg_process_envelopes()
# ---------------------------------------------------------------------------
class ENV_PHASE(IntEnum):
    ENV_IDLE    = 0
    ENV_ATTACK  = 1
    ENV_DECAY   = 2
    ENV_SUSTAIN = 3
    ENV_RELEASE = 4
    ENV_FADEOUT = 5


# ---------------------------------------------------------------------------
# SOUND_EFFECT_ID — DoSound effect indices (23 effects total)
# addr: soundeffect_irq_play(), SOUNDS.LCP
# ---------------------------------------------------------------------------
class SOUND_EFFECT_ID(IntEnum):
    SFX_FOOTSTEP_STAIRS  = 0
    SFX_FOOTSTEP_CARPET  = 1
    SFX_FOOTSTEP_WOOD    = 2
    SFX_FOOTSTEP_3       = 3
    SFX_FOOTSTEP_4       = 4
    SFX_FOOTSTEP_5       = 5
    SFX_TV_CLICK         = 6
    SFX_SPEECH           = 7
    SFX_HEAD_NOD         = 8
    SFX_GREETING         = 9
    SFX_CLICK            = 10
    SFX_TYPEWRITER_KEY   = 11
    SFX_DOORBELL         = 12
    SFX_DOORBELL_ECHO    = 13
    SFX_DOOR_OPEN        = 14
    SFX_DOOR_CLOSE       = 15
    SFX_TOILET_FLUSH     = 16
    SFX_TOILET_REFILL    = 17
    SFX_WATER_RUNNING    = 18
    SFX_WATER_TAP        = 19
    SFX_ALARM_CLOCK      = 20
    SFX_PHONE_RING       = 21
    SFX_SNORING          = 22


# ---------------------------------------------------------------------------
# VDI_COPY_MODE — VDI vro_cpyfm raster operation modes
# addr: sprite_draw(), screen_render_8hz()
# ---------------------------------------------------------------------------
class VDI_COPY_MODE(IntEnum):
    ALL_WHITE     = 0
    S_AND_D       = 1
    S_AND_NOTD    = 2
    S_ONLY        = 3
    NOTS_AND_D    = 4   # AND inverted mask → punch transparent hole
    D_ONLY        = 5
    S_XOR_D       = 6   # XOR sprite onto cleared area → paint pixels
    S_OR_D        = 7
    NOTS_AND_NOTD = 8
    NOTS_XOR_D    = 9
    NOT_D         = 10
    S_OR_NOTD     = 11
    NOT_S         = 12
    NOTS_OR_D     = 13
    NOTS_OR_NOTD  = 14
    ALL_BLACK     = 15


# ---------------------------------------------------------------------------
# CARD_TYPE — playing card suit/rank encoding used by mini-games
# addr: poker_main(), poker_blackjack_main(), poker_war_main()
# ---------------------------------------------------------------------------
class CARD_SUIT(IntEnum):
    SUIT_CLUBS    = 0
    SUIT_DIAMONDS = 1
    SUIT_HEARTS   = 2
    SUIT_SPADES   = 3


class CARD_RANK(IntEnum):
    RANK_2  = 0
    RANK_3  = 1
    RANK_4  = 2
    RANK_5  = 3
    RANK_6  = 4
    RANK_7  = 5
    RANK_8  = 6
    RANK_9  = 7
    RANK_10 = 8
    RANK_J  = 9
    RANK_Q  = 10
    RANK_K  = 11
    RANK_A  = 12


# ---------------------------------------------------------------------------
# WORD_ID — vocabulary indices for natural-language command parser
# addr: enteredword_to_action, words[] in LCP.py
# 161 words total (indices 0–160)
# ---------------------------------------------------------------------------
class WORD_ID(IntEnum):
    WORD_PLEASE      = 0
    WORD_DO          = 1
    WORD_YOU         = 2
    WORD_LIKE        = 3
    WORD_ENJOY       = 4
    WORD_WILL        = 5
    WORD_WOULD       = 6
    WORD_PLAY        = 7
    WORD_PERFORM     = 8
    WORD_USE         = 9
    WORD_TRY         = 10
    WORD_PLAYING     = 11
    WORD_ALLERGY     = 12
    WORD_ALLERGIC    = 13
    WORD_FEVER       = 14
    WORD_DUST        = 15
    WORD_POLLEN      = 16
    WORD_HANKY       = 17
    WORD_RELAX       = 18
    WORD_LIGHT       = 19
    WORD_START       = 20
    WORD_MAKE        = 21
    WORD_BURN        = 22
    WORD_IGNITE      = 23
    WORD_BUILD       = 24
    WORD_LOOKS       = 25
    WORD_IS          = 26
    WORD_SEEMS       = 27
    WORD_APPEARS     = 28
    WORD_SEEM        = 29
    WORD_LOOK        = 30
    WORD_APPEAR      = 31
    WORD_HEAR        = 32
    WORD_LISTEN      = 33
    WORD_PUT         = 34
    WORD_START2      = 35
    WORD_SPIN        = 36
    WORD_ON          = 37
    WORD_CLEAN       = 38
    WORD_TIDY        = 39
    WORD_PICK        = 40
    WORD_UP          = 41
    WORD_SLOPPY      = 42
    WORD_MESSY       = 43
    WORD_UNTIDY      = 44
    WORD_SHOULD      = 45
    WORD_OUGHT       = 46
    WORD_PROGRAM     = 47
    WORD_UTILITIES   = 48
    WORD_MATH        = 49
    WORD_HOMEWORK    = 50
    WORD_ADD         = 51
    WORD_SUBTRACT    = 52
    WORD_MULTIPLY    = 53
    WORD_DIVIDE      = 54
    WORD_TICKLE      = 55
    WORD_TYPE        = 56
    WORD_TELL        = 57
    WORD_WRITE       = 58
    WORD_CONFIDE     = 59
    WORD_BRUSH       = 60
    WORD_FLOSS       = 61
    WORD_DRINK       = 62
    WORD_IMBIBE      = 63
    WORD_GET         = 64
    WORD_FEED        = 65
    WORD_FILL        = 66
    WORD_OPEN        = 67
    WORD_DANCE       = 68
    WORD_MOON        = 69
    WORD_SHOW        = 70
    WORD_LIKE2       = 71
    WORD_TIRED       = 72
    WORD_BORED       = 73
    WORD_APATHETIC   = 74
    WORD_HATE        = 75
    WORD_AWFUL       = 76
    WORD_IF          = 77
    WORD_WHAT        = 78
    WORD_WHATS       = 79
    WORD_IN          = 80
    WORD_INSIDE      = 81
    WORD_STORED      = 82
    WORD_KEEP        = 83
    WORD_IS2         = 84
    WORD_PIANO       = 85
    WORD_ORGAN       = 86
    WORD_STEREO      = 87
    WORD_TURNTABLE   = 88
    WORD_MUSIC       = 89
    WORD_RECORD      = 90
    WORD_PLATTER     = 91
    WORD_FIRE        = 92
    WORD_FIREPLACE   = 93
    WORD_LOG         = 94
    WORD_CHILLY      = 95
    WORD_COLD        = 96
    WORD_PROBLEM     = 97
    WORD_PROBLEMS    = 98
    WORD_TROUBLES    = 99
    WORD_MATTER      = 100
    WORD_LETTER      = 101
    WORD_NOTE        = 102
    WORD_SONG        = 103
    WORD_TUNE        = 104
    WORD_SONATA      = 105
    WORD_FUGUE       = 106
    WORD_SERENADE    = 107
    WORD_JAZZ        = 108
    WORD_BOOGIE      = 109
    WORD_IVORIES     = 110
    WORD_TEETH       = 111
    WORD_HYGIENE     = 112
    WORD_GLASS       = 113
    WORD_COOLER      = 114
    WORD_DOG         = 115
    WORD_PET         = 116
    WORD_MUTT        = 117
    WORD_POOCH       = 118
    WORD_BOWL        = 119
    WORD_DISH        = 120
    WORD_CAN         = 121
    WORD_TV          = 122
    WORD_CHAIR       = 123
    WORD_COMPUTER    = 124
    WORD_ATARI       = 125
    WORD_WATER       = 126
    WORD_LIQUID      = 127
    WORD_LIQUIDS     = 128
    WORD_FLUID       = 129
    WORD_FLUIDS      = 130
    WORD_UPSTAIRS    = 131
    WORD_BEDROOM     = 132
    WORD_CLOSET      = 133
    WORD_KITCHEN     = 134
    WORD_FILING      = 135
    WORD_CABINET     = 136
    WORD_FREEZER     = 137
    WORD_REFRIDGERATOR = 138
    WORD_FRIDGE      = 139
    WORD_DRESSER     = 140
    WORD_NIGHTSTAND  = 141
    WORD_ADDITION    = 142
    WORD_SUBTRACTION = 143
    WORD_MULTIPLICATION = 144
    WORD_DIVISION    = 145
    WORD_HOUSE       = 146
    WORD_HOME        = 147
    WORD_GAME        = 148
    WORD_CARDS       = 149
    WORD_POKER       = 150
    WORD_WAR         = 151
    WORD_CARD        = 152
    WORD_ANAGRAMS    = 153
    WORD_BLACKJACK   = 154
    WORD_EXCUSE      = 155
    WORD_PARDON      = 156
    WORD_HELLO       = 157
    WORD_ATTENTION   = 158
    WORD_HEY         = 159
    # word 160 is a sentinel / padding entry
