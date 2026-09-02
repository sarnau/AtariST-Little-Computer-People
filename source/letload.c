/*
 * letload.c -- decompress LETTER.TXT into g_ltlp[] for a_writl().
 *
 * On-disk format:
 *   +0    short   uncompressed_size + 0x11 header bytes
 *   +2    byte    comp_tok[15]  (15 most common bytes)
 *   +17   ...     compressed body (nibble stream; 15 = literal byte escape)
 *
 * Fidelity note: the 1985 code passes the *advanced* fbuffer to Mfree;
 * Alcyon's allocator tolerated that, modern free(3) traps.  We stash
 * the original pointer in fbuffer_orig.
 *
 * addr: fr_reac(), fl_ltpl()
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "globals.h"
#include "letload.h"
#include "save.h"


/* fr_reac -> parts/fr_reac.c (STX: 0x53b8, in the 0x400c object ahead of main). */
#ifdef FAITHFUL
#include "parts/fr_reac.c"
#endif

/* fl_ltpl -> parts/fl_ltpl.c (STX: 0x648c, just before cpyScr). */
#ifdef FAITHFUL
#include "parts/fl_ltpl.c"
#endif
