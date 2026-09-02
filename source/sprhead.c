/*
 * sprhead.c -- LCP head-animation state machine.
 * g_hacur/g_hatas are 8-bit direction codes:
 *   bits 0..2 horizontal angle 0..7, bits 3..4 vertical tilt 0..3,
 *   bit 7 set = "no active target" (idle).
 * addr: sp_lcha()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "globals.h"
#include "random.h"
#include "sprglobs.h"
#include "sprhead.h"


/* Bit-fields inside g_hamod.  Distinct from HEAD_ANIM_* target-state
   constants in enums.h (Ghidra collapsed both meanings on decompile). */
#define HEAD_MODE_H_AMPLITUDE           0x07
#define HEAD_MODE_H_RANGE               0x08
#define HEAD_MODE_V_RANGE               0x60
#define HEAD_MODE_V_OVERRIDE            0x80

/* sp_lcha -> parts/sp_lcha.c (STX: 0x16368, between sp_updb and sp_lchu). */
#ifdef FAITHFUL
#include "parts/sp_lcha.c"
#endif
