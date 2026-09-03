/*
 * dat_u3a.c -- the initialized globals that belong to the stx_u3
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


BOOL16  dg_petok               = NO;


BOOL16  g_ptdoa              = NO;



short   g_trel[10] = {
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE,
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE
};


/* Ghidra sprite_layer_flags @ 0x2b6ee: entries 0,1 = SPRITE_IN_FRONT (1),
   rest = SPRITE_HIDDEN (0).  These are the two dog slot flags (slots
   0 and 7 in the hardware layout, per sp_upds). */
short   g_selaf[SPRITE_SLOTS] = { 1, 1 };


/* Ghidra sprite_slot_map @ 0x2b766: which hardware slot each logical
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



/* bm32or[i] = 1<<i, bm32and[i] = ~(1<<i).  LCP_STX has no builder for
   these -- there is not a single `not.l` in its whole text -- because
   it ships them as initialized DATA instead.
   addr: bm32or, bm32and */

/* LCP_STX ships both tables as initialized DATA rather than
   building them at run time. */
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


/* Ghidra dog_destination_position_table @ 0x2B8DE, 10 HOUSE_POS
   entries the dog picks (via rndRng) as its next wander target.
   Last two duplicate POS_BTM_SCREEN_EDGE so it's picked with 2x
   probability -- the dog favours wandering off-screen. */
short   g_ddipt[10] = {
        POS_TOP_LIVING_ROOM,       POS_TOP_GAME_CHAIR_RIGHT,
        POS_TOP_FIREPLACE_RIGHT,   POS_MID_BEDROOM_WALK,
        POS_MID_COMPUTER_DESK,     POS_BTM_STAIR_LANDING,
        POS_BTM_DOG_BOWL,          POS_BTM_WATER_TAP,
        POS_BTM_SCREEN_EDGE,       POS_BTM_SCREEN_EDGE
};


/* Ghidra g_dgitx @ 0x2b8f0 = POS_BTM_SCREEN_EDGE.  Used by cutscene
   at startup to seed the dog's first wander target -- the dog walks
   in from the bottom-screen edge. */
short   g_dgitx        = POS_BTM_SCREEN_EDGE;


short   g_ddyot[10]     = { 3, 9, 2, 10, 6, 0, 0, 11, 3, 3 };


/* Ghidra g_dgiyo @ 0x2b904 = 3.  Y micro-nudge applied
   to the initial dog target position. */
short   g_dgiyo            = 3;


/* Ghidra dog_dest_x_offset_table @ 0x2B906, dog_dest_y_offset_table
   @ 0x2B8F2 (10 shorts each): per-destination pixel nudges applied
   after hs_posXY returns the anchor for the destination. */
short   g_ddxot[10]     = { 0, 0, 0, 0, 10, 0, 0, 0, 0, 0 };


short   g_dseat[3]   = {
        SPRITE_DOG_EATING_1, SPRITE_DOG_EATING_2, SPRITE_DOG_EATING_3
};



/* Animation frame tables consumed by gameTick.  Every
   value is an object_tab_mfdb index; game_tick indexes these by a
   small counter to pick which sprite/frame to draw. */
/* Object-animation frame tables (dumped from Ghidra data segment).
   The previous port assignments were SCRAMBLED across each other:
   g_obala had fire[0..1], g_obpha had alarm+phone[0..1], g_obfia had
   phone[0..3].  Every od_draw of these tables drew the wrong sprite. */
short   g_obcla[4]     = { OBJ_CLOCK_1, OBJ_CLOCK_2,
                           OBJ_CLOCK_1, OBJ_CLOCK_3 };          /* clock_animation @ 0x2B922 */


short   g_obala[2]     = { OBJ_ALARM_1, OBJ_ALARM_2 };          /* alarm_animation @ 0x2B92A */


short   g_obpha[4]     = { OBJ_PHONE_2, OBJ_PHONE_1,
                           OBJ_PHONE_2, OBJ_PHONE_3 };          /* phone_animation @ 0x2B92E */


short   g_obfia[4]     = { OBJ_FIRE_1, OBJ_FIRE_2,
                           OBJ_FIRE_3, OBJ_FIRE_4 };            /* fire_animation  @ 0x2B936 */



/* Petting-dog sprite frames -- sprite ids the petting animation
   cycles through: ping-pong over frames 1..6 back down to 2.  TEN
   entries, with no trailing SPRITE_PET_HAND_1 and no 0 terminator --
   LCP_STX's data gap here is 20 bytes. */
short   g_ptdsi[10]    = {
        SPRITE_PET_HAND_1, SPRITE_PET_HAND_2, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_4, SPRITE_PET_HAND_5, SPRITE_PET_HAND_6,
        SPRITE_PET_HAND_5, SPRITE_PET_HAND_4, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_2
};



/* Frame-state globals for the animation loop.  8-char-safe port names.
   g_ptanf (petting_anim_frame) already lives in globals.c; the rest
   are added here to keep globals.c under Alcyon's symbol-table
   limit. */
/* Ghidra petting_last_sprite_slot @ 0x2b952 = SPRITE_PET_HAND_1 (0x1b). */
short   g_ptlss                         = SPRITE_PET_HAND_1;


short   g_obdea[3]     = { OBJ_DOG_FOOD_BOWL_3,
                           OBJ_DOG_FOOD_BOWL_2,
                           OBJ_DOG_FOOD_BOWL_1 };  /* ROM 0x13584 */
