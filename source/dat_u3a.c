/*
 * dat_u3a.c -- the initialized globals that belong to the stx_u3
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The ORDER of the
 * declarations below therefore decides the data layout and must not
 * change.  See CLAUDE.md, "DATA and BSS layout".
 *
 * Not compiled standalone -- included by stx_u3.
 */


BOOL16  pat_ok                 = NO;


BOOL16  g_ptdoa              = NO;



short   g_trel[10] = {
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE,
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE
};


/* Sprite layer flags: entries 0,1 = SPRITE_IN_FRONT (1),
   rest = SPRITE_HIDDEN (0).  These are the two dog slot flags (slots
   0 and 7 in the hardware layout, per sp_upds). */
short   g_selaf[SPRITE_SLOTS] = { 1, 1 };


/* Which hardware slot each logical
   sprite is currently mapped to.  Entries 0..1 pin the LCP body/head
   to their dedicated slots; the rest default to HW_SLOT_NONE (=9,
   the compositor's off-screen sentinel) and get assigned dynamically
   by sprite_update_slots when the sprite is queued. */
short   g_seslm[SPRITE_SLOTS] = {
        /* 0..9   */ HW_SLOT_LCP_BODY, HW_SLOT_LCP_HEAD,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
        /* 10..19 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 20..29 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 30..39 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 40..49 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE,
        /* 50..59 */ HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE, HW_SLOT_NONE,
                     HW_SLOT_NONE, HW_SLOT_NONE
};



/* bm32or[i] = 1<<i, bm32and[i] = ~(1<<i).  The original ships both
   tables as initialized data rather than building them at run time. */
long    bm32or[32] = {
        0x00000001L,
        0x00000002L,
        0x00000004L,
        0x00000008L,
        0x00000010L,
        0x00000020L,
        0x00000040L,
        0x00000080L,
        0x00000100L,
        0x00000200L,
        0x00000400L,
        0x00000800L,
        0x00001000L,
        0x00002000L,
        0x00004000L,
        0x00008000L,
        0x00010000L,
        0x00020000L,
        0x00040000L,
        0x00080000L,
        0x00100000L,
        0x00200000L,
        0x00400000L,
        0x00800000L,
        0x01000000L,
        0x02000000L,
        0x04000000L,
        0x08000000L,
        0x10000000L,
        0x20000000L,
        0x40000000L,
        0x80000000L
};



long    bm32and[32] = {
        0xfffffffeL,
        0xfffffffdL,
        0xfffffffbL,
        0xfffffff7L,
        0xffffffefL,
        0xffffffdfL,
        0xffffffbfL,
        0xffffff7fL,
        0xfffffeffL,
        0xfffffdffL,
        0xfffffbffL,
        0xfffff7ffL,
        0xffffefffL,
        0xffffdfffL,
        0xffffbfffL,
        0xffff7fffL,
        0xfffeffffL,
        0xfffdffffL,
        0xfffbffffL,
        0xfff7ffffL,
        0xffefffffL,
        0xffdfffffL,
        0xffbfffffL,
        0xff7fffffL,
        0xfeffffffL,
        0xfdffffffL,
        0xfbffffffL,
        0xf7ffffffL,
        0xefffffffL,
        0xdfffffffL,
        0xbfffffffL,
        0x7fffffffL
};


/* NINE HOUSE_POS entries the dog picks (via rndRng) as its next
   wander target -- the picker's index is rndRng(base, 8), so 0..8. */
short   g_ddipt[9] = {
        POS_TOP_LIVING_ROOM,       POS_TOP_GAME_CHAIR_RIGHT,
        POS_TOP_FIREPLACE_RIGHT,   POS_MID_BEDROOM_WALK,
        POS_MID_COMPUTER_DESK,     POS_BTM_STAIR_LANDING,
        POS_BTM_DOG_BOWL,          POS_BTM_WATER_TAP,
        POS_BTM_SCREEN_EDGE
};


/* Used by the cutscene
   at startup to seed the dog's first wander target -- the dog walks
   in from the bottom-screen edge. */
short   g_dgitx        = POS_BTM_SCREEN_EDGE;


short   g_ddyot[9]      = { 3, 9, 2, 10, 6, 0, 0, 11, 3 };


/* Y micro-nudge applied
   to the initial dog target position. */
short   g_dgiyo            = 3;


/* Per-destination pixel nudges applied after hs_posXY returns the
   anchor for the destination.  g_ddyot is nine like g_ddipt, but
   g_ddxot takes ELEVEN entries' worth of storage in the original.
   Only 0..8 are ever indexed; the two extra zeros keep the layout. */
short   g_ddxot[11]     = { 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0 };


short   g_dseat[3]   = {
        SPRITE_DOG_EATING_1, SPRITE_DOG_EATING_2, SPRITE_DOG_EATING_3
};



/* Animation frame tables consumed by gameTick.  Every
   value is an object_tab_mfdb index; gameTick indexes these by a
   small counter to pick which sprite/frame to draw. */
short   g_obcla[4]     = { OBJ_CLOCK_1, OBJ_CLOCK_2,
                           OBJ_CLOCK_1, OBJ_CLOCK_3 };


short   g_obala[2]     = { OBJ_ALARM_1, OBJ_ALARM_2 };


short   g_obpha[4]     = { OBJ_PHONE_2, OBJ_PHONE_1,
                           OBJ_PHONE_2, OBJ_PHONE_3 };


short   g_obfia[4]     = { OBJ_FIRE_1, OBJ_FIRE_2,
                           OBJ_FIRE_3, OBJ_FIRE_4 };



/* Petting-dog sprite frames -- sprite ids the petting animation
   cycles through: ping-pong over frames 1..6 back down to 2.  TEN
   entries, with no trailing SPRITE_PET_HAND_1 and no 0 terminator. */
short   g_ptdsi[10]    = {
        SPRITE_PET_HAND_1, SPRITE_PET_HAND_2, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_4, SPRITE_PET_HAND_5, SPRITE_PET_HAND_6,
        SPRITE_PET_HAND_5, SPRITE_PET_HAND_4, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_2
};



/* Frame-state globals for the petting animation (g_ptanf, the frame
   counter, lives in globals.c).  g_ptlss is the last sprite drawn. */
short   g_ptlss                         = SPRITE_PET_HAND_1;


short   g_obdea[3]     = { OBJ_DOG_FOOD_BOWL_3,
                           OBJ_DOG_FOOD_BOWL_2,
                           OBJ_DOG_FOOD_BOWL_1 };
