/*
 * dat_aitables.c -- the initialized globals that belong to the stx_u1
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  See CLAUDE.md,
 * "DATA and BSS layout".
 *
 *
 *
 * SECOND of three parts.  stx_u1's data is NOT all-globals-then-code:
 * runEvent's and runAction's switch jump tables sit between pexName and
 * activeActions, and getKey's right after mirrorDstBit.  So these six globals are
 * declared between actions.c and parts/getKey.c in the unit, and
 * dat_colors's three come after getKey.  Do not reorder.
 */

/* AI action tables: 16 ACTION_IDs each, picked by pickIdleAction() at the
   active/moderate/relaxed tier. */

short   activeActions[16] = {
        ACTION_OPEN_UPSTAIRS_CLOSET,
        ACTION_CLEAN_UP,
        ACTION_PLAY_COMPUTER,
        ACTION_WRITE_LETTER,
        ACTION_TIDY_HOUSE,
        ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_EVENT_PHONE_CALL,
        ACTION_FEED_DOG,
        ACTION_HELLO,
        ACTION_SIT_AND_EXERCISE,
        ACTION_PLAY_COMPUTER,
        ACTION_CLEAN_UP,
        ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_CHECK_FRONT_DOOR,
        ACTION_PLAY_COMPUTER,
        ACTION_TIDY_HOUSE
};

/* Moderate tier (pickIdleAction, TIER_MODERATE). */
short   moderateActions[16] = {
        ACTION_HELLO,
        ACTION_DANCE,
        ACTION_CHECK_FRONT_DOOR,
        ACTION_TOGGLE_TV,
        ACTION_LISTEN_SONG,
        ACTION_PLAY_ORGAN,
        ACTION_EVENT_PHONE_CALL,
        ACTION_TOGGLE_TV,
        ACTION_READ_NEWSPAPER,
        ACTION_PACE_NERVOUSLY,
        ACTION_PLAY_A_GAME,
        ACTION_OPEN_UPSTAIRS_CLOSET,
        ACTION_SIT_AND_EXERCISE,
        ACTION_HELLO,
        ACTION_DANCE,
        ACTION_EVENT_PHONE_CALL
};

/* Relaxed tier (pickIdleAction, TIER_RELAXED). */
short   relaxedActions[16] = {
        ACTION_READ_NEWSPAPER,
        ACTION_PET_DOG,
        ACTION_LIGHT_FIREPLACE,
        ACTION_LISTEN_SONG,
        ACTION_OPEN_UPSTAIRS_CLOSET,
        ACTION_TOGGLE_TV,
        ACTION_EVENT_PHONE_CALL,
        ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_HELLO,
        ACTION_SLEEP,
        ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_PET_DOG,
        ACTION_CHECK_FRONT_DOOR,
        ACTION_STOP_RECORD,
        ACTION_READ_NEWSPAPER,
        ACTION_TOGGLE_TV
};

/* scheduleTiers[3][8]: (phase, activity_level) -> TIER_*, i.e. byte offset
   hours_bucket*16 + activity_level*2.  A real 2-D array, not a table
   of row pointers -- pickIdleAction's code depends on that -- and it must
   sit directly after relaxedActions in data. */
short           scheduleTiers[3][8] = {
        { TIER_ACTIVE,    TIER_ACTIVE,    TIER_RELAXED,   TIER_RELAXED,
          TIER_MODERATE,  TIER_MODERATE,  TIER_ACTIVE,    TIER_MODERATE },
        { TIER_RELAXED,   TIER_MODERATE,  TIER_ACTIVE,    TIER_MODERATE,
          TIER_RELAXED,   TIER_ACTIVE,    TIER_RELAXED,   TIER_ACTIVE },
        { TIER_MODERATE,  TIER_RELAXED,   TIER_MODERATE,  TIER_ACTIVE,
          TIER_ACTIVE,    TIER_RELAXED,   TIER_MODERATE,  TIER_RELAXED }
};

/* Bit-reversal pairs for buildMirrorTable: when bit mirrorSrcBit[i] is set in a
   byte, bit mirrorDstBit[i] is set in its mirror image in mirrorTable. */
short           mirrorSrcBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

short           mirrorDstBit[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };   /* mirror bit for mirrorSrcBit[i] */
