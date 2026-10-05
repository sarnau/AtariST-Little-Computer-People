/*
 * dat_anim.c -- the initialized globals that belong to the stx_u3
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The ORDER of the
 * declarations below therefore decides the data layout and must not
 * change.  See docs/history.md, "DATA and BSS layout".
 *
 */

BOOL16  patAllowed = NO;

/* YES while a Ctrl-P pat (hand animation) is running; gameTick clears it
   when the cycle ends. */
BOOL16  patActive = NO;

/* Event queue: up to ten ACTION_* events filled by queueEvent and drained
   from the front by nextEvent; ACTION_NONE marks an empty slot, so
   eventQueue[0] != ACTION_NONE means an event is waiting. */
short   eventQueue[10] = {
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE,
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE
};

/* Layer of each logical sprite (SPRITE_HIDDEN, SPRITE_BEHIND_LCP or
   SPRITE_IN_FRONT).  Only the resident's body and head start visible;
   every other sprite is hidden until an action shows it. */
short   spriteLayer[SPRITE_SLOTS] = { 1, 1 };

/* Hardware slot each logical sprite is drawn through.  The resident's
   body and head own slots 3 and 4 for good; every other sprite starts
   at HW_SLOT_NONE, which the compositor never draws, and layoutSlots
   hands it a slot by layer when it becomes visible. */
short   spriteSlot[SPRITE_SLOTS] = {
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

/* bitSet32[i] = 1<<i, bitClear32[i] = ~(1<<i).  The original ships both
   tables as initialized data rather than building them at run time. */
long    bitSet32[32] = {
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

/* bitClear32[i] = ~(1<<i), used by maskHead to clear one bit of a mask. */
long    bitClear32[32] = {
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

/* NINE house positions (POS_*) the dog picks (via rndRng) as its next
   wander target -- the picker's index is rndRng(base, 8), so 0..8. */
short   dogRoamSpots[9] = {
        POS_TOP_LIVING_ROOM,       POS_TOP_GAME_CHAIR_RIGHT,
        POS_TOP_FIREPLACE_RIGHT,   POS_MID_BEDROOM_WALK,
        POS_MID_COMPUTER_DESK,     POS_BTM_STAIR_LANDING,
        POS_BTM_DOG_BOWL,          POS_BTM_WATER_TAP,
        POS_BTM_SCREEN_EDGE
};

/* Used by the cutscene
   at startup to seed the dog's first wander target -- the dog walks
   in from the bottom-screen edge. */
short   dogStartPos = POS_BTM_SCREEN_EDGE;

/* Y nudge added to the dog's target, per dogRoamSpots entry (see
   dogXNudge). */
short   dogYNudge[9] = { 3, 9, 2, 10, 6, 0, 0, 11, 3 };

/* Y micro-nudge applied
   to the initial dog target position. */
short   dogYStartNudge = 3;

/* Per-destination pixel nudges applied after posToXY returns the
   anchor for the destination.  dogYNudge is nine like dogRoamSpots, but
   dogXNudge takes ELEVEN entries' worth of storage in the original.
   Only 0..8 are ever indexed; the two extra zeros keep the layout. */
short   dogXNudge[11] = { 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0 };

/* Dog eating frames, chosen by the eating countdown dogEatCount % 3. */
short   dogEatFrames[3] = {
        SPRITE_DOG_EATING_1, SPRITE_DOG_EATING_2, SPRITE_DOG_EATING_3
};

/* Animation frame tables for gameTick: each entry is an OBJ_* id that
   drawObject blits, picked by a small frame counter. */
short   clockFrames[4] = { OBJ_CLOCK_1, OBJ_CLOCK_2,
                           OBJ_CLOCK_1, OBJ_CLOCK_3 };

/* Ringing alarm clock, two frames. */
short   alarmFrames[2] = { OBJ_ALARM_1, OBJ_ALARM_2 };

/* Ringing phone frames. */
short   phoneFrames[4] = { OBJ_PHONE_2, OBJ_PHONE_1,
                           OBJ_PHONE_2, OBJ_PHONE_3 };

/* Fireplace flame frames while fireBurning. */
short   fireFrames[4] = { OBJ_FIRE_1, OBJ_FIRE_2,
                          OBJ_FIRE_3, OBJ_FIRE_4 };

/* Ctrl-P petting-hand sprite frames: ping-pong over hands 1..6 and back
   down to 2.  PAT_FRAMES entries; the animation's final frame, hand 1,
   is read from patLastSprite just past the table (see PAT_FRAMES), so
   do not add it here and do not move patLastSprite away. */
short   patSprites[PAT_FRAMES] = {
        SPRITE_PET_HAND_1, SPRITE_PET_HAND_2, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_4, SPRITE_PET_HAND_5, SPRITE_PET_HAND_6,
        SPRITE_PET_HAND_5, SPRITE_PET_HAND_4, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_2
};

/* Frame-state globals for the petting animation (patFrame, the frame
   counter, lives in globals.c).  patLastSprite is the last sprite drawn. */
short   patLastSprite = SPRITE_PET_HAND_1;

/* Dog bowl object per bowlLevel (BOWL_EMPTY, BOWL_HALF, BOWL_FULL). */
short   bowlFrames[3] = { OBJ_DOG_FOOD_BOWL_3,
                          OBJ_DOG_FOOD_BOWL_2,
                          OBJ_DOG_FOOD_BOWL_1 };
