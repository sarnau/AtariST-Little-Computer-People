/*
 * psg_io.c -- ST hardware register writes (MIDI ACIA + YM2149 PSG).
 */

#include "types.h"
#include "structs.h"
#include "st_io.h"
#include "psg_io.h"

#ifdef HOST
/* Host scratch bytes -- each hardware register aliases one of these
   via #defines in st_io.h.  volatile keeps the compiler from optimising
   the writes away. */
volatile unsigned char  g_hmc    = 2;    /* TDRE always set */
volatile unsigned char  g_hms       = 0;
volatile unsigned char  g_hgis   = 0;
volatile unsigned char  g_hgiw    = 0;
#endif

/* mowrit: poll ACIA TDRE (bit 1) then write one byte.  On host, TDRE
   is preseeded to 1 so the poll returns immediately.  Hand-assembly
   (psg_asm.s). */

/* psg_cpE -> parts/psg_cpE.c. */

/* psg_wr: YM2149 two-stage latch -- select the register, then write
   the value.  Hand-assembly (psg_asm.s). */

/* psg_mix: read-modify-write on YM2149 mixer register 7.
   Hand-assembly (psg_asm.s). */
