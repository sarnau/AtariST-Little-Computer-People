/*
 * enums.h -- symbolic constants.  Alcyon C has no enum, so every value
 * is a #define.  Some groups list every value of a table or file (all
 * animation states, house positions, objects, sounds, words) even where
 * the code names only a few of them: they document the data.
 */

#ifndef ENUMS_H
#define ENUMS_H

/* ---- Need levels: resident.thirst_level / resident.hunger_level ----------------
   A need timer that runs out at NEED_SEVERE makes the resident sick
   instead of raising the level further. */
#define NEED_SATISFIED          0
#define NEED_MILD               1
#define NEED_MODERATE           2
#define NEED_SEVERE             3

/* ---- Sickness: resident.sickness_level, and the direction it moves in --------
   (sickness_direction / happiness_direction add one of DIR_* per step). */
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

/* ---- Floors -- what floorOfY() returns, counted from the ground ------- */
#define FLOOR_BOTTOM            1       /* y > 140 */
#define FLOOR_MIDDLE            2       /* y > 77  */
#define FLOOR_TOP               3

/* ---- Open/close argument of openFrontDoor / openKitchenCab / openDresser --------------
   (front door, kitchen cabinet, dresser).  Each tests `== 0` for open
   and re-tests `!= 0` for close, so any non-zero value closes. */
#define DOOR_OPEN               0
#define DOOR_CLOSE              1

/* dozeOff(SLEEP_RANDOM) walks to the floor's centre line and sleeps
   rndRng(7, 15) rounds; any other argument is the round count. */
#define SLEEP_RANDOM            (-1)

/* ---- Sprite layers ---------------------------------------------------- */
#define SPRITE_HIDDEN           0
#define SPRITE_BEHIND_LCP       (-1)
#define SPRITE_IN_FRONT         1

/* ---- Resident animation states (animState) -----------------------------
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
/* cp68's macro-name lookup table truncates identifiers to 22 chars.
   STATE_CARRY_WALK_FRAME_0 and _1 both truncate to STATE_CARRY_WALK_FRAME
   and silently collide (cp68 emits `redefining STATE_CARRY_WALK_FRAME`).
   Use the shorter F0 / F1 convention matching STATE_STR_CLIMB_F0 etc. */
#define STATE_CARRY_WALK_F0                     36
#define STATE_CARRY_WALK_F1                     37
#define STATE_ORGAN_IDLE                38
#define STATE_ORGAN_REACH_L             39
#define STATE_ORGAN_REACH_R             40
#define STATE_ORGAN_PULL_OUT            41
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

/* ---- Colours ---------------------------------------------------------
   Values 0..15 are colour indices, NOT VDI palette slots.  The
   drawing calls pass a colour index through colorPens[] to get the
   underlying VDI palette slot -- that table is a permutation, so
   using the wrong colour index here produces the wrong on-screen hue. */
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
#define COLOR_dk_brown                          15

/* ---- Head frames in a PEx.LCP file ---------------------------------
   A PEx file holds three blocks of HEAD_FRAMES_PER_MOOD frames, in the
   order content, sad, happy (moodHeadBase).  In each block, frames 0..6
   are special poses (eating uses 0..2, peeking 2, hello and the phone
   4..6) and frames 7..21 are a grid of three tilt rows by five turn
   directions -- 0 the face seen from the front, through to 4 the back
   of the head; directions 5..7 reuse 3..1 mirrored.  Each lower row
   tilts the head further down. */
#define HEAD_FRAMES_PER_MOOD            22
#define HEAD_TURN_FRAMES                5
#define HEAD_FRAMES                     (3 * HEAD_FRAMES_PER_MOOD)  /* 66, a PEx.LCP */
/* BODY.LCP holds BODY_FRAMES body frames (the highest any table uses is
   97); bodyFrames is declared with BODY_FRAME_SLOTS, the original's
   size, which the BSS layout depends on. */
#define BODY_FRAMES                     98
#define BODY_FRAME_SLOTS                120
#define HEAD_ROW_LEVEL                  7       /* first frame of each tilt row */
#define HEAD_ROW_LOWER                  (HEAD_ROW_LEVEL + HEAD_TURN_FRAMES)
#define HEAD_ROW_LOWEST                 (HEAD_ROW_LOWER + HEAD_TURN_FRAMES)
/* A head pose (headPose, headTarget, headLastWalk) packs a turn
   direction in bits 0..2 and a tilt row in bits 3..4. */
#define HEAD_DIR_FRONT                  0       /* face seen from the front */
#define HEAD_DIR_FRONT_RIGHT            1
#define HEAD_DIR_RIGHT                  2       /* right profile */
#define HEAD_DIR_BACK_RIGHT             3
#define HEAD_DIR_BACK                   4       /* back of the head */
#define HEAD_DIR_BACK_LEFT              5       /* 5..7: 3..1 mirrored */
#define HEAD_DIR_LEFT                   6
#define HEAD_DIR_FRONT_LEFT             7
#define HEAD_DIRS                       8
#define HEAD_DIR_MASK                   7
#define HEAD_TILT_LEVEL                 0
#define HEAD_TILT_LOWER                 1
#define HEAD_TILT_LOWEST                2
#define HEAD_TILT_SHIFT                 3
#define HEAD_TILT_MASK                  0x18
#define HEAD_POSE(dir, tilt)            ((tilt) << HEAD_TILT_SHIFT | (dir))

/* headTurnStep's marker for "no direct step from this direction to the
   target"; stepHead then steps toward the state's rest direction. */
#define HEAD_TURN_NONE                  99

/* ---- Head animation modes (headMode, headTarget) ------------------------
   Bit fields inside headMode:
     bits 0..2   HEAD_ANIM_HORIZONTAL_AMPLITUDE (mask 0x03 in binary, but
                 the enum encodes it as value 3 for the "amplitude enabled"
                 marker; stepHead masks with HEAD_MODE_H_AMPLITUDE = 0x07)
     bit 3       HEAD_ANIM_HORIZONTAL_RANGE (0x08) or 0xC (see stepHead)
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

/* ---- House positions: the index posToXY looks up --------------------- */
#define POS_PER_FLOOR                   16      /* top 0..15, middle 16..31, bottom 32..47 */
#define POS_TOP_LIVING_ROOM              0
#define POS_TOP_DANCE_FLOOR              1
#define POS_TOP_ARMCHAIR                 2
#define POS_TOP_GAME_TABLE               3
#define POS_TOP_GAME_CHAIR_LEFT          4
#define POS_TOP_GAME_CHAIR_RIGHT         5
#define POS_TOP_ORGAN                   6
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
#define POS_BTM_STAIR_LANDING           32
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
#define POS_BTM_COUCH                   43
#define POS_BTM_DOG_FOOD_STORE          44
#define POS_BTM_FIREPLACE_LOGS          45
#define POS_BTM_FRONT_DOOR              46
#define POS_BTM_SCREEN_EDGE             47

/* ---- Sprite ids (study doors and carried objects) --------------------- */
/* Logical sprite-def IDs 0 and 1 are the LCP body and head sprites --
   pinned to hardware slots HW_SLOT_LCP_BODY / HW_SLOT_LCP_HEAD in
   spriteSlot[] at boot.  Distinct from the sprite-layer values
   SPRITE_HIDDEN=0 / SPRITE_IN_FRONT=1 above (different domain: layer
   flags for spriteLayer[] vs sprite-def indices for spriteBitmap[]/spriteSlot[]). */
#define SPRITE_LCP_BODY_ID              0x00
#define SPRITE_LCP_HEAD_ID              0x01
#define SPRITE_GLASS                    0x03
#define SPRITE_GAME_BOX                 0x04       /* also mini-game box */
#define SPRITE_STUDY_DOOR_FRAME         0x06       /* also used as toothbrush */
#define SPRITE_ORGAN_PROP               0x07
#define SPRITE_TYPEWRITER               0x08
#define SPRITE_FOOD_PACKAGE             0x09
#define SPRITE_TABLE_SETTING            0x0c
#define SPRITE_DOOR_ANIM_1              0x0d
#define SPRITE_DOOR_ANIM_2              0x0e
#define SPRITE_DOOR_ANIM_3              0x0f
#define SPRITE_CLOSET_LCP_INSIDE        0x10
#define SPRITE_CLOSET_WIDE_OPEN         0x12
#define SPRITE_CLOSET_AJAR              0x11
#define SPRITE_DOG_SIT                  0x15
#define SPRITE_FIREWOOD                 0x16
#define SPRITE_COOKING_POT              0x17
#define SPRITE_DOOR_STUDY_1             0x18
#define SPRITE_DOOR_STUDY_AJAR          0x19
#define SPRITE_DOOR_STUDY_WIDE_OPEN     0x1a
/* Ctrl-P petting-hand animation frames, cycled through patSprites
   (dat_anim.c).  The table has PAT_FRAMES entries, but gameTick steps
   patFrame from 0 up to PAT_FRAMES inclusive: the last frame reads one
   past the table, into patLastSprite (SPRITE_PET_HAND_1).  That is the
   original's behaviour and what ends the cycle on hand 1. */
#define PAT_FRAMES                      10
#define SPRITE_PET_HAND_1               0x1b
#define SPRITE_PET_HAND_2               0x1c
#define SPRITE_PET_HAND_3               0x1d
#define SPRITE_PET_HAND_4               0x1e
#define SPRITE_PET_HAND_5               0x1f
#define SPRITE_PET_HAND_6               0x20
#define SPRITE_DOG_LAY_DOWN             0x21
/* cp68 truncates identifiers to 22 chars.  SPRITE_DOG_WALK_RIGHT_N is
   23 chars and all 9 collide on truncation -- use SPRITE_DOG_WLK_RN.
   The names skip _R6 (frames are 1..5, 7..9) -- the walk cycle is 8
   frames but the naming is not sequential.  Last
   walk frame is _R9 at 0x29; 0x2a is SPRITE_DOG_EATING_1. */
#define SPRITE_DOG_WLK_R1               0x22
#define SPRITE_DOG_WLK_R2               0x23
#define SPRITE_DOG_WLK_R3               0x24
#define SPRITE_DOG_WLK_R4               0x25
#define SPRITE_DOG_WLK_R5               0x26
#define SPRITE_DOG_WLK_R7               0x27
#define SPRITE_DOG_WLK_R8               0x28
#define SPRITE_DOG_WLK_R9               0x29
#define SPRITE_DOG_EATING_1             0x2a
#define SPRITE_DOG_EATING_2             0x2b
#define SPRITE_DOG_EATING_3             0x2c
#define SPRITE_READING_1                0x2d
#define SPRITE_READING_2                0x2e
#define SPRITE_READING_3                0x2f
#define SPRITE_SUITCASE                 0x30   /* carried in moveInScene */
#define SPRITE_BOOK                     0x31
#define SPRITE_VINYL_CARRY              0x32
#define SPRITE_TYPING_1                 0x33
#define SPRITE_TYPING_2                 0x34
#define SPRITE_TYPING_3                 0x35
#define SPRITE_TYPING_4                 0x36
#define SPRITE_COOKED_MEAL              0x37   /* carried stove -> cabinet after cooking */

/* ---- Object frames: indices into the OBJECTS table for drawObject() ------
   The fixed compile-time indices passed as the first argument to
   drawObject().  cp68's 22-char macro-name limit forces the short OBJ_
   prefix. */
#define OBJ_FILING_CABINET_CLOSED               0
#define OBJ_FILING_CAB_OPEN_1                   1       /* filing_cabinet_open_1 */
#define OBJ_FILING_CAB_OPEN_2                   2       /* filing_cabinet_open_2 */
#define OBJ_ALARM_1                             3
#define OBJ_ALARM_2                             4
#define OBJ_STOVE_1                             5
#define OBJ_STOVE_2                             6
#define OBJ_STOVE_3                             7
#define OBJ_STOVE_4                             8
#define OBJ_STOVE_5                             9
#define OBJ_DRESSER_CLOSED                     10
#define OBJ_DRESSER_OPEN_1                     11
#define OBJ_DRESSER_OPEN_2                     12
#define OBJ_CLOCK_1                            13
#define OBJ_CLOCK_2                            14
#define OBJ_CLOCK_3                            15
#define OBJ_FRIDGE_CLOSED                      16
#define OBJ_FRIDGE_OPEN_1                      17
#define OBJ_FRIDGE_OPEN_2                      18
#define OBJ_CABINET_CLOSED                     19
#define OBJ_CABINET_OPEN_1                     20
#define OBJ_CABINET_OPEN_2                     21
#define OBJ_PHONE_1                            22
#define OBJ_PHONE_2                            23
#define OBJ_PHONE_3                            24
#define OBJ_DOOR_TOILET_CLOSED                 25
#define OBJ_DOOR_TOILET_OPEN_1                 26
#define OBJ_DOOR_TOILET_OPEN_2                 27
#define OBJ_DOOR_CLOSET_CLOSED                 28
#define OBJ_DOOR_CLOSET_OPEN_1                 29
#define OBJ_DOOR_CLOSET_OPEN_2                 30
#define OBJ_FIRE_OFF                           31
#define OBJ_FIRE_1                             32
#define OBJ_FIRE_2                             33
#define OBJ_FIRE_3                             34
#define OBJ_FIRE_4                             35
#define OBJ_DOOR_FRONT_CLOSED                  36
#define OBJ_DOOR_FRONT_OPEN_1                  37
#define OBJ_DOOR_FRONT_OPEN_2                  38
#define OBJ_MEDICINE_CLOSED                    39
#define OBJ_MEDICINE_OPEN_1                    40
#define OBJ_MEDICINE_OPEN_2                    41
#define OBJ_STOVE_OFF                          42
#define OBJ_STOVE_ON_1                         43
#define OBJ_STOVE_ON_2                         44
#define OBJ_STOVE_ON_3                         45
#define OBJ_DOOR_STUDY_CLOSED                  46
#define OBJ_DOOR_STUDY_OPEN_1                  47
#define OBJ_DOOR_STUDY_OPEN_2                  48
#define OBJ_DOG_FOOD_BOWL_1                    49
#define OBJ_DOG_FOOD_BOWL_2                    50
#define OBJ_DOG_FOOD_BOWL_3                    51
#define OBJ_PHONE_CALL                         52
#define OBJ_CABINET_ITEM                       53   /* food-pip in open kitchen cabinet */
#define OBJ_WHITE_BLUE                         54
#define OBJ_TYPEWRITER                         55

/* ---- Minigame key menu -----------------------------------------------
   The card games list their function-key choices in a panel on the
   right of the screen, one prompt per line (F1, F3, then F5 or F10),
   and wipe the panel with panelErase before showing the next set. */
#define KEYMENU_X                       225
#define KEYMENU_LINE1                   18
#define KEYMENU_LINE2                   26
#define KEYMENU_LINE3                   34
#define KEYMENU_TOP                     10
#define KEYMENU_RIGHT                   319
#define KEYMENU_BOTTOM                  60

/* ---- Furniture screen positions -------------------------------------
   Where drawObject paints each piece of furniture (the top-left corner of
   its object frame).  Every open/closed frame of one piece is drawn at
   the same spot, and the study door's sprites sit there too. */
#define CLOSET_DOOR_X           75
#define CLOSET_DOOR_Y           87
#define DOG_BOWL_X              8
#define DOG_BOWL_Y              190
#define DRESSER_X               97
#define DRESSER_Y               115
#define FILING_CAB_X            258
#define FILING_CAB_Y            47
#define FIREPLACE_X             257
#define FIREPLACE_Y             170
#define FRIDGE_X                24
#define FRIDGE_Y                153
#define FRONT_DOOR_X            294
#define FRONT_DOOR_Y            151
#define KITCHEN_CAB_X           46
#define KITCHEN_CAB_Y           140
#define PHONE_X                 190
#define PHONE_Y                 168
#define STOVE_X                 6
#define STOVE_Y                 172
#define STUDY_DOOR_X            178
#define STUDY_DOOR_Y            23
#define TOILET_DOOR_X           187
#define TOILET_DOOR_Y           87

/* ---- Dog bowl state --------------------------------------------------- */
#define BOWL_EMPTY                      0
#define BOWL_HALF                       1
#define BOWL_FULL                       2

/* ---- Sound-effect IDs (block index in SOUNDS.LCP) -------------------- */
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
/* Skin tone used by setSkinColor: normal and sick. */
#define ST_PEACH                        0x754
#define ST_SICK_GREEN                   0x453

/* ---- MIDI sequencer phase ------------------------------------------- */
#define SEQ_PHASE_IDLE                          0
#define SEQ_PHASE_WAIT_NOTE_EXPIRE              0
#define SEQ_PHASE_PARSE_NEXT_EVENT              1
#define SEQ_PHASE_SONG_ENDING                   2

/* ---- Song header command bytes -------------------------------------- */
#define MIDI_HDR_SET_KEY                        0x80    /* key signature -> songKey, buildNoteMap */
#define MIDI_HDR_SET_TEMPO                      0x81
#define MIDI_HDR_SET_VOLUME                     0x83
#define MIDI_HDR_SET_VELOCITY                   0x84    /* default velocity -> defVelocity, defPsgVol */
#define MIDI_HDR_PROGRAM_CHANGE                 0xC0
#define MIDI_HDR_END                            0xFF

/* ---- Song body control bytes (parseEvents's switch) -------------------- */
#define SEQ_BAR                                 0x82
#define SEQ_LOOP_START                          0x85    /* + repeat count */
#define SEQ_LOOP_END                            0x86
#define SEQ_END                                 0xFF

/* ---- YM2149 PSG registers ------------------------------------------
   Giaccess(data, reg | PSG_WRITE) writes, Giaccess(0, reg) reads.
   Note psgWrite(data, reg) takes the REGISTER second, like Giaccess.
   Tone period for channel n is registers 2n (fine) / 2n+1 (coarse). */
#define XBIOS_GIACCESS                          28
#define PSG_WRITE                               0x80
#define PSG_NOISE_PERIOD                        6
#define PSG_MIXER                               7
#define PSG_VOL_A                               8
#define PSG_VOL_B                               9
#define PSG_VOL_C                               10

/* ---- PSG envelope phases (stepEnvelopes's state machine) ----------------- */
#define ENV_IDLE                                0
#define ENV_ATTACK                              1
#define ENV_DECAY                               2
#define ENV_SUSTAIN                             3
#define ENV_RELEASE                             4
#define ENV_FADEOUT                             5

/* Card game constants -- CARD_TYPE values 0..51 are the 52 face cards
   (index into cardMfdb).  CARD_BACK selects the shared face-down back
   MFDB.  CARD_NONE is the sentinel used by war/blackjack to mark
   empty slots in the war-cards arrays and to signal end-of-hand from
   popCard when the source pile is empty.

   card / 13 is the suit (Hearts, Spades, Diamonds, Clubs) and
   card % 13 the rank, ASCENDING with the ace high: 0 is the two, 8 the
   ten, 9..11 J/Q/K, 12 the ace.  The game logic says so three ways --
   blackjack scores a plain card `% 13 + 2` and 8..11 as ten, pkrEvalHand
   takes rank 8 as the low card of a royal flush, and war's "Ace? I
   don't believe it!" is rank 12 -- and the images agree.

   The CARDS FILE uses a different order: each suit there runs
   K, Q, J, 10 .. 2, A, and cardLoad reads the first twelve into slots
   11..0 and the ace into slot 12.  The bitmaps in DATA/CARDS show it
   (file card 0 is the king of hearts, 11 the two, 12 the ace).  These
   names follow cardMfdb, not the file. */
#define CARDS_PER_SUIT                  13
#define CARD_RANK_2                      0
#define CARD_RANK_3                      1
#define CARD_RANK_4                      2
#define CARD_RANK_5                      3
#define CARD_RANK_6                      4
#define CARD_RANK_7                      5
#define CARD_RANK_8                      6
#define CARD_RANK_9                      7
#define CARD_RANK_10                     8
#define CARD_RANK_JACK                   9
#define CARD_RANK_QUEEN                 10
#define CARD_RANK_KING                  11
#define CARD_RANK_ACE                   12
#define CARD_HEART_2                     0
#define CARD_HEART_3                     1
#define CARD_HEART_4                     2
#define CARD_HEART_5                     3
#define CARD_HEART_6                     4
#define CARD_HEART_7                     5
#define CARD_HEART_8                     6
#define CARD_HEART_9                     7
#define CARD_HEART_10                    8
#define CARD_HEART_JACK                  9
#define CARD_HEART_QUEEN                10
#define CARD_HEART_KING                 11
#define CARD_HEART_ACE                  12
#define CARD_SPADE_2                    13
#define CARD_SPADE_3                    14
#define CARD_SPADE_4                    15
#define CARD_SPADE_5                    16
#define CARD_SPADE_6                    17
#define CARD_SPADE_7                    18
#define CARD_SPADE_8                    19
#define CARD_SPADE_9                    20
#define CARD_SPADE_10                   21
#define CARD_SPADE_JACK                 22
#define CARD_SPADE_QUEEN                23
#define CARD_SPADE_KING                 24
#define CARD_SPADE_ACE                  25
#define CARD_DIAMOND_2                  26
#define CARD_DIAMOND_3                  27
#define CARD_DIAMOND_4                  28
#define CARD_DIAMOND_5                  29
#define CARD_DIAMOND_6                  30
#define CARD_DIAMOND_7                  31
#define CARD_DIAMOND_8                  32
#define CARD_DIAMOND_9                  33
#define CARD_DIAMOND_10                 34
#define CARD_DIAMOND_JACK               35
#define CARD_DIAMOND_QUEEN              36
#define CARD_DIAMOND_KING               37
#define CARD_DIAMOND_ACE                38
#define CARD_CLUB_2                     39
#define CARD_CLUB_3                     40
#define CARD_CLUB_4                     41
#define CARD_CLUB_5                     42
#define CARD_CLUB_6                     43
#define CARD_CLUB_7                     44
#define CARD_CLUB_8                     45
#define CARD_CLUB_9                     46
#define CARD_CLUB_10                    47
#define CARD_CLUB_JACK                  48
#define CARD_CLUB_QUEEN                 49
#define CARD_CLUB_KING                  50
#define CARD_CLUB_ACE                   51
#define CARD_BACK                       52
/* The 53rd MFDB slot -- an all-background
   coloured card used to clear a slot when the player selects a card
   for discard (shown while the replacement is animating in). */
#define CARD_HIGHLIGHT                  53
/* "Empty slot" / end-of-pile sentinel in the war, blackjack and poker
   hand arrays.  It is 255, not -1: the original stores the byte 0xff. */
#define CARD_NONE                       255

/* Blackjack hit counter: at most 5 cards in a hand, so at most 3 hits.
   The player's (and the split hand's) counter starts at CARD_BJ_MAX,
   each hit subtracts CARD_BJ_STEP, and the hit loop runs while the
   counter is not CARD_BJ_STOP. */
#define CARD_BJ_MAX                     3
#define CARD_BJ_STEP                    1
#define CARD_BJ_STOP                    0

/* Poker hand ranks -- what pkrEvalHand stores through *hand_rank and what
   compRank / plyrRank hold.  Higher beats lower.  (pkrEvalHand's hc/bp
   counters use 1/3/7 as SCORES that are summed; those are not ranks.) */
#define HAND_HIGH_CARD                  0
#define HAND_ONE_PAIR                   1
#define HAND_TWO_PAIR                   2
#define HAND_THREE_OF_A_KIND            3
#define HAND_STRAIGHT                   4
#define HAND_FLUSH                      5
#define HAND_FULL_HOUSE                 6
#define HAND_FOUR_OF_A_KIND             7
#define HAND_STRAIGHT_FLUSH             8
#define HAND_ROYAL_FLUSH                9

/* cardKeyInput(a, b, c) return values.  The first three name a POSITION in
   the argument list, not a key: the caller decides which F-key each
   one is.  Digits '1'..'5' come back as 4..8, so `r - PK_IN_DIGIT_1`
   is the card slot 0..4.  An unused argument slot is passed as 255
   (never a key code); playPoker's discard loop passes 0 instead. */
#define PK_IN_ARG_A                     1
#define PK_IN_ARG_B                     2
#define PK_IN_ARG_C                     3
#define PK_IN_DIGIT_1                   4
#define PK_IN_DIGIT_5                   8
#define PK_IN_TIMEOUT                   (-1)    /* mgTimedOut set */
#define PK_IN_UNUSED                    255

/* ---- VDI fill styles ------------------------------------------------
   As used when drawing to the back buffer:
     vsf_interior(vdiHandle, 2)   -- interior = PATTERN
     vsf_style(vdiHandle, 8)   -- pattern index 8 (renders solid at slot 0) */
#define FILL_SOLID                      8
#ifndef FIS_PATTERN
#define FIS_PATTERN                     2       /* vsf_interior: pattern fill */
#endif

/* ---- TOS / GEM constants the port passes as numbers ------------------
   Standard Atari names where the DRI headers have one (tosdefs.h is
   not included here, so its RMODE_* are repeated; the guards keep a
   later include of it harmless).  MD_REPLACE comes from obdefs.h. */
#ifndef M_OFF
#define M_OFF                           256     /* graf_mouse: hide */
#define M_ON                            257     /* graf_mouse: show */
#endif
#ifndef RMODE_RD
#define RMODE_RD                        0       /* Fopen: read only */
#define RMODE_WR                        1       /* Fopen: write only */
#endif
#define F_NORMAL                        0       /* Fsfirst: no attribute bits,
                                                   plain files only */
#define GEMDOS_FSNEXT                   0x4F    /* bare gemdos() call */
#define ALERT_NO_DEFAULT                0       /* form_alert default button */
#define VEC_TIMER_A                     0x4d    /* Setexc vector ($134 / 4) */
#define SETEXC_QUERY                    (-1L)   /* Setexc: read, don't set */
#define XB_TIMER_A                      0       /* Xbtimer timer number */
#define MFP_STOP                        0       /* Xbtimer control: stopped */
#define MFP_DIV64                       5       /* Xbtimer control: /64 delay */

/* ---- Door / furniture state bitfield in resident.door_states_and_flags ---- */
#define DSF_FRONT_DOOR                  0x001
#define DSF_STUDY_DOOR                  0x002
#define DSF_CLOSET_DOOR                 0x004
#define DSF_KITCHEN_CABINET             0x008
#define DSF_DRESSER                     0x010
#define DSF_TOILET_DOOR                 0x020
#define DSF_FILING_CABINET              0x040
#define DSF_DOG_BOWL_MASK               0x180
#define DSF_FOOD_MASK                   0xE00
#define DSF_PRESERVE_UPPER_MASK         0xFE00
/* The kitchen cabinet's food count lives in DSF_FOOD_MASK: a 3-bit
   field, 0..FOOD_PACKS_MAX packs.  A new resident starts with it full. */
#define DSF_FOOD_SHIFT                  9
#define DSF_FOOD_FIELD                  7       /* DSF_FOOD_MASK >> DSF_FOOD_SHIFT */
#define FOOD_PACKS_MAX                  4

/* ---- Other resident status values ------------------------------------------ */
#define WATER_START                     7       /* new resident's tank */
#define WATER_MAX                       10      /* waterLevel, a full tank */
/* bathroom_timer once the need has fired: effectively off until eating
   (eatFromCabinet) reloads it from bathroom_timer_max. */
#define BATHROOM_TIMER_OFF              9999
/* sickness_countdown reloads, in simStep steps: sickness worsens one
   level every 60 and, once recovering, improves one every 5. */
#define SICK_DELAY_WORSENING            60
#define SICK_DELAY_IMPROVING            5

/* ---- Keyboard scancodes / Ctrl combos --------------------------------
   The 1985 code uses a keycode_enum where Ctrl+X maps to X-'@' (i.e.
   Ctrl+A=1, Ctrl+B=2, ...).  The names carry both the key and what the
   game does with it. */
/* KEY_NONE (-1) signals "nothing in the buffer". */
#define KEY_NONE                        (-1)
/* getKey maps the extended keys to its own small codes
   (cursor-left -> 8, F1..F10 -> 241..250) instead of 0x100|scan. */
#define KEY_CURSOR_LEFT                 8
#define KEY_F1                          241
#define KEY_F2                          242
#define KEY_F3                          243
#define KEY_F4                          244
#define KEY_F5                          245
#define KEY_F6                          246
#define KEY_F7                          247
#define KEY_F8                          248
#define KEY_F9                          249
#define KEY_F10                         250

/* The IKBD scancodes getKey translates into the codes above. */
#define SCAN_F1                         0x3b
#define SCAN_F2                         0x3c
#define SCAN_F3                         0x3d
#define SCAN_F4                         0x3e
#define SCAN_F5                         0x3f
#define SCAN_F6                         0x40
#define SCAN_F7                         0x41
#define SCAN_F8                         0x42
#define SCAN_F9                         0x43
#define SCAN_F10                        0x44
#define SCAN_CURSOR_LEFT                0x4b
#define KEY_CTRL_A_ALARM                0x01
#define KEY_CTRL_B_BOOK                 0x02
#define KEY_CTRL_C_CALL                 0x03
#define KEY_CTRL_D_DOGFOOD              0x04
#define KEY_CTRL_F_FOOD                 0x06
#define KEY_CTRL_M                      0x0D    /* Enter */
#define KEY_CTRL_P_PATTING              0x10
#define KEY_CTRL_R_RECORD               0x12
#define KEY_CTRL_W_WATER                0x17

/* ---- Activity tiers -- pickIdleAction's table_pick --------------------------
   scheduleTiers maps (time of day, resident.activity_level) to the first three;
   each picks one of the action tables activeActions / moderateActions / relaxedActions.
   Sunday turns ACTIVE into RELAXED and Saturday into MODERATE.  SLEEP
   (18+ hours awake, or sick) has no table: bed or nothing. */
#define TIER_ACTIVE                     0       /* activeActions */
#define TIER_MODERATE                   1       /* moderateActions */
#define TIER_RELAXED                    2       /* relaxedActions */
#define TIER_SLEEP                      3

/* phraseTable's end-of-table marker, in a row's first mask byte.  Alcyon
   narrows it to a signed char, so matchCommand's compare against the char
   field matches. */
#define EW2A_END                        0xff

/* ---- Action ids (runAction, the action queue, the AI tables) -------------
   The 5 EVENT actions (28..32) are INTERLEAVED with the regular actions,
   not appended at the end.  ACTION_NONE (-1)
   is the empty sentinel used by nextAction and the event FIFO. */
#define ACTION_NONE                     (-1)
#define ACTION_SIT_AND_EXERCISE          0
#define ACTION_READ_NEWSPAPER            1
#define ACTION_PLAY_COMPUTER             2
#define ACTION_WASH_HANDS                3
#define ACTION_GET_IN_OUT_OF_BED         4
#define ACTION_LISTEN_SONG               5
#define ACTION_STOP_RECORD               6       /* stopRecord */
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
#define ACTION_PLAY_ORGAN               26       /* playOrgan */
#define ACTION_OPEN_UPSTAIRS_CLOSET     27
#define ACTION_EVENT_RECORD_DELIVERY    28
#define ACTION_EVENT_FOOD_DELIVERY      29
#define ACTION_EVENT_PHONE_CALL         30
#define ACTION_EVENT_DOG_FOOD           31
#define ACTION_EVENT_BOOK_DELIVERY      32
#define ACTION_GET_SNACK_FROM_FRIDGE    33
#define ACTION_OPEN_BEDROOM_CLOSET      34
#define ACTION_NOD_OK                   35       /* nodOk; nothing queues it */
#define ACTION_CLEAN_UP                 36
#define ACTION_TIDY_HOUSE               37
#define ACTION_CHECK_FRONT_DOOR         38
#define ACTION_TOGGLE_TV                39
#define ACTION_CALL_DOG                 40
#define ACTION_WAKE_FROM_ALARM          41
#define ACTION_PET_DOG                  42
#define ACTION_WAKE_UP_MORNING          43
#define ACTION_GO_TO_BED_NIGHT          44

/* ---- Vocabulary word ids: index into vocabulary (161 entries) ------------
   The `2` suffixes on WORD_START2 / WORD_LIKE2 / WORD_IS2 tell apart
   the dictionary's duplicated entries; WORD_WHATS is spelt without the
   apostrophe so the name is a legal C identifier (the dictionary entry
   is literally "WHAT'S").

   The values are 1-based (vocabulary[0] is "PLEASE", WORD_PLEASE is 1),
   and WORD_NONE is -1. */
#define WORD_NONE                       (-1)
#define WORD_PLEASE                      1
#define WORD_DO                          2
#define WORD_YOU                         3
#define WORD_LIKE                        4
#define WORD_ENJOY                       5
#define WORD_WILL                        6
#define WORD_WOULD                       7
#define WORD_PLAY                        8
#define WORD_PERFORM                     9
#define WORD_USE                         10
#define WORD_TRY                         11
#define WORD_PLAYING                     12
#define WORD_ALLERGY                     13
#define WORD_ALLERGIC                    14
#define WORD_FEVER                       15
#define WORD_DUST                        16
#define WORD_POLLEN                      17
#define WORD_HANKY                       18
#define WORD_RELAX                       19
#define WORD_LIGHT                       20
#define WORD_START                       21
#define WORD_MAKE                        22
#define WORD_BURN                        23
#define WORD_IGNITE                      24
#define WORD_BUILD                       25
#define WORD_LOOKS                       26
#define WORD_IS                          27
#define WORD_SEEMS                       28
#define WORD_APPEARS                     29
#define WORD_SEEM                        30
#define WORD_LOOK                        31
#define WORD_APPEAR                      32
#define WORD_HEAR                        33
#define WORD_LISTEN                      34
#define WORD_PUT                         35
#define WORD_START2                      36
#define WORD_SPIN                        37
#define WORD_ON                          38
#define WORD_CLEAN                       39
#define WORD_TIDY                        40
#define WORD_PICK                        41
#define WORD_UP                          42
#define WORD_SLOPPY                      43
#define WORD_MESSY                       44
#define WORD_UNTIDY                      45
#define WORD_SHOULD                      46
#define WORD_OUGHT                       47
#define WORD_PROGRAM                     48
#define WORD_UTILITIES                   49
#define WORD_MATH                        50
#define WORD_HOMEWORK                    51
#define WORD_ADD                         52
#define WORD_SUBTRACT                    53
#define WORD_MULTIPLY                    54
#define WORD_DIVIDE                      55
#define WORD_TICKLE                      56
#define WORD_TYPE                        57
#define WORD_TELL                        58
#define WORD_WRITE                       59
#define WORD_CONFIDE                     60
#define WORD_BRUSH                       61
#define WORD_FLOSS                       62
#define WORD_DRINK                       63
#define WORD_IMBIBE                      64
#define WORD_GET                         65
#define WORD_FEED                        66
#define WORD_FILL                        67
#define WORD_OPEN                        68
#define WORD_DANCE                       69
#define WORD_MOON                        70
#define WORD_SHOW                        71
#define WORD_LIKE2                       72
#define WORD_TIRED                       73
#define WORD_BORED                       74
#define WORD_APATHETIC                   75
#define WORD_HATE                        76
#define WORD_AWFUL                       77
#define WORD_IF                          78
#define WORD_WHAT                        79
#define WORD_WHATS                       80
#define WORD_IN                          81
#define WORD_INSIDE                      82
#define WORD_STORED                      83
#define WORD_KEEP                        84
#define WORD_IS2                         85
#define WORD_PIANO                       86
#define WORD_ORGAN                       87
#define WORD_STEREO                      88
#define WORD_TURNTABLE                   89
#define WORD_MUSIC                       90
#define WORD_RECORD                      91
#define WORD_PLATTER                     92
#define WORD_FIRE                        93
#define WORD_FIREPLACE                   94
#define WORD_LOG                         95
#define WORD_CHILLY                      96
#define WORD_COLD                        97
#define WORD_PROBLEM                     98
#define WORD_PROBLEMS                    99
#define WORD_TROUBLES                    100
#define WORD_MATTER                      101
#define WORD_LETTER                      102
#define WORD_NOTE                        103
#define WORD_SONG                        104
#define WORD_TUNE                        105
#define WORD_SONATA                      106
#define WORD_FUGUE                       107
#define WORD_SERENADE                    108
#define WORD_JAZZ                        109
#define WORD_BOOGIE                      110
#define WORD_IVORIES                     111
#define WORD_TEETH                       112
#define WORD_HYGIENE                     113
#define WORD_GLASS                       114
#define WORD_COOLER                      115
#define WORD_DOG                         116
#define WORD_PET                         117
#define WORD_MUTT                        118
#define WORD_POOCH                       119
#define WORD_BOWL                        120
#define WORD_DISH                        121
#define WORD_CAN                         122
#define WORD_TV                          123
#define WORD_CHAIR                       124
#define WORD_COMPUTER                    125
#define WORD_ATARI                       126
#define WORD_WATER                       127
#define WORD_LIQUID                      128
#define WORD_LIQUIDS                     129
#define WORD_FLUID                       130
#define WORD_FLUIDS                      131
#define WORD_UPSTAIRS                    132
#define WORD_BEDROOM                     133
#define WORD_CLOSET                      134
#define WORD_KITCHEN                     135
#define WORD_FILING                      136
#define WORD_CABINET                     137
#define WORD_FREEZER                     138
#define WORD_REFRIDGERATOR               139
#define WORD_FRIDGE                      140
#define WORD_DRESSER                     141
#define WORD_NIGHTSTAND                  142
#define WORD_ADDITION                    143
#define WORD_SUBTRACTION                 144
#define WORD_MULTIPLICATION              145
#define WORD_DIVISION                    146
#define WORD_HOUSE                       147
#define WORD_HOME                        148
#define WORD_GAME                        149
#define WORD_CARDS                       150
#define WORD_POKER                       151
#define WORD_WAR                         152
#define WORD_CARD                        153
#define WORD_ANAGRAMS                    154
#define WORD_BLACKJACK                   155
#define WORD_EXCUSE                      156
#define WORD_PARDON                      157
#define WORD_HELLO                       158
#define WORD_ATTENTION                   159
#define WORD_HEY                         160

#endif  /* ENUMS_H */
