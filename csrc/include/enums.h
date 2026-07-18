/*
 * enums.h -- symbolic constants (Alcyon C has no enum; use #define).
 *
 * Values match the Ghidra enum definitions verified against LCP.PRG.
 * Only the constants referenced by the currently-ported modules are
 * defined here; add to this file as new subsystems come online.
 *
 * addr: enum blocks in Ghidra, mirrored from lcp/enums.py.
 */

#ifndef ENUMS_H
#define ENUMS_H

/* ---- Need levels ------------------------------------------------------- */
#define NEED_NONE               0
#define NEED_MILD               1
#define NEED_MODERATE           2
#define NEED_SEVERE             3

/* ---- Sickness ---------------------------------------------------------- */
#define SICKNESS_HEALTHY        0
#define SICKNESS_MILD           1
#define SICKNESS_MODERATE       2
#define SICKNESS_SEVERE         3
#define SICKNESS_CRITICAL       4

#define DIR_IMPROVING           (-1)
#define DIR_WORSENING           1
#define DIR_STABLE              0

/* ---- Mood -------------------------------------------------------------- */
#define MOOD_HAPPY              0
#define MOOD_CONTENT            1
#define MOOD_SAD                2

/* ---- Facing direction ------------------------------------------------- */
#define FACING_RIGHT            0
#define FACING_LEFT             1

/* ---- Sprite layers ---------------------------------------------------- */
#define SPRITE_HIDDEN           0
#define SPRITE_BEHIND_LCP       (-1)
#define SPRITE_IN_FRONT         1

/* ---- PLAYER_STATE ----------------------------------------------------
   Dumped verbatim from Ghidra's PLAYER_STATE enum (LCP.PRG loaded).
     0..7     Walk cycle (frames 3 and 7 trigger footstep)
     8        STATE_STAND_IDLE (== stand at rest)
     9..12    Stair climb (frame 12 = FRAME_3_STEP)
     13..16   Stair top-of-flight entry
     17..20   Stair descend
     21..24   Stair bottom-of-flight
     25..90   Single-pose actions.  0x36..0x37 and 0x49..0x4f are unused. */
#define STATE_WALK_FRAME_0                       0
#define STATE_WALK_FRAME_1                       1
#define STATE_WALK_FRAME_2                       2
#define STATE_WALK_FRAME_3_STEP                  3
#define STATE_WALK_FRAME_4                       4
#define STATE_WALK_FRAME_5                       5
#define STATE_WALK_FRAME_6                       6
#define STATE_WALK_FRAME_7_STEP                  7
#define STATE_STAND_IDLE                         8
#define STATE_STR_CLIMB_F0                9
#define STATE_STR_CLIMB_F1               10
#define STATE_STR_CLIMB_F2               11
#define STATE_STR_CLIMB_F3S          12
#define STATE_STR_TOP_F0                 13
#define STATE_STR_TOP_F1                 14
#define STATE_STR_TOP_F2                 15
#define STATE_STR_TOP_F3S            16
#define STATE_STR_DESC_F0             17
#define STATE_STR_DESC_F1             18
#define STATE_STR_DESC_F2             19
#define STATE_STR_DESC_F3S        20
#define STATE_STR_BTM_F0                 21
#define STATE_STR_BTM_F1                 22
#define STATE_STR_BTM_F2                 23
#define STATE_STR_BTM_F3                 24
#define STATE_BEND_AND_REACH                    25
#define STATE_HANDS_DOWN                 26
#define STATE_HANDS_UP                   27
#define STATE_SITTING_LEAN_BACK                 28
#define STATE_SITTING_AT_DESK                   29
#define STATE_SIT_AT_DESK                       29      /* alias */
#define STATE_BEND_DOWN                         30
#define STATE_REACH_FORWARD                     31
#define STATE_PICK_UP_FROM_FLOOR                32
#define STATE_STAND_FACING_SCREEN               33
#define STATE_STAND_SIDE_VIEW                   34
#define STATE_CROUCH_DOWN                       35
#define STATE_EXERCISE_CROUCH                   35      /* alias for CROUCH_DOWN */
#define STATE_CARRY_WALK_FRAME_0                36
#define STATE_CARRY_WALK_FRAME_1                37
#define STATE_VINYL_IDLE                 38
#define STATE_VINYL_REACH_L           39
#define STATE_VINYL_REACH_R          40
#define STATE_VINYL_PULL_OUT             41
#define STATE_STOKE_FIREPLACE                   42
#define STATE_WRITE_AT_DESK                     43
#define STATE_DESK_TYPE_L            44
#define STATE_DESK_TYPE_R           45
#define STATE_SIT_IN_ARMCHAIR                   46
#define STATE_READ_PAPER_HOLD                   47
#define STATE_READ_PAPER_TURN_PAGE              48
#define STATE_UNDRESS_AT_BED                    49
#define STATE_LIE_DOWN_GETTING_IN               50
#define STATE_LIE_DOWN_IN_BED                   51
#define STATE_SIT_COUCH_UPRIGHT                 52
#define STATE_SIT_COUCH_PETTING_DOG             53
/* 54..55 unused */
#define STATE_PHONE_PICKUP                      56
#define STATE_PHONE_TALKING                     57
#define STATE_EX_ARMS_CTR              58
#define STATE_EX_ARMS_UP                  59
#define STATE_EX_ARMS_WIDE                60
#define STATE_WASH_HANDS_CENTER                 61
#define STATE_WASH_HANDS_LEFT                   62
#define STATE_WASH_HANDS_RIGHT                  63
#define STATE_SHOWER_STAND                      64
#define STATE_SHR_WASH_L                  65
#define STATE_SHR_WASH_R                 66
#define STATE_SHR_SCRUB_L                 67
#define STATE_SHR_SCRUB_R                68
#define STATE_BRUSH_TEETH                       69
#define STATE_DRINK_FROM_GLASS                  70
#define STATE_EAT_BITE                          71
#define STATE_EAT_CHEW                          72
/* 73..79 unused */
#define STATE_DANCE_STEP_LEFT                   80
#define STATE_DANCE_STEP_RIGHT                  81
#define STATE_YAWN_MOUTH_OPEN                   82
#define STATE_YAWN_STRETCH_ARMS                 83
#define STATE_PACE_SHIFT_LEFT                   84
#define STATE_PACE_SHIFT_RIGHT                  85
#define STATE_IDLE_SHRUG_START                  86
#define STATE_IDLE_SHRUG_HOLD                   87
#define STATE_SLP_BREATHE_I                  88
#define STATE_SLP_BREATHE_O                 89
#define STATE_REACH_INTO_CABINET                90

/* ---- Head animation modes (see HEAD_ANIM_MODE below for canonical set) */

/* ---- color_enum (dumped from Ghidra) --------------------------------
   Values 0..15 are color_enum indices, NOT VDI palette slots.  The
   drawing calls pass a color_enum through vdi_colt[] to get the
   underlying VDI palette slot -- Ghidra's table is a permutation, so
   using the wrong color_enum here produces the wrong on-screen hue. */
#define COLOR_black                              0
#define COLOR_olive                              1
#define COLOR_lt_green                           2
#define COLOR_pink                               3
#define COLOR_brown                              4
#define COLOR_green                              5
#define COLOR_pink_2                             6
#define COLOR_yellow                             7
#define COLOR_blueish_sky                        8
#define COLOR_lt_brown                           9
#define COLOR_red                               10
#define COLOR_grey                              11
#define COLOR_lt_grey                           12
#define COLOR_blue                              13
#define COLOR_white                             14

/* Sat/Sun weekday consts (used by chk_timA). */
#define NEED_SATISFIED                          0

/* ---- HEAD_ANIM_MODE (dumped from Ghidra) ------------------------------
   Bit fields inside head_anim_mode (g_hamod):
     bits 0..2   HEAD_ANIM_HORIZONTAL_AMPLITUDE (mask 0x03 in binary, but
                 the enum encodes it as value 3 for the "amplitude enabled"
                 marker; sp_lcha masks with HEAD_MODE_H_AMPLITUDE = 0x07)
     bit 3       HEAD_ANIM_HORIZONTAL_RANGE (0x08) or 0xC (see sp_lcha)
     bits 5..6   HEAD_ANIM_VERTICAL_RANGE (0x60)
     bit 7       HEAD_ANIM_VERTICAL_OVERRIDE (0x80)
   Composite values (HEAD_ANIM_READING = 0x41, WALKING = 0x42, etc.) mix
   the bits into ready-made mode selectors. */
#define HEAD_ANIM_DISABLED              (-1)
#define HEAD_ANIM_SHOWER                0x02
#define HEAD_ANIM_HORIZONTAL_AMPLITUDE  0x03
#define HEAD_ANIM_HORIZONTAL_RANGE      0x0C
#define HEAD_ANIM_READING               0x41
#define HEAD_ANIM_WALKING               0x42
#define HEAD_ANIM_COMPUTER              0x4A
#define HEAD_ANIM_VERTICAL_RANGE        0x60
#define HEAD_ANIM_VERTICAL_OVERRIDE     0x80

/* ---- HOUSE_POS (dumped verbatim from Ghidra HOUSE_POS enum) ----------- */
#define POS_TOP_LIVING_ROOM              0
#define POS_TOP_DANCE_FLOOR              1
#define POS_TOP_ARMCHAIR                 2
#define POS_TOP_GAME_TABLE               3
#define POS_TOP_GAME_CHAIR_LEFT          4
#define POS_TOP_GAME_CHAIR_RIGHT         5
#define POS_TOP_RECORD_SHELF             6
#define POS_TOP_STUDY_DOOR               7
#define POS_TOP_FIREPLACE_LEFT           8
#define POS_TOP_FIREPLACE_CENTER         9
#define POS_TOP_DESK_CHAIR              10
#define POS_TOP_FIREPLACE_RIGHT         11
#define POS_TOP_FILING_CABINET          12
#define POS_TOP_FIREPLACE_HEARTH        13
#define POS_TOP_GAME_WALK_IN            14
#define POS_TOP_GAME_WALK_OUT           15
#define POS_MID_STAIR_LANDING           16
#define POS_MID_COUCH                   17
#define POS_MID_BED                     18
#define POS_MID_BEDROOM_WALK            19
#define POS_MID_BEDROOM_CLOSET          20
#define POS_MID_DRESSER                 21
#define POS_MID_BATHROOM_SINK           22
#define POS_MID_TOILET_DOOR             23
#define POS_MID_SHOWER_INSIDE           24
#define POS_MID_SHOWER_DOOR             25
#define POS_MID_TOILET                  26
#define POS_MID_BATHROOM_ENTRANCE       27
/* 28 unused */
#define POS_MID_COMPUTER_DESK           29
#define POS_MID_PIANO                   30
/* 31 unused */
/* POS_BTM_STAIR_LANDING = 32, defined below */
#define POS_BTM_DOG_BOWL                33
#define POS_BTM_STOVE                   34
#define POS_BTM_FRIDGE                  35
#define POS_BTM_KITCHEN_SINK            36
#define POS_BTM_KITCHEN_CABINET         37
#define POS_BTM_TABLE_LEFT              38
#define POS_BTM_TABLE_RIGHT             39
#define POS_BTM_FRONT_DOOR_INSIDE       40
#define POS_BTM_WATER_TAP               41
#define POS_BTM_DINING_AREA             42
#define POS_BTM_DOG_FOOD                43
#define POS_BTM_DOG_FOOD_STORE          44
#define POS_BTM_FIREPLACE_LOGS          45
#define POS_BTM_FRONT_DOOR              46
#define POS_BTM_SCREEN_EDGE             47

/* ---- SPRITE_ID (study doors + carried objects) ------------------------ */
#define SPRITE_DOOR_STUDY_1             0x18
#define SPRITE_DOOR_STUDY_AJAR          0x19
#define SPRITE_DOOR_STUDY_WIDE_OPEN     0x1a
#define SPRITE_FOOD_PACKAGE             9
#define SPRITE_BOOK                     0x31
#define SPRITE_VINYL_CARRY              0x32
#define SPRITE_SUITCASE                 48      /* 0x30, carried in cs_mvIn */
#define SPRITE_GLASS                    3
#define SPRITE_DOOR_ANIM_1              0x0d
#define SPRITE_DOOR_ANIM_2              0x0e
#define SPRITE_DOOR_ANIM_3              0x0f
#define SPRITE_COOKING_POT              0x17
#define SPRITE_TABLE_SETTING            0x0c
#define SPRITE_STUDY_DOOR_FRAME         6       /* also used as toothbrush */
#define SPRITE_55                       55
#define SPRITE_VINYL_RECORD             7
#define SPRITE_READING_1                0x2d
#define SPRITE_DOG_SIT                  0x15
#define SPRITE_FIREWOOD                 0x16
#define SPRITE_CLOSET_WIDE_OPEN         0x12
#define SPRITE_CLOSET_AJAR              0x11
#define SPRITE_CLOSET_LCP_INSIDE        0x10
#define SPRITE_TYPEWRITER               8
#define SPRITE_TYPING_1                 0x33
#define SPRITE_TYPING_2                 0x34
#define SPRITE_TYPING_3                 0x35
#define SPRITE_TYPING_4                 0x36
#define SPRITE_GAME_BOX                 4       /* also mini-game box */

/* ---- Dog bowl state --------------------------------------------------- */
#define BOWL_EMPTY                      0
#define BOWL_HALF                       1
#define BOWL_FULL                       2

/* ---- Sound-effect IDs (dumped from Ghidra SOUND_EFFECT_ID) ------------ */
#define SFX_FOOTSTEP_STAIRS              0
#define SFX_FOOTSTEP_CARPET              1
#define SFX_FOOTSTEP_WOOD                2
#define SFX_FOOTSTEP_3                   3
#define SFX_FOOTSTEP_4                   4
#define SFX_FOOTSTEP_5                   5
#define SFX_TV_CLICK                     6
#define SFX_SPEECH                       7
#define SFX_HEAD_NOD                     8
#define SFX_GREETING                     9
#define SFX_CLICK                       10
#define SFX_TYPEWRITER_KEY              11
#define SFX_DOORBELL                    12
#define SFX_DOORBELL_ECHO               13
#define SFX_DOOR_OPEN                   14
#define SFX_DOOR_CLOSE                  15
#define SFX_TOILET_FLUSH                16
#define SFX_TOILET_REFILL               17
#define SFX_WATER_RUNNING               18
#define SFX_WATER_TAP                   19
#define SFX_ALARM_CLOCK                 20
#define SFX_PHONE_RING                  21
#define SFX_SNORING                     22

/* ---- Palette values (12-bit RGB, Atari ST format) --------------------- */
#define ST_PEACH                        0x743
#define ST_SICK_GREEN                   0x363

/* ---- VDI mode ------------------------------------------------------- */
#define MD_TRANS                        2
#define MD_REPLACE                      1

/* ---- MIDI sequencer phase ------------------------------------------- */
#define SEQ_PHASE_IDLE                          0
#define SEQ_PHASE_WAIT_NOTE_EXPIRE              0
#define SEQ_PHASE_PARSE_NEXT_EVENT              1
#define SEQ_PHASE_SONG_ENDING                   2

/* ---- Song header command bytes -------------------------------------- */
#define MIDI_HDR_SET_CHANNEL_COUNT              0x80
#define MIDI_HDR_SET_TEMPO                      0x81
#define MIDI_HDR_SET_VOLUME                     0x83
#define MIDI_HDR_BUILD_SCALE_TABLE              0x84
#define MIDI_HDR_PROGRAM_CHANGE                 0xC0
#define MIDI_HDR_END                            0xFF

/* ---- ENV_PHASE (PSG envelope state machine) ------------------------- */
#define ENV_IDLE                                0
#define ENV_ATTACK                              1
#define ENV_DECAY                               2
#define ENV_SUSTAIN                             3
#define ENV_RELEASE                             4
#define ENV_FADEOUT                             5

/* ---- XBIOS Midiws (function 12): send raw MIDI bytes to the MIDI OUT port. */
#define XBIOS_Midiws                            12
#define XBIOS_Dosound                           32      /* run PSG sequence  */

#define COLOR_dk_brown                          15

/* ---- VDI raster op modes ---------------------------------------------
   Source/destination combining modes for vro_cpyfm.  Names match the
   GEM VDI header. */
#define ALL_WHITE                       0       /* dest set to all 1s     */
#define S_ONLY                          3       /* replace dest w/ source */

/* Card game constants -- CARD_TYPE values 0..51 are the 52 face cards
   (index into crd_mfdb).  CARD_BACK selects the shared face-down back
   MFDB.  CARD_NONE is the sentinel used by war/blackjack to mark
   empty slots in the war-cards arrays and to signal end-of-hand from
   pk_rmch when the source pile is empty. */
#define CARD_BACK                       52
#define CARD_NONE                       0xff
/* Card empty background: the 53rd MFDB slot, an all-background
   coloured card used to clear a slot when the player selects a
   card for discard (shown while the replacement is animating in). */
#define CARD_HIGHLIGHT                  53

/* Blackjack hit-counter constants.  Ghidra shows these as
   CARD_HEART_10 / _QUEEN / _KING because the 1985 source aliased
   three unrelated ROM constants onto card-name symbols; the values
   encode the "at most 5 total cards, so at most 3 hits per hand"
   rule.  Verified against the ORIGINAL disassembly:
      poker_blackjack_main  0x1c492 / 0x1c49a
        move.w #0x3, (0x501a6)  ; poker_player_card_count = 3
        move.w #0x3, (0x50240)  ; poker_player_split_card_count = 3
      poker_blackjack_round 0x1d3be / 0x1d51c
        subq.w #0x1, (A1)         ; card_count -= 1
      poker_blackjack_round 0x1d5e4
        tst.w (A0); bne 0x1d600   ; loop while != 0
   Confirms MAX=3, STEP=1, STOP=0 as ported. */
#define CARD_BJ_MAX                     3       /* Ghidra: CARD_HEART_10 */
#define CARD_BJ_STEP                    1       /* Ghidra: CARD_HEART_QUEEN */
#define CARD_BJ_STOP                    0       /* Ghidra: CARD_HEART_KING */
#define NOTS_AND_D                      4       /* (NOT src) AND dest    */
#define S_XOR_D                         6       /* source XOR dest       */

/* ---- VDI fill styles ------------------------------------------------
   Match Ghidra's vdi_erase_screen at 0x166fe / screen_set_draw_to_backbuffer:
     vsf_interior(vdihnd, 2)   -- interior = PATTERN
     vsf_style(vdihnd, 8)   -- pattern index 8 (renders solid at slot 0)
   Numeric values must match the ROM byte-for-byte. */
#define FILL_SOLID                      8
#define VSFPATT                         2

/* Extra HOUSE_POS used by the dog wander logic. */
#define POS_BTM_STAIR_LANDING           32      /* Ghidra: 0x20 */

/* ---- Door / furniture state bitfield in lcp.door_states_and_flags ---- */
#define DSF_FRONT_DOOR                  0x001
#define DSF_STUDY_DOOR                  0x002
#define DSF_CLOSET_DOOR                 0x004
#define DSF_KITCHEN_CABINET             0x008
#define DSF_DRESSER                     0x010
#define DSF_TOILET_DOOR                 0x020
#define DSF_FILING_CABINET              0x040
#define DSF_DOG_BOWL_MASK               0x180
#define DSF_FOOD_COUNT_MASK             0xE00
#define DSF_FOOD_MASK                   0xE00
#define DSF_PRESERVE_UPPER_MASK         0xFE00

/* ---- GEMDOS trap numbers (subset used by save.c) --------------------- */
#define GEMDOS_Fopen                    0x3D
#define GEMDOS_Fclose                   0x3E
#define GEMDOS_Fread                    0x3F
#define GEMDOS_Fwrite                   0x40
#define GEMDOS_Fcreate                  0x3C
#define GEMDOS_Fsfirst                  0x4E
#define GEMDOS_Fsnext                   0x4F
#define GEMDOS_Dsetpath                 0x3B    /* set current directory */
#define BIOS_Setexc                     5       /* install exception vector */
#define XBIOS_Xbtimer                   31      /* install MFP timer */
#define GEMDOS_Fgetdta                  0x2F
#define GEMDOS_Malloc                   0x48
#define GEMDOS_Mfree                    0x49
#define GEMDOS_Cconis                   0x0B    /* console status */
#define GEMDOS_Crawcin                  0x07    /* raw char input */
#define GEMDOS_Super                    0x20    /* supervisor mode */
#define XBIOS_Vsync                     37      /* wait for vertical retrace */

/* ---- Keyboard scancodes / Ctrl combos --------------------------------
   The 1985 code uses a keycode_enum where Ctrl+X maps to X-'@' (i.e.
   Ctrl+A=1, Ctrl+B=2, ...).  The Ghidra decompile uses names like
   keycode_enum_ctrl_a_alarm; we split the semantic (what the game
   does with it) from the raw scan value. */
/* KEY_NONE (-1) is used to signal "nothing in the buffer".  Extended
   keys (function + cursor) live above the 0..0xff ASCII range at
   0x100 | scancode -- keeps them within positive-short territory. */
#define KEY_NONE                        (-1)
#define KEY_CURSOR_LEFT                 0x14B
#define KEY_F1                          0x13B
#define KEY_F2                          0x13C
#define KEY_F3                          0x13D
#define KEY_F4                          0x13E
#define KEY_F5                          0x13F
#define KEY_F6                          0x140
#define KEY_F7                          0x141
#define KEY_F8                          0x142
#define KEY_F9                          0x143
#define KEY_F10                         0x144
#define KEY_CTRL_A_ALARM                0x01
#define KEY_CTRL_B_BOOK                 0x02
#define KEY_CTRL_C_CALL                 0x03
#define KEY_CTRL_D_DOGFOOD              0x04
#define KEY_CTRL_F_FOOD                 0x06
#define KEY_CTRL_M                      0x0D    /* Enter */
#define KEY_CTRL_P_PATTING              0x10
#define KEY_CTRL_R_RECORD               0x12
#define KEY_CTRL_W_WATER                0x17

/* ---- SPRITE_ID (subset for dog) --------------------------------------- */
#define SPRITE_DOG_LAY_DOWN             0x21
#define SPRITE_DOG_WALK_RIGHT_9         0x2a

/* ---- ACTION_ID (dumped verbatim from Ghidra) --------------------------
   The 5 EVENT actions (28..32) are INTERLEAVED with the regular actions
   in the original binary, not appended at the end.  ACTION_NONE (-1)
   is the empty sentinel used by g_trac and the event FIFO. */
#define ACTION_NONE                     (-1)
#define ACTION_SIT_AND_EXERCISE          0
#define ACTION_READ_NEWSPAPER            1
#define ACTION_PLAY_COMPUTER             2
#define ACTION_WASH_HANDS                3
#define ACTION_GET_IN_OUT_OF_BED         4
#define ACTION_LISTEN_SONG               5
#define ACTION_PLAY_PIANO                6
#define ACTION_WRITE_LETTER              7
#define ACTION_DANCE                     8
#define ACTION_YAWN_AND_STRETCH          9
#define ACTION_PACE_NERVOUSLY           10
#define ACTION_WANDER_IDLY              11
#define ACTION_SLEEP                    12
#define ACTION_DRINK                    13
#define ACTION_NOD_HEAD                 14
#define ACTION_PEEK_AROUND              15
#define ACTION_PLAY_A_GAME              16
#define ACTION_BRUSH_TEETH              17
#define ACTION_KITCHEN_CABINET          18
#define ACTION_SIT_ON_COUCH_WITH_DOG    19
#define ACTION_LIGHT_FIREPLACE          20
#define ACTION_USE_TOILET               21
#define ACTION_TAKE_SHOWER              22
#define ACTION_FEED_DOG                 23
#define ACTION_HELLO                    24
#define ACTION_EAT_MEAL                 25
#define ACTION_PLAY_WITH_RECORD         26
#define ACTION_OPEN_UPSTAIRS_CLOSET     27
#define ACTION_EVENT_RECORD_DELIVERY    28
#define ACTION_EVENT_FOOD_DELIVERY      29
#define ACTION_EVENT_PHONE_CALL         30
#define ACTION_EVENT_DOG_FOOD           31
#define ACTION_EVENT_BOOK_DELIVERY      32
#define ACTION_GET_SNACK_FROM_FRIDGE    33
#define ACTION_OPEN_BEDROOM_CLOSET      34
#define ACTION_GET_DRESSED              35
#define ACTION_CLEAN_UP                 36
#define ACTION_TIDY_HOUSE               37
#define ACTION_CHECK_FRONT_DOOR         38
#define ACTION_TOGGLE_TV                39
#define ACTION_CALL_DOG                 40
#define ACTION_WAKE_FROM_ALARM          41
#define ACTION_PET_DOG                  42
#define ACTION_WAKE_UP_MORNING          43
#define ACTION_GO_TO_BED_NIGHT          44

/* ---- Word IDs (subset used by the parser) ----------------------------
   WORD_NONE (-1) marks "not in dictionary".  The rest are populated
   at runtime from the vocabulary table -- add named constants here as
   specific words become referenced by name in other .c files. */
#define WORD_NONE                       (-1)
#define WORD_PLEASE                     1

#endif  /* ENUMS_H */
