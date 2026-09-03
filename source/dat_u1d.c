/*
 * dat_u1d.c -- the PEx.LCP filename, declared where the reference
 * declares it: after ldSpr and before main.
 *
 * A compilation unit's string bodies are emitted in the order c168
 * meets them, so the pool records the declaration's position in the
 * source even when the variable itself is a pointer sitting back in
 * the globals region.  Here the pool runs cntSong's "*.sng"/"*.org",
 * ldObj's "objects", ldSpr's "sprites", this string, then main's
 * "data"/"house.scn"/"body.lcp".  Never compiled standalone.
 */

#include "types.h"

/* The PEx.LCP filename, lowercase, with main() poking index 2 to pick
   the character.  It is the last of this object's globals, so moving
   the declaration here does not disturb their order -- but the string
   body moves with it, into the pool between ldSpr's "sprites" and
   main's "data". */
char *  pex_name                     = "pex.lcp";
