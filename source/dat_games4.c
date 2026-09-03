/*
 * dat_games4.c -- poker's draw template.
 *
 * pk_tcm's string lands between "You're so lucky!!!" and "I'll stay!"
 * in the reference's literal pool, which puts the declaration just
 * ahead of pk_cdrw, its only user.  Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

char *          pk_tcm    = "I'll take _ cards.";
