/*
 * dat_u1b.c -- the initialized globals that belong to the stx_u1
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  See CLAUDE.md,
 * "DATA and BSS layout".
 *
 * Not compiled standalone -- included by stx_u1.
 *
 * SECOND of three parts.  stx_u1's data is NOT all-globals-then-code:
 * execEv's and doAct's switch jump tables sit between pex_name and
 * g_atact, and getKey's right after rv_val.  So these six globals are
 * declared between actions.c and parts/getKey.c in the unit, and
 * dat_u1c's three come after getKey.  Do not reorder.
 */

/* AI action tables: 16 ACTION_IDs each, picked by chk_timA() at the
   active/moderate/relaxed tier. */

short   g_atact[16] = {
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

/* Moderate tier (chk_timA, TIER_MODERATE). */
short   g_atmod[16] = {
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

/* Relaxed tier (chk_timA, TIER_RELAXED). */
short   g_atrel[16] = {
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

/* sch_tab[3][8]: (phase, activity_level) -> TIER_*, i.e. byte offset
   hours_bucket*16 + activity_level*2.  A real 2-D array, not a table
   of row pointers -- chk_timA's code depends on that -- and it must
   sit directly after g_atrel in data. */
short           sch_tab[3][8] = {
        { TIER_ACTIVE,    TIER_ACTIVE,    TIER_RELAXED,   TIER_RELAXED,
          TIER_MODERATE,  TIER_MODERATE,  TIER_ACTIVE,    TIER_MODERATE },
        { TIER_RELAXED,   TIER_MODERATE,  TIER_ACTIVE,    TIER_MODERATE,
          TIER_RELAXED,   TIER_ACTIVE,    TIER_RELAXED,   TIER_ACTIVE },
        { TIER_MODERATE,  TIER_RELAXED,   TIER_MODERATE,  TIER_ACTIVE,
          TIER_ACTIVE,    TIER_RELAXED,   TIER_MODERATE,  TIER_RELAXED }
};

/* Bit-reversal pairs for rv_bld: when bit rv_msk[i] is set in a
   byte, bit rv_val[i] is set in its mirror image in rev_tab. */
short           rv_msk[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

short           rv_val[8] = {   1,  2,  4,  8, 16, 32, 64, 128 };   /* mirror bit for rv_msk[i] */
