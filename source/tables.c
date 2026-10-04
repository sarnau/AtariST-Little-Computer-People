/* tables.c -- static data tables. */

#include "types.h"
#include "enums.h"
#include "tables.h"

/* rev_tab[256]: 8-bit bit-reversal LUT used to mirror sprites.
   It is NOT shipped as data -- initBRev builds it at boot
   from the two 8-entry bit tables below (mask MSB-first, value
   LSB-first), so the array lives in BSS. */
short           rev_tab[256];
