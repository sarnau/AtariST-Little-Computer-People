/*
 * dat_u2b.c -- one global from the middle of stx_u2's data.
 *
 * Alcyon emits a string literal into the data segment where it first
 * meets it, so the literal pool follows the unit's source order.  In
 * the reference, g_ltg's four sign-off strings sit between "*.sng"
 * (a_lists, 0x1398c) and "%s %d, %4d" (a_writl, 0x13cd6), which puts
 * the declaration between those two functions rather than at the head
 * of the unit with the other globals -- the 1985 habit of declaring a
 * global just above its only user.  Never compiled standalone.
 */

#include "types.h"

/* g_ltg[4]: the letter sign-off a_writl picks at random.  The four
   pointers are real here -- the reference relocates all of them, to
   strings this object emits after "*.sng". */
char *  g_ltg[4]        = {
        "Sincerely,", "Cordially,", "Yours Truly,", "Love,"
};
