/*
 * dat_u3b.c -- the initialized globals that belong to the stx_u3
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u3.
 */



/* 15-entry delta table (Ghidra 0x2BA06). */
short   hd_mvd[15]   = {
         1,  1,  1, 99, -1, -1, -1,  0,
         1,  1,  1, 99, -1, -1, -1
};



/* Per-tilt frame-index offset (Ghidra 0x2BA24, 3 shorts -- distance
   to head_anim_delay_countdown @ 0x2BA2A is 6 bytes). */
short   hd_tilt[3]       = { 7, 12, 17 };


/* Ghidra head_anim_delay_countdown @ 0x2ba2a = 1. */
short   g_hadec                         = 1;



/* Per-happiness-level head frame base index (into pex_ptr). */
/* Ghidra 0x2BA2C. */
short   mood_hfo[3]  = { 44, 0, 22 };



/* ONE HUNDRED AND SIXTY-ONE bytes: the reference closes the table
   with a -1 and Alcyon pads the odd length to 162, which is why
   g_ew2b starts at 0x23f8 rather than 0x23f6.  The 0xff was
   previously mistaken for a sentinel at the HEAD of g_ew2b. */
char  ew2pos[161] = {
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



/* g_ew2b: 160-byte WORD_ID -> bit, starting at 3.  It has no head
   sentinel; the {255, 0} the port once carried here was ew2pos's
   trailing -1 plus its alignment pad. */
char  g_ew2b[160] = {
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



/* ---- Vocabulary (160 words, from lcp/LCP.py reference) ---- */
char * vwd_tab[161] = {
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
   "mismatched curly braces", but takes the flattened list -- which
   is how LCP_STX ships this table as initialized DATA.  Ten mask
   bytes, the action id at +10, the priority offset at +11. */
WORD_TO_ACTION g_ew2a[34] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,  24,  15,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,  20,   4,
    0x02, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,  20,   2,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,  20,   4,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,   5,   4,
    0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  36,   8,
    0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00,  36,   2,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  26,   4,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,  26,   4,
    0x00, 0x00, 0x04, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,  26,   4,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,   7,   8,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,   7,   6,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,  17,   2,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,  17,   2,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,  13,   2,
    0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,  13,   4,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,  31,   8,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,  31,   8,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,  31,   8,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   8,   2,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,   6,   8,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,   6,   8,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,  16,   8,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,  14,   6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,   2,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x05, 0x00, 0x00,  27,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00,  34,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x28, 0x00, 0x00,  18,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x30, 0x00, 0x00,  16,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00,  18,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x00, 0x00,  18,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00,  34,   6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00,  34,   6,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0,   0
};


/* Eight entries, not nine: LCP_STX's gap here is 8 bytes. */
char            bm_lo[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};


/* Ghidra happiniess_to_priority (sic) @ 0x2bf98: {3, 1, 0}.  Used as
   the base priority for parsed commands -- HAPPY (0) gives priority 3
   (accepts more), SAD (2) gives 0 (rejects most).  Port previously
   had guessed {2, 4, 6} which inverted the intended behavior. */
short           mood_pri[3]        = { 3, 1, 0 };
