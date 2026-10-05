/*
 * dat_games4.c -- poker's draw template.
 *
 * pkrMsgTake's string lands between "You're so lucky!!!" and "I'll stay!"
 * in the original's literal pool, which puts the declaration just
 * ahead of pkrCompDraw, its only user.  Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

/* The resident's draw announcement: pkrCompDraw writes the count into
   [10] and makes the ending "card." or "cards." from [16]. */
char *          pkrMsgTake    = "I'll take _ cards.";
