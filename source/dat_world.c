/*
 * dat_world.c -- the initialized globals that belong to the stx_u1
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The ORDER of the
 * declarations below is the data layout and must not change.  See
 * CLAUDE.md, "DATA and BSS layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */




/* mainPalette[16]: Atari ST 12-bit RGB palette (4 bits per channel).
   Entries 0..15 map to the 16 screen colours in low-res mode.
   initAes loads this via Setpalette(mainPalette) at boot -- there is no
   later runtime palette rewrite from this table; slot 0 is the
   background (black), slot 14 white, etc.  pickClothes overwrites slots
   1 and 2 from the primary/secondary clothing tables; slot 6 is
   overwritten by setSkinColor for the sickness skin. */
short   mainPalette[16]           = {
        0x000, 0x442, 0x265, 0x754,
        0x310, 0x040, 0x754, 0x760,
        0x247, 0x631, 0x700, 0x333,
        0x555, 0x007, 0x777, 0x410
};

/* colorPens: color_enum -> VDI-color permutation.  The table is
   exactly TOS's default ST-low permutation from VDI-index to
   palette-slot.
   The game names its own colours by palette slot (see mainPalette) and
   calls vsl_color(colorPens[color_enum]) so that after TOS's
   permutation the pen lands on palette slot `color_enum`.

   With a properly-opened VDI workstation
   (LCP.PRG launched directly from the GEM desktop / Hatari --auto),
   TOS applies its default permutation and color_enum 13 (blue) ->
   colorPens[13] = 15 -> palette 13 = mainPalette[13] = 0x007 blue.
   Launching via COMMAND.PRG leaves the workstation in a state that
   collapses vsl_color's colour arg into pen 15 (dark brown 0x410)
   regardless of index -- see the beginDraw comment. */
short   colorPens[16]            = {
        0,  2,  3,  6,  4,  7,  5,  8,
        9, 10, 11, 14, 12, 15, 13,  1
};

short   keysBlocked          = NO;   /* YES while an activity owns the keyboard (letter writing, minigames): gameTick stops reading keys */

/* ---- Hardware sprite double-buffer (SPRITE_HW_SLOTS) -------------------
   Two parallel state sets per hardware slot: `pe` = pending (what game
   logic queued for the next 8 Hz compositor tick) and `ac` = active
   (currently drawn on the visible frame).  Slot layout: 0/7 = dog
   (behind/in-front of LCP by Y depth), 3 = LCP body, 4 = LCP head,
   1..2 and 5..6 = door/object overlay slots. */
/* Sized SPRITE_HW_SLOTS_ALLOC; see the note in sprglobs.h.  layoutSlots
   parks HIDDEN sprites in the disabled slot HW_SLOT_NONE (9), and
   gameTick's carrying path / carryBehind can then index these arrays at
   [9]; with 8 slots that write lands in the adjacent array, as in the
   original, so the declaration order matters. */
/* Explicitly initialized, so it lands in DATA (all zeros) rather than
   as a .comm -- that is where the original has it. */
short   pendReady[SPRITE_HW_SLOTS_ALLOC] = { 0 }; /* per-slot "pending" flag */

short   headPose                         = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);    /* head pose now: bits 0..2 angle, 3..4 tilt (see sprhead.c); HEAD_ANIM_DISABLED stops the head animation */

short   headTarget                         = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);    /* head pose the head animation is turning towards, same encoding */

short   headMode                         = HEAD_ANIM_DISABLED;   /* head animation mode (HEAD_ANIM_* / sprhead.c bit-fields) */

/* Two bytes of -1 that nothing references, between headMode and
   nextAction.  Dead 1985 data that Alcyon still allocates; it must stay
   for the layout. */
short   spareWord                         = -1;

short   nextAction                  = ACTION_NONE;   /* action chooseAction chose for runAction to run; ACTION_NONE = none */

short   recordPlaying              = 0;   /* YES while a record is playing (animates the player); saved in resident.record_playing */

short   tvRunning                       = 0;   /* YES while the TV is on (tvNoise draws the picture); saved in resident.tv_on */

BOOL16  phoneRinging  = NO;   /* phone is ringing: set by simStep or Ctrl-C, cleared when answered */

BOOL16  fireBurning                = NO;   /* fireplace is burning: animated each tick until fireTimeLeft runs out */

BOOL16  phoneAnswered     = NO;   /* resident is on the phone (answerPhone); blocks new calls */

/* Once-a-day flags for chooseAction's scheduled lunch, dinner, wake-up and
   bedtime actions: set when the action fires at its hour, cleared at
   midnight by resetDailyFlags. */
BOOL16  lunchDone      = NO;

BOOL16  dinnerDone     = NO;   /* dinner already triggered today */

BOOL16  wakeupDone  = NO;   /* wake-up already triggered today */

BOOL16  bedtimeDone         = NO;   /* bedtime already triggered today */

/* ---- Body / carry frame tables (index = PLAYER_STATE) ------------------ */
/* bodyIndex: maps animState -> body-frame index into body.lcp /
   bodyShapes. */
short   bodyIndex[93] = {
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

/* carryFrames: alternate arms-up frames used while carrying an object in
   walking states 0..24. */
short   carryFrames[25]      = {
        55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
        13, 14, 15, 16, 17, 18, 19, 18
};

/* Per-PLAYER_STATE horizontal offset for the head anchor (93
   entries, one per state 0..92). */
short   headXOffset[93] = {
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

/* Per-PLAYER_STATE head Y contribution (subtracted from body top). */
short   headYOffset[93] = {
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
   pick the "resting" horizontal direction the head drifts toward). */
short   headRestDir[93] = {
        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /*  0.. 3 */
        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /*  4.. 7 */
        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /*  8..11 */
        HEAD_DIR_RIGHT,        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_BACK,   /* 12..15 */
        HEAD_DIR_BACK,         HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /* 16..19 */
        HEAD_DIR_RIGHT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 20..23 */
        HEAD_DIR_FRONT,        HEAD_DIR_BACK_RIGHT,   HEAD_DIR_BACK,         HEAD_DIR_BACK,   /* 24..27 */
        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_BACK,   /* 28..31 */
        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 32..35 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_BACK,         HEAD_DIR_BACK,   /* 36..39 */
        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_FRONT,   /* 40..43 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /* 44..47 */
        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_FRONT,   /* 48..51 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 52..55 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 56..59 */
        HEAD_DIR_FRONT,        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_BACK,   /* 60..63 */
        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,        HEAD_DIR_RIGHT,   /* 64..67 */
        HEAD_DIR_RIGHT,        HEAD_DIR_FRONT_RIGHT,  HEAD_DIR_BACK,         HEAD_DIR_FRONT,   /* 68..71 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_BACK,   /* 72..75 */
        HEAD_DIR_BACK,         HEAD_DIR_BACK,         HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 76..79 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 80..83 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,   /* 84..87 */
        HEAD_DIR_FRONT,        HEAD_DIR_FRONT,        HEAD_DIR_BACK,         HEAD_DIR_BACK_RIGHT,   /* 88..91 */
        HEAD_DIR_FRONT_RIGHT   /* 92..92 */
};

/* posXHalf[48]: X half-pixel coordinate per house position (POS_*).
   Table value gets left-shifted by 1 at the call site to yield the
   full-pixel X (see posToXY). */
short   posXHalf[48] = {
        /* Floor 3 -- top       0..15 */
         22,  36,  49,  55,  60,  56,  73,  96,
        106, 118, 113, 110, 131,  47, 133, 146,
        /* Floor 2 -- middle   16..31 */
         16,  40,  27,  31,  45,  55,  84, 100,
        111, 100, 109, 124, 134, 135, 144,  67,
        /* Floor 1 -- bottom   32..47 */
          8,   8,  12,  19,  40,  25,  54,  49,
         67,  70, 106, 110, 123, 132, 147, 140
};

/* posYOffset[48]: Y offset from floor baseline per house position (POS_*). */
short   posYOffset[48] = {
          9,  14,   9,  10,  11,  14,  12,  13,
         12,  12,  12,   6,  15,  10,  14,   3,
          3,   3,   8,  15,  13,  13,  12,  13,
         14,  12,   8,  14,  13,  14,  13,   5,
          8,   3,  10,  13,  13,  14,  10,  14,
         14,  12,  13,   7,  14,  12,  13,   2
};

/* bodyYOffset: Y anchor offset per animState. */
short   bodyYOffset[109] = {
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

/* Staircase waypoints.  The two values that follow in memory (124,
   137) are the separate globals xLanding and yLanding, not part of
   this table. */
short   stairWaypts[6]    = { 170, 185, 133, 124, 182, 72 };

/* Middle-floor staircase-2 landing coordinates (top-of-flight X and Y).
   The middle-floor branch of nextWaypoint uses these to
   route through the between-floor landing instead of the raw
   stairWaypts entries. */
short   xLanding           = 124;

short   yLanding        = 137;   /* landing Y; xLanding (above) is the landing X */

short   floorWalkY[3]        = { 198, 135, 71 };   /* walking-line Y per floor, indexed floorOfY() - 1: bottom, middle, top */

/* On-stairs flag (short, YES/NO).  YES while
   the path stepper is inside a stair-traversal path; drives the
   stair-specific sprite-state sequence 9..24 and the wood-stairs SFX
   selection. */
short   onStairs              = 0;

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   floorBottomY[0] = bottom floor, [1] = middle floor, [2] = top. */
short   floorBottomY[3]        = { 202, 140, 77 };

/* spriteFileId: file-record index -> sprite_id slot to store its pointers in. */
short   spriteFileId[50] = {
        12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27,
        28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43,
        44,  9, 45, 46, 47, 48, 49,  3,
         4, 50,  7,  6, 51, 52, 53, 54,
         8, 55
};

/* ---- Dog sprite pointers / buffers ------------------------------------- */
/* dogWalkSprites: 8 sprite ids the walk cycle rotates through in moveDog. */
short   dogWalkSprites[8] = {
        SPRITE_DOG_WLK_R1, SPRITE_DOG_WLK_R2,
        SPRITE_DOG_WLK_R3, SPRITE_DOG_WLK_R4,
        SPRITE_DOG_WLK_R5, SPRITE_DOG_WLK_R7,
        SPRITE_DOG_WLK_R8, SPRITE_DOG_WLK_R9
};
