/*
 * sprender.c -- masked-blit sprite renderer (two-pass NOTS_AND_D + S_XOR_D).
 * addr: sp_draw(), sp_iniM()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include "obdefs1.h"
#include "globals.h"
#include "sprender.h"
#include "vdiown.h"
#include "sprglobs.h"

/* sp_iniM -> parts/sp_iniM.c (STX: 0x6612, right after stpScrB). */

/* sp_draw -> parts/sp_draw.c (STX 0x1605c, in the sprite object;
   stx_u3.c includes it there). */
