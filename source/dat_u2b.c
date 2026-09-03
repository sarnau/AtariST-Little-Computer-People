/*
 * dat_u2b.c -- one global from the middle of stx_u2's data.
 *
 * Alcyon emits a string literal into the data segment where it first
 * meets it, so the literal pool follows the unit's source order.  In
 * the reference, g_ltg's four sign-off strings sit between "*.sng"
 * (a_lists, 0x1398c) and "%s %d, %4d" (a_writl, 0x13cd6), which puts
 * the declaration between those two functions rather than at the head
 * of the unit with the other globals -- the 1985 habit of declaring a
 * global just above its only user.  g_ltcwt follows it here because
 * the reference puts g_ltg's pointers at 0x1eda and g_ltcwt at
 * 0x1eea: whatever order the .data definitions come in, they come in
 * source order, so g_ltcwt has to be declared after this point too.
 * Never compiled standalone.
 */

#include "types.h"

/* g_ltg[4]: the letter sign-off a_writl picks at random.  The four
   pointers are real here -- the reference relocates all of them, to
   strings this object emits after "*.sng". */
char *  g_ltg[4]        = {
        "Sincerely,", "Cordially,", "Yours Truly,", "Love,"
};

/* g_ltcwt[4]: sprite IDs used to hide previously-typed
   characters as the buffer position advances (SPRITE_TYPING_1..4). */
short   g_ltcwt[4]      = {
        SPRITE_TYPING_1, SPRITE_TYPING_2,
        SPRITE_TYPING_3, SPRITE_TYPING_4
};
