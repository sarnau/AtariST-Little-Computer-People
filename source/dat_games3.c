/*
 * dat_games3.c -- poker's bet and raise templates.
 *
 * Their strings are the last two before playPoker's "Do you feel lucky
 * today?", so the declarations sit immediately ahead of playPoker.
 * Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

/* The resident's raise announcement: playPoker writes his raise
   (pk_dpos, two digits, a leading zero blanked) over the underscores
   at [11] and [12] before showing it. */
char *          pk_rm     = "I'll raise __.";

/* Editable poker prompts, patched in place before each is shown.  The
   underscores are the digit slots the original ships -- pkrCallOrRaise and
   dispPlyrChips overwrite the two in pk_bm/pk_rm, pkrCompDraw the one in
   pk_tcm (and the trailing "." becomes "s." for a plural draw).
   They are POINTERS, not arrays, so every patch loads the pointer
   first; declaring them as arrays changes the compiled code. */
char *          pk_bm     = "I'll bet __.";
