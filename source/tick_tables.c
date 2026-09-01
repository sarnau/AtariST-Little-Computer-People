/*
 * tick_tables.c -- animation frame tables + state globals for
 * gameTick (see tick.c).  Kept separate from globals.c
 * so Alcyon C168's fixed-size symbol table doesn't overflow.
 *
 * addr: (data-segment tables sourced from Ghidra addresses noted per
 * variable; state globals track runtime animation counters that the
 * 1985 binary stores in BSS).
 */

#include "types.h"
#include "enums.h"
#include "tick_tables.h"

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
short   g_obdea[3]     = { OBJ_DOG_FOOD_BOWL_3,
                           OBJ_DOG_FOOD_BOWL_2,
                           OBJ_DOG_FOOD_BOWL_1 };  /* ROM 0x13584 */

/* Petting-dog sprite frames -- sprite ids the petting animation
   cycles through (ROM data 0x1358a, referenced from tick at
   0xced0/0xceec): ping-pong over frames 1..6 back down to 1,
   closed by a 0 terminator. */
short   g_ptdsi[12]    = {
        SPRITE_PET_HAND_1, SPRITE_PET_HAND_2, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_4, SPRITE_PET_HAND_5, SPRITE_PET_HAND_6,
        SPRITE_PET_HAND_5, SPRITE_PET_HAND_4, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_2, SPRITE_PET_HAND_1, 0
};

/* Carried-object sprite table (Ghidra carried_object_id_table
   @ 0x2B95A; ROM data 0x135a2, 38 bytes): 19 shorts forming
   {sprite_id, 0} pairs closed by a single 0 terminator.
   NOTE: the ROM's per-object dispatch each write the same
   `g_sepey[g_seslm[SPRITE_X]] = lcp_y - 20` with only the stored
   sprite-def index differing; the port collapses this to a single
   inline write in gameTick's carrying-mode positioning block.  Table
   kept here for byte-fidelity to the ROM data segment. */
short   g_cotbl[19]    = {
        SPRITE_GLASS, 0, SPRITE_GAME_BOX, 0,
        SPRITE_FOOD_PACKAGE, 0, SPRITE_FIREWOOD, 0,
        SPRITE_COOKING_POT, 0, SPRITE_SUITCASE, 0,
        SPRITE_BOOK, 0, SPRITE_VINYL_CARRY, 0,
        SPRITE_COOKED_MEAL, 0, 0
};

/* Frame-state globals for the animation loop.  8-char-safe port names.
   g_ptanf (petting_anim_frame) already lives in globals.c; the rest
   are added here to keep globals.c under Alcyon's symbol-table
   limit. */
/* Ghidra petting_last_sprite_slot @ 0x2b952 = SPRITE_PET_HAND_1 (0x1b). */
short   g_ptlss                         = SPRITE_PET_HAND_1;
BOOL16  g_alsts                         = NO;   /* alarm_sound_started */
short   g_phrc                          = 0;    /* phone_ring_countdown */
/* g_srsdc (screen_scroll_down_count) lives in globals.c. */
