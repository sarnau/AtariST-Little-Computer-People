/*
 * dat_u2b.c -- one global from the middle of stx_u2's data.
 *
 * Alcyon emits a string literal into the data segment where it first
 * meets it, so the literal pool follows the unit's source order.  In
 * the original, g_ltg's four sign-off strings sit between "*.sng"
 * (a_lists) and "%s %d, %4d" (a_writl), which puts the declaration
 * between those two functions rather than at the head of the unit with
 * the other globals -- the 1985 habit of declaring a global just above
 * its only user.  g_ltcwt follows g_ltg in the original's data, and
 * .data definitions come out in source order, so g_ltcwt has to be
 * declared after this point too.  Do not reorder.
 * Never compiled standalone.
 */

#include "types.h"

/* g_ltg[4]: the letter sign-off a_writl picks at random.  These are
   real string pointers, emitted after "*.sng" in this unit's pool. */
char *  g_ltg[4]        = {
        "Sincerely,", "Cordially,", "Yours Truly,", "Love,"
};

/* g_ltcwt[4]: sprite IDs used to hide previously-typed
   characters as the buffer position advances (SPRITE_TYPING_1..4). */
short   g_ltcwt[4]      = {
        SPRITE_TYPING_1, SPRITE_TYPING_2,
        SPRITE_TYPING_3, SPRITE_TYPING_4
};
