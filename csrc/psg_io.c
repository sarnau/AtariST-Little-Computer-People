/*
 * psg_io.c -- ST hardware register writes (MIDI ACIA + YM2149 PSG).
 *
 * All four functions are direct memory-mapped register writes with no
 * game logic.  On the ST target they touch the real 68901 MFP-adjacent
 * ACIA and the YM2149 chip; on the host they hit scratch bytes defined
 * below so the compilable-on-host promise holds without any observable
 * audio effect.
 *
 * addr: mowrit(), psg_copy_envelope_params(),
 *       psg_write_register(), psg_set_mixer()
 */

#include "types.h"
#include "structs.h"
#include "st_io.h"

#ifdef HOST
/* Host scratch bytes -- each hardware register aliases one of these
   via #defines in st_io.h.  volatile keeps the compiler from optimising
   the writes away. */
volatile unsigned char  g_hmc    = 2;    /* TDRE always set */
volatile unsigned char  g_hms       = 0;
volatile unsigned char  g_hgis   = 0;
volatile unsigned char  g_hgiw    = 0;
#endif

/* mowrit: poll the ACIA status register for
   TDRE (Transmit Data Register Empty, bit 1), then write one byte
   to the data register.  Used by the "direct write" path of
   mq_dise when the sequencer is in speed-critical
   mode (bypassing the XBIOS Midiws trap).

   On the host the TDRE bit is preseeded to 1 (see g_hmc
   above) so the poll returns immediately -- otherwise host builds
   would spin forever.

   addr: mowrit() */

void
mowrit(byte)
char    byte;
{
        unsigned char   status;

        do {
                status = midictl;
        } while ((status & 2) == 0);
        midi = byte;
}

/* psg_copy_envelope_params: 8-byte memcpy from an ADSR parameter block
   in the .SNG file to a PSG_ENVELOPE struct.  The 1985 code implements
   this inline rather than calling libc memcpy(3), which makes sense
   given Alcyon's minimal runtime -- the 8-byte block is small enough
   that the unrolled loop is comparable in size to a memcpy call.
   Preserved verbatim.

   addr: psg_copy_envelope_params() */

void
psg_copy_envelope_params(src, dest, count)
unsigned char * src;
unsigned char * dest;
short           count;
{
        while (count != 0) {
                *dest = *src;
                src   = src  + 1;
                dest  = dest + 1;
                count = count - 1;
        }
}

/* psg_write_register: write to a YM2149 register.

   ST quirk: the 1985 source has the argument order swapped relative
   to typical documentation -- `reg` is the register number, `val` is
   the data.  But the writes below store `val` into giselect and `reg`
   into giwrite.  Preserved verbatim; on the real ST the YM2149 latches
   the value written to giselect and then reads it back through giwrite
   in a two-stage sequence, so the apparent swap is actually correct
   for the hardware.

   addr: psg_write_register() */

void
psg_write_register(reg, val)
char    reg;
char    val;
{
        giselect = (unsigned char) val;
        giwrite  = (unsigned char) reg;
}

/* psg_set_mixer: read-modify-write on YM2149 mixer register 7.
   Selects register 7 by writing to giselect, reads the current value
   back through giselect (the register acts as both write-address and
   read-data), then applies (or_mask | (and_mask & current)) and
   writes the result via giwrite.

   Note the double read of giselect in the 1985 code (`giselect = 7;
   bVar1 = giselect;`) -- the first write latches the register
   selection, the second read pulls the current register value.  On the
   host this collapses to a scratch-byte read that returns whatever was
   written last, matching the ST behaviour.

   addr: psg_set_mixer() */

void
psg_set_mixer(or_mask, and_mask)
char    or_mask;
char    and_mask;
{
        unsigned char   current;

        giselect = 7;
        current  = giselect;
        giwrite  = (unsigned char)
                (or_mask | (and_mask & current));
}
