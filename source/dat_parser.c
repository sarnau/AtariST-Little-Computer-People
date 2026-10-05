/*
 * dat_parser.c -- the initialized globals that belong to the stx_u3
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The ORDER of the
 * declarations below therefore decides the data layout and must not
 * change.  See docs/history.md, "DATA and BSS layout".
 *
 */

/* One step of head turning, indexed by (target - current direction) + 7:
   +1 or -1 moves the head one direction toward the target the short
   way round, 0 means it is there, and HEAD_TURN_NONE marks the
   half-turn, which has no short way. */
short   headTurnStep[15] = {
         1,  1,  1, HEAD_TURN_NONE, -1, -1, -1,  0,
         1,  1,  1, HEAD_TURN_NONE, -1, -1, -1
};

/* First frame of each tilt row, indexed by the tilt bits of headPose. */
short   headTiltFrame[3] = { HEAD_ROW_LEVEL, HEAD_ROW_LOWER, HEAD_ROW_LOWEST };

/* Head-animation delay countdown. */
short   headDelay = 1;

/* First head frame of each mood's block in pexFrames, indexed by
   resident.happiness (MOOD_HAPPY, MOOD_CONTENT, MOOD_SAD): the file
   stores content, sad, happy. */
short   moodHeadBase[3] = { 2 * HEAD_FRAMES_PER_MOOD, 0, HEAD_FRAMES_PER_MOOD };

/* WORD_ID -> byte index into phraseBits.  ONE HUNDRED AND SIXTY-ONE
   bytes: the table closes with a -1 and Alcyon pads the odd length
   to 162. */
char  wordByte[161] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 9, 9, 9, 9, 9,
   -1
};

/* wordBit: 160-byte WORD_ID -> bit number within that byte.  It has
   no head sentinel. */
char  wordBit[160] = {
      3,   0,   1,   2,   2,   4,   4,   5,   5,   5,
      5,   5,   6,   6,   6,   6,   6,   6,   7,   0,
      0,   0,   0,   0,   0,   1,   1,   1,   1,   2,
      2,   2,   3,   3,   3,   3,   3,   4,   5,   5,
      5,   6,   7,   7,   7,   0,   0,   1,   1,   1,
      1,   1,   1,   1,   1,   2,   3,   3,   3,   3,
      4,   4,   5,   5,   6,   7,   0,   1,   2,   2,
      2,   3,   4,   4,   4,   5,   5,   7,   1,   1,
      2,   2,   2,   2,   3,   0,   0,   1,   1,   1,
      1,   1,   2,   2,   2,   3,   3,   4,   4,   4,
      4,   4,   4,   5,   5,   5,   5,   5,   5,   5,
      6,   0,   0,   1,   1,   2,   2,   2,   2,   3,
      3,   3,   4,   5,   6,   6,   7,   7,   7,   7,
      7,   0,   1,   2,   3,   4,   5,   6,   7,   7,
      0,   1,   2,   2,   2,   2,   3,   3,   4,   4,
      4,   4,   4,   4,   4,   1,   1,   1,   1,   1
};

/* ---- Vocabulary (160 words) ---- */
char * vocabulary[161] = {
    "PLEASE", "DO", "YOU", "LIKE",
    "ENJOY", "WILL", "WOULD", "PLAY",
    "PERFORM", "USE", "TRY", "PLAYING",
    "ALLERGY", "ALLERGIC", "FEVER", "DUST",
    "POLLEN", "HANKY", "RELAX", "LIGHT",
    "START", "MAKE", "BURN", "IGNITE",
    "BUILD", "LOOKS", "IS", "SEEMS",
    "APPEARS", "SEEM", "LOOK", "APPEAR",
    "HEAR", "LISTEN", "PUT", "START",
    "SPIN", "ON", "CLEAN", "TIDY",
    "PICK", "UP", "SLOPPY", "MESSY",
    "UNTIDY", "SHOULD", "OUGHT", "PROGRAM",
    "UTILITIES", "MATH", "HOMEWORK", "ADD",
    "SUBTRACT", "MULTIPLY", "DIVIDE", "TICKLE",
    "TYPE", "TELL", "WRITE", "CONFIDE",
    "BRUSH", "FLOSS", "DRINK", "IMBIBE",
    "GET", "FEED", "FILL", "OPEN",
    "DANCE", "MOON", "SHOW", "LIKE",
    "TIRED", "BORED", "APATHETIC", "HATE",
    "AWFUL", "IF", "WHAT", "WHAT\'S",
    "IN", "INSIDE", "STORED", "KEEP",
    "IS", "PIANO", "ORGAN", "STEREO",
    "TURNTABLE", "MUSIC", "RECORD", "PLATTER",
    "FIRE", "FIREPLACE", "LOG", "CHILLY",
    "COLD", "PROBLEM", "PROBLEMS", "TROUBLES",
    "MATTER", "LETTER", "NOTE", "SONG",
    "TUNE", "SONATA", "FUGUE", "SERENADE",
    "JAZZ", "BOOGIE", "IVORIES", "TEETH",
    "HYGIENE", "GLASS", "COOLER", "DOG",
    "PET", "MUTT", "POOCH", "BOWL",
    "DISH", "CAN", "TV", "CHAIR",
    "COMPUTER", "ATARI", "WATER", "LIQUID",
    "LIQUIDS", "FLUID", "FLUIDS", "UPSTAIRS",
    "BEDROOM", "CLOSET", "KITCHEN", "FILING",
    "CABINET", "FREEZER", "REFRIDGERATOR", "FRIDGE",
    "DRESSER", "NIGHTSTAND", "ADDITION", "SUBTRACTION",
    "MULTIPLICATION", "DIVISION", "HOUSE", "HOME",
    "GAME", "CARDS", "POKER", "WAR",
    "CARD", "ANAGRAMS", "BLACKJACK", "EXCUSE",
    "PARDON", "HELLO", "ATTENTION", "HEY",

    (char *) 0    /* sentinel */
};

/* Alcyon C 4.14 rejects the NESTED form `{ {..}, a, p }` with
   "mismatched curly braces", but takes the flattened list, which
   gives the same initialized data.  Ten mask
   bytes, the action id at +10, the priority offset at +11.

   A row fires when every bit of its mask is set in phraseBits, i.e. when
   the command contains one word from EACH group named in its comment;
   other words do not matter, and the first matching row wins.  Word w
   sets bit wordBit[w] of byte wordByte[w].  The word groups below are
   derived from those two tables and vocabulary -- vocabulary's later
   duplicates (START, LIKE, IS) and PLEASE (index 0, which matchCommand
   treats as unrecognised) never set a bit. */
WORD_TO_ACTION phraseTable[34] = {
    /* 0: (no word) + EXCUSE/PARDON/HELLO/ATTENTION/HEY -- never fires:
       no word sets byte 9 bit 0x01 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, ACTION_HELLO, 15,
    /* 1: LIGHT/START/MAKE/BURN/IGNITE/BUILD + FIRE/FIREPLACE/LOG */
    0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, ACTION_LIGHT_FIREPLACE, 4,
    /* 2: YOU + SEEM/LOOK/APPEAR + CHILLY/COLD */
    0x02, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, ACTION_LIGHT_FIREPLACE, 2,
    /* 3: PLAY/PERFORM/USE/TRY/PLAYING + FIRE/FIREPLACE/LOG */
    0x20, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, ACTION_LIGHT_FIREPLACE, 4,
    /* 4: HEAR/LISTEN/PUT/SPIN + STEREO/TURNTABLE/MUSIC/RECORD/PLATTER */
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, ACTION_LISTEN_SONG, 4,
    /* 5: CLEAN/TIDY/PICK + UP + SHOULD/OUGHT */
    0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ACTION_CLEAN_UP, 8,
    /* 6: SLOPPY/MESSY/UNTIDY + (no word) + HOUSE/HOME -- never fires:
       only vocabulary's shadowed second IS sets byte 4 bit 0x08 */
    0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, ACTION_CLEAN_UP, 2,
    /* 7: PLAY/PERFORM/USE/TRY/PLAYING + PIANO/ORGAN */
    0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, ACTION_PLAY_ORGAN, 4,
    /* 8: PLAY/PERFORM/USE/TRY/PLAYING +
       SONG/TUNE/SONATA/FUGUE/SERENADE/JAZZ/BOOGIE */
    0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, ACTION_PLAY_ORGAN, 4,
    /* 9: TICKLE + IVORIES */
    0x00, 0x00, 0x04, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, ACTION_PLAY_ORGAN, 4,
    /* 10: TYPE/TELL/WRITE/CONFIDE +
       PROBLEM/PROBLEMS/TROUBLES/MATTER/LETTER/NOTE */
    0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, ACTION_WRITE_LETTER, 8,
    /* 11: LOOKS/IS/SEEMS/APPEARS +
       PROBLEM/PROBLEMS/TROUBLES/MATTER/LETTER/NOTE */
    0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, ACTION_WRITE_LETTER, 6,
    /* 12: BRUSH/FLOSS + TEETH/HYGIENE */
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, ACTION_BRUSH_TEETH, 2,
    /* 13: SLOPPY/MESSY/UNTIDY + TEETH/HYGIENE */
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, ACTION_BRUSH_TEETH, 2,
    /* 14: DRINK/IMBIBE + WATER/LIQUID/LIQUIDS/FLUID/FLUIDS */
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, ACTION_DRINK, 2,
    /* 15: SEEM/LOOK/APPEAR + GLASS/COOLER */
    0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, ACTION_DRINK, 4,
    /* 16: FEED + DOG/PET/MUTT/POOCH */
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, ACTION_EVENT_DOG_FOOD, 8,
    /* 17: FILL + BOWL/DISH/CAN */
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, ACTION_EVENT_DOG_FOOD, 8,
    /* 18: OPEN + BOWL/DISH/CAN */
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, ACTION_EVENT_DOG_FOOD, 8,
    /* 19: DANCE/MOON/SHOW */
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ACTION_DANCE, 2,
    /* 20: TIRED/BORED/APATHETIC + STEREO/TURNTABLE/MUSIC/RECORD/PLATTER */
    0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, ACTION_STOP_RECORD, 8,
    /* 21: HATE/AWFUL + STEREO/TURNTABLE/MUSIC/RECORD/PLATTER */
    0x00, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, ACTION_STOP_RECORD, 8,
    /* 22: PLAY/PERFORM/USE/TRY/PLAYING +
       GAME/CARDS/POKER/WAR/CARD/ANAGRAMS/BLACKJACK */
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, ACTION_PLAY_A_GAME, 8,
    /* 23: ALLERGY/ALLERGIC/FEVER/DUST/POLLEN/HANKY +
       ADDITION/SUBTRACTION/MULTIPLICATION/DIVISION */
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, ACTION_NOD_HEAD, 6,
    /* 24: COMPUTER/ATARI */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, ACTION_PLAY_COMPUTER, 6,
    /* 25: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + UPSTAIRS + CLOSET */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x05, 0x00, 0x00, ACTION_OPEN_UPSTAIRS_CLOSET, 6,
    /* 26: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + BEDROOM + CLOSET */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, ACTION_OPEN_BEDROOM_CLOSET, 6,
    /* 27: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + KITCHEN + CABINET */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x28, 0x00, 0x00, ACTION_KITCHEN_CABINET, 6,
    /* 28: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + FILING + CABINET */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x30, 0x00, 0x00, ACTION_PLAY_A_GAME, 6,
    /* 29: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + FREEZER */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00, ACTION_KITCHEN_CABINET, 6,
    /* 30: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + REFRIDGERATOR/FRIDGE */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x00, 0x00, ACTION_KITCHEN_CABINET, 6,
    /* 31: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + DRESSER */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, ACTION_OPEN_BEDROOM_CLOSET, 6,
    /* 32: WHAT/WHAT'S + IN/INSIDE/STORED/KEEP + NIGHTSTAND */
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, ACTION_OPEN_BEDROOM_CLOSET, 6,
    /* end of table */
    EW2A_END, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0,  0
};

/* Bit masks for bit numbers 0..7.  Eight entries, not nine. */
char            bitMask8[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/* Mood -> base priority for parsed commands: HAPPY (0) gives
   priority 3 (accepts more), SAD (2) gives 0 (rejects most). */
short           moodPriority[3] = { 3, 1, 0 };
