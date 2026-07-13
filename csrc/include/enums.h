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

/* ---- PLAYER_STATE (subset ported so far) ------------------------------
   The 0..24 block covers all walk / stair states.  Layout (verified by
   the wrap tests in lcp_pathfind_one_step):
     0        STATE_STAND_SIDE_VIEW / IDLE (== facing-right stand)
     1..7     Walking (frame 3 and 7 trigger footstep)
     9..12    Stair climb, mid-stair (frame 12 = STATE_STAIR_CLIMB_FRAME_3_STEP)
     13..16   Stair top-of-stair entry animation
     17..20   Stair descend
     21..24   Stair bottom-of-stair animation
   Beyond 24 are single-action poses (yawn, phone, read newspaper, etc). */
#define STATE_STAND_IDLE                        0
#define STATE_STAND_SIDE_VIEW                   0
#define STATE_WALK_FRAME_0                      0
#define STATE_WALK_FRAME_1                      1
#define STATE_WALK_FRAME_3_STEP                 3
#define STATE_WALK_FRAME_4                      4
#define STATE_WALK_FRAME_5                      5
#define STATE_WALK_FRAME_7_STEP                 7
#define STATE_STAIR_CLIMB_FRAME_0               9
#define STATE_STAIR_CLIMB_FRAME_3_STEP          12
#define STATE_STAIR_TOP_FRAME_0                 13
#define STATE_STAIR_TOP_FRAME_3_STEP            16
#define STATE_STAIR_DESCEND_FRAME_0             17
#define STATE_STAIR_DESCEND_FRAME_1             18
#define STATE_STAIR_DESCEND_FRAME_3_STEP        20
#define STATE_STAIR_BTM_FRAME_0                 21
#define STATE_STAIR_BTM_FRAME_1                 22
#define STATE_STAIR_BTM_FRAME_2                 23
#define STATE_STAIR_BTM_FRAME_3                 24
#define STATE_YAWN_MOUTH_OPEN                   17      /* aliased with descend */
#define STATE_YAWN_STRETCH_ARMS                 18      /* aliased with descend */
#define STATE_STAND_FACING_SCREEN               28
#define STATE_BEND_DOWN                         29
#define STATE_REACH_FORWARD                     30
#define STATE_BEND_AND_REACH                    31
#define STATE_REACH_INTO_CABINET                32
#define STATE_CROUCH_DOWN                       34
#define STATE_EXERCISE_CROUCH                   35
#define STATE_READ_PAPER_HOLD                   40
#define STATE_READ_PAPER_TURN_PAGE              41
#define STATE_PHONE_PICKUP                      74
#define STATE_PHONE_TALKING                     75

/* Action-specific states (26+ block).  Values are internally consistent
   with the body_sprite_frame_table lookup but not visible outside the
   dispatchers that reference them. */
#define STATE_SIT_IN_ARMCHAIR                   50
#define STATE_DRINK_FROM_GLASS                  51
#define STATE_IDLE_SHRUG_START                  52
#define STATE_IDLE_SHRUG_HOLD                   53
#define STATE_PACE_SHIFT_LEFT                   54
#define STATE_PACE_SHIFT_RIGHT                  55
#define STATE_SLEEP_BREATHE_IN                  56
#define STATE_SLEEP_BREATHE_OUT                 57
#define STATE_UNDRESS_AT_BED                    58
#define STATE_LIE_DOWN_GETTING_IN               59
#define STATE_LIE_DOWN_IN_BED                   60
#define STATE_DANCE_STEP_LEFT                   61
#define STATE_DANCE_STEP_RIGHT                  62
#define STATE_EAT_BITE                          63
#define STATE_EAT_CHEW                          64
#define STATE_SHOWER_STAND                      65
#define STATE_SHOWER_SCRUB_LEFT                 66
#define STATE_SHOWER_SCRUB_RIGHT                67
#define STATE_SHOWER_WASH_LEFT                  68
#define STATE_SHOWER_WASH_RIGHT                 69
#define STATE_BRUSH_TEETH                       70
#define STATE_WASH_HANDS_CENTER                 71
#define STATE_WASH_HANDS_LEFT                   72
#define STATE_WASH_HANDS_RIGHT                  73
#define STATE_BROWSE_VINYL_REACH_RIGHT          76
#define STATE_BROWSE_VINYL_IDLE                 77
#define STATE_BROWSE_VINYL_REACH_LEFT           78
#define STATE_BROWSE_VINYL_PULL_OUT             79
#define STATE_SIT_COUCH_UPRIGHT                 80
#define STATE_SIT_COUCH_PETTING_DOG             81
#define STATE_EXERCISE_ARMS_CENTER              82
#define STATE_EXERCISE_ARMS_UP                  83
#define STATE_EXERCISE_ARMS_WIDE                84
#define STATE_STOKE_FIREPLACE                   85
#define STATE_WRITE_AT_DESK                     86
#define STATE_TYPE_AT_DESK_LEFT_HAND            87
#define STATE_TYPE_AT_DESK_RIGHT_HAND           88
#define STATE_PICK_UP_FROM_FLOOR                89
#define STATE_TYPING_HANDS_DOWN                 90
#define STATE_TYPING_HANDS_UP                   91
#define STATE_SITTING_AT_DESK                   92
#define STATE_SIT_AT_DESK                       92

/* ---- Head animation modes (extra) ------------------------------------ */
#define HEAD_ANIM_COMPUTER                      5

/* ---- Text color (VDI palette index) ------------------------------------ */
#define COLOR_black                             1
#define COLOR_white                             0

/* Sat/Sun weekday consts (used by check_time_based_actions). */
#define NEED_SATISFIED                          0

/* ---- Head animation modes / target ------------------------------------ */
#define HEAD_ANIM_DISABLED              0
#define HEAD_ANIM_HORIZONTAL_RANGE      12
#define HEAD_ANIM_WALKING               1
#define HEAD_ANIM_SHOWER                2
#define HEAD_ANIM_READING               4

/* ---- HOUSE_POS (subset) ----------------------------------------------- */
#define POS_MID_BEDROOM_WALK            17
#define POS_TOP_STUDY_DOOR              7
#define POS_TOP_DANCE_FLOOR             14
#define POS_MID_BATHROOM_ENTRANCE       25
#define POS_BTM_KITCHEN_CABINET         34
#define POS_BTM_DOG_FOOD                43
#define POS_BTM_FRONT_DOOR              46
#define POS_TOP_ARMCHAIR                2
#define POS_MID_BED                     18
#define POS_MID_TOILET_DOOR             22
#define POS_BTM_KITCHEN_SINK            36
#define POS_BTM_WATER_TAP               35
#define POS_BTM_STOVE                   33
#define POS_BTM_TABLE_LEFT              37
#define POS_BTM_TABLE_RIGHT             38
#define POS_BTM_FRIDGE                  32
#define POS_BTM_DOG_BOWL                42
#define POS_MID_SHOWER_DOOR             20
#define POS_MID_SHOWER_INSIDE           21
#define POS_MID_BATHROOM_SINK           24
#define POS_TOP_FILING_CABINET          12
#define POS_TOP_RECORD_SHELF            13
#define POS_MID_COUCH                   19
#define POS_MID_DRESSER                 27
#define POS_MID_BEDROOM_CLOSET          28
#define POS_BTM_FIREPLACE_LOGS          45
#define POS_TOP_DESK_CHAIR              10
#define POS_TOP_LIVING_ROOM             0
#define POS_MID_COMPUTER_DESK           29

/* ---- SPRITE_ID (study doors + carried objects) ------------------------ */
#define SPRITE_DOOR_STUDY_1             0x18
#define SPRITE_DOOR_STUDY_AJAR          0x19
#define SPRITE_DOOR_STUDY_WIDE_OPEN     0x1a
#define SPRITE_FOOD_PACKAGE             9
#define SPRITE_BOOK                     0x31
#define SPRITE_VINYL_CARRY              0x32
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

/* ---- Sound-effect IDs (subset) ---------------------------------------- */
#define SFX_DOOR_OPEN                   3
#define SFX_DOOR_CLOSE                  4
#define SFX_ALARM_CLOCK                 5
#define SFX_PHONE_RING                  21
#define SFX_FOOTSTEP_CARPET             10
#define SFX_FOOTSTEP_WOOD               11
#define SFX_FOOTSTEP_STAIRS             12
#define SFX_SNORING                     14
#define SFX_WATER_RUNNING               15
#define SFX_TOILET_FLUSH                16
#define SFX_TOILET_REFILL               17
#define SFX_TV_CLICK                    18
#define SFX_CLICK                       10
#define SFX_TYPEWRITER_KEY              11

/* ---- Palette values (12-bit RGB, Atari ST format) --------------------- */
#define ST_PEACH                        0x743
#define ST_SICK_GREEN                   0x363

/* ---- VDI mode ------------------------------------------------------- */
#define MD_TRANS                        2
#define MD_REPLACE                      1

/* ---- MIDI sequencer phase ------------------------------------------- */
#define SEQ_PHASE_IDLE                          0
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

/* ---- Additional colours used by td_nois --------------- */
#define COLOR_dk_brown                  15
#define COLOR_red                       2
#define COLOR_grey                      8

/* ---- VDI raster op modes ---------------------------------------------
   Source/destination combining modes for vro_cpyfm.  Names match the
   GEM VDI header. */
#define S_ONLY                          3       /* replace dest w/ source */
#define NOTS_AND_D                      4       /* (NOT src) AND dest    */
#define S_XOR_D                         6       /* source XOR dest       */

/* ---- VDI fill styles ------------------------------------------------ */
#define FILL_SOLID                      1
#define vsf_interior_PATTERN            1       /* VDI fill interior=pattern */

/* ---- More SFX IDs --------------------------------------------------- */
#define SFX_GREETING                    9
#define SFX_SPEECH                      7
#define SFX_HEAD_NOD                    8
#define SFX_WATER_TAP                   13
#define SFX_DOORBELL                    19
#define SFX_DOORBELL_ECHO               20

/* Extra HOUSE_POS used by the dog wander logic. */
#define POS_BTM_STAIR_LANDING           40

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

/* ---- Action / event IDs -----------------------------------------------
   Verified against Ghidra ACTION_ID enum.  0..37 are player actions
   dispatched by do_action(); 38..44 are deferred events dispatched by
   execute_event().  ACTION_NONE (-1) is the empty sentinel used by the
   trigger_action global and the event FIFO. */
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
#define ACTION_GET_SNACK_FROM_FRIDGE    28
#define ACTION_OPEN_BEDROOM_CLOSET      29
#define ACTION_GET_DRESSED              30
#define ACTION_CLEAN_UP                 31
#define ACTION_TIDY_HOUSE               32
#define ACTION_CHECK_FRONT_DOOR         33
#define ACTION_TOGGLE_TV                34
#define ACTION_CALL_DOG                 35
#define ACTION_WAKE_FROM_ALARM          36
#define ACTION_PET_DOG                  37
#define ACTION_WAKE_UP_MORNING          38
#define ACTION_GO_TO_BED_NIGHT          39
#define ACTION_EVENT_PHONE_CALL         40
#define ACTION_EVENT_DOG_FOOD           41
#define ACTION_EVENT_BOOK_DELIVERY      42
#define ACTION_EVENT_RECORD_DELIVERY    43
#define ACTION_EVENT_FOOD_DELIVERY      44

/* ---- Word IDs (subset used by the parser) ----------------------------
   WORD_NONE (-1) marks "not in dictionary".  The rest are populated
   at runtime from the vocabulary table -- add named constants here as
   specific words become referenced by name in other .c files. */
#define WORD_NONE                       (-1)
#define WORD_PLEASE                     1

#endif  /* ENUMS_H */
