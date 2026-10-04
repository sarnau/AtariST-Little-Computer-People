/*
 * letload.c -- decompress LETTER.TXT into g_ltlp[] for a_writl().
 *
 * On-disk format:
 *   +0    short   uncompressed_size + 0x11 header bytes
 *   +2    byte    comp_tok[15]  (15 most common bytes)
 *   +17   ...     compressed body (nibble stream; 15 = literal byte escape)
 *
 * The 1985 code passes the *advanced* fbuffer to Mfree, not the
 * pointer Malloc returned.  Kept as in the original.
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "globals.h"
#include "letload.h"
#include "save.h"


/* fr_reac -> parts/fr_reac.c. */

/* fl_ltpl -> parts/fl_ltpl.c. */
