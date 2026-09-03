/*
 * psg_io.c -- ST hardware register writes (MIDI ACIA + YM2149 PSG).
 * addr: mowrit(), psg_cpE(), psg_wr(), psg_mix()
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

/* Poll ACIA TDRE (bit 1) then write one byte.  On host, TDRE is
   preseeded to 1 so the poll returns immediately.
   addr: mowrit() */
/* mowrit: LCP_STX has this as hand-assembly (psg_asm.s). */

/* psg_cpE -> parts/psg_cpE.c (STX: 0x1586, in the MIDI object). */

/* ST quirk: the 1985 source stores `val` into giselect and `reg` into
   giwrite (YM2149 two-stage latch).  Preserved verbatim.
   addr: psg_wr() */
/* psg_wr: LCP_STX has this as hand-assembly (psg_asm.s). */

/* Read-modify-write on YM2149 mixer register 7.
   addr: psg_mix() */
/* psg_mix: LCP_STX has this as hand-assembly (psg_asm.s). */
