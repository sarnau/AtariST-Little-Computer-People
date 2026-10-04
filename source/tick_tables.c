/*
 * tick_tables.c -- animation frame tables + state globals for
 * gameTick (see tick.c).  Kept separate from globals.c
 * so Alcyon C168's fixed-size symbol table doesn't overflow.
 *
 * The state globals are runtime animation counters, kept in BSS.
 */

#include "types.h"
#include "enums.h"
#include "tick_tables.h"
BOOL16  g_alsts;   /* alarm sound has started */
short   g_phrc;    /* phone ring countdown */
/* g_srsdc (screen scroll-down count) lives in globals.c. */
