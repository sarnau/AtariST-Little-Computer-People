/*
 * dat_games3.c -- poker's bet and raise templates.
 *
 * Their strings are the last two before pk_main's "Do you feel lucky
 * today?", so the declarations sit immediately ahead of pk_main.
 * Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

char *          pk_rm     = "I'll raise __.";

/* Editable poker prompts, patched in place before each is shown.  The
   underscores are the digit slots the original ships -- pk_dbet and
   pk_dppm overwrite the two in pk_bm/pk_rm, pk_cdrw the one in
   pk_tcm (and the trailing "." becomes "s." for a plural draw).
   They are POINTERS, not arrays, so every patch goes through a
   movea.l of the variable first. */
char *          pk_bm     = "I'll bet __.";
