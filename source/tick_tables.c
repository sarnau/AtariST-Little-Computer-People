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
BOOL16  g_alsts;   /* alarm_sound_started */
short   g_phrc;    /* phone_ring_countdown */
/* g_srsdc (screen_scroll_down_count) lives in globals.c. */
