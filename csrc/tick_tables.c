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
                           OBJ_DOG_FOOD_BOWL_1 };               /* dog_eating_animation @ 0x2B954 */

/* Petting-dog sprite frames -- 11-frame array of sprite ids the
   petting animation cycles through (Ghidra sprite_id array
   @ 0x2B93E; ping-pong pattern over frames 1..6). */
short   g_ptdsi[11]    = {
        SPRITE_PET_HAND_1, SPRITE_PET_HAND_2, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_4, SPRITE_PET_HAND_5, SPRITE_PET_HAND_6,
        SPRITE_PET_HAND_5, SPRITE_PET_HAND_4, SPRITE_PET_HAND_3,
        SPRITE_PET_HAND_2, SPRITE_PET_HAND_1
};

/* Carried-object jump table (Ghidra carried_object_id_table @ 0x2B95A):
   long[10] indexed by lcp_carried_object matching one of {SPRITE_GLASS,
   SPRITE_GAME_BOX, ...}.  Each entry holds a sprite_id in the low
   word; high word is always 0 (Ghidra: `long`, not `short`).
   NOTE: the port dispatches via the cy_yoff() switch in tick.c; this
   table itself is not read by any C code -- kept for byte-fidelity. */
long    g_cotbl[10]    = {
        SPRITE_GLASS, SPRITE_GAME_BOX, SPRITE_FOOD_PACKAGE,
        SPRITE_FIREWOOD, SPRITE_COOKING_POT,
        SPRITE_SUITCASE, SPRITE_BOOK, SPRITE_VINYL_CARRY,
        SPRITE_COOKED_MEAL, 0
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
