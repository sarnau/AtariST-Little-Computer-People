/*
 * sfx_irq.c -- SFX playback tick (XBIOS Dosound driver).
 *
 * sf_irqp is called from the 8Hz game loop (in
 * sc_ren8) whenever g_sfacf has been raised
 * by sf_sele.  It's the "commit" step of the queued-SFX
 * pipeline:
 *
 *   sf_sele   -> queue a candidate + set active_flag
 *   sf_irqp -> if allowed by priority + MIDI state:
 *                             copy Dosound bytes into the DMA buffer,
 *                             fire XBIOS Dosound, arm the countdown
 *
 * Playback is gated three ways:
 *   1. mi_play -- MIDI takes exclusive PSG when running.
 *   2. g_sfplf -- if another SFX is currently on,
 *      only preempt if the new one has higher priority (lower value).
 *   3. g_sfcup tracks the priority of whatever's
 *      currently playing, so priority-tied SFX don't fight.
 *
 * Duration is either taken from the SFX's own 4-byte trailer (last
 * word=hi, second-last=lo of a 32-bit tick count divided by 25 to
 * convert Dosound envelope time to game 8Hz ticks) or overridden by
 * g_sfdur when the caller pinned it to a specific value.
 *
 * addr: sf_irqp()
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "sfx_irq.h"
#include "sound.h"


/* CONCAT22: pack two 16-bit shorts into a 32-bit long, high-word
   first.  Ghidra emits this as a macro; we express it as a small
   inline for readability. */
static long
concat22(hi, lo)
short   hi;
short   lo;
{
        return ((long) (unsigned short) hi << 16) |
               (long) (unsigned short) lo;
}

/* Read the 200 Hz clock via GEMDOS Super mode.  Shared by
   sc_ren8 -- but each file keeps its own copy to avoid
   pulling in an internal-linkage helper across translation units.  On
   host builds g_hzlo is 0 so the read is a no-op. */

static short
rd_hz()
{
        void *  saveSSP;
        short   lo;

        saveSSP = (void *) Super(0L);
        lo = g_hzlo;
        Super(saveSSP);
        return lo;
}

/* sf_irqp: commit the queued SFX to the Dosound driver.
   addr: sf_irqp() */

void
sf_irqp()
{
        short           size;
        short           i;
        unsigned char * effectPtr;
        char *          dosound_ptr;
        short           new_priority;
        long            raw_ticks;

        /* MIDI has exclusive PSG access. */
        if (mi_play != NO)
                return;

        new_priority = sf_pri[g_sfcur];

        /* If something's already playing, only interrupt for a strictly
           higher-priority effect (lower priority number). */
        if (g_sfplf != NO) {
                if (g_sfcup < new_priority)
                        return;
                sf_so();
        }
        g_sfcup = new_priority;
        g_sfplf     = YES;

        /* Fetch the SFX's Dosound sequence.  Layout:
             +0..1     size (short): number of bytes that follow
             +2..N     Dosound register-command stream
             (trailer) last 4 bytes = duration high/low words */
        effectPtr = mi_ntLp[g_sfcur];
        size      = *(short *) effectPtr;
        effectPtr = effectPtr + 2;

        /* Copy the sequence into the DMA-friendly buffer.  The 1985
           code walks the pointer past the end of the copy, which is
           fine because we know the trailing 4 bytes are the duration
           block we're about to overwrite with the Dosound terminator. */
        dosound_ptr = g_sfDoB;
        for (i = 0; i < size; i = i + 1) {
                *dosound_ptr = (char) *effectPtr;
                effectPtr    = effectPtr    + 1;
                dosound_ptr  = dosound_ptr  + 1;
        }

        /* Overwrite the last 4 bytes with the Dosound terminator
           sequence (0x00 0x00 0x00 0x00 = "stop").  The 1985 duration
           bytes we just copied are then read back from *before* the
           terminator via effectPtr[-4..-1]. */
        dosound_ptr[-4] = 0;
        dosound_ptr[-3] = 0;
        dosound_ptr[-2] = 0;
        dosound_ptr[-1] = 0;

        g_sfddh = *(short *) (effectPtr - 4);
        g_sfddl = *(short *) (effectPtr - 2);
        g_sfpli          = g_sfcur;

        /* Hand the sequence to the XBIOS Dosound player.  Runs from
           the VBL interrupt on the ST; no-op on the host. */
        Dosound(g_sfDoB);

        /* Snapshot the 200 Hz counter for the countdown loop and
           compute the remaining ticks in game 8Hz units (Dosound
           envelope time / 25 = game ticks, since 200 Hz / 25 = 8 Hz). */
        g_sfHz2 = (long) (unsigned short) rd_hz();
        raw_ticks = concat22(g_sfddh,
                             g_sfddl);
        g_sfret = raw_ticks / 25L;

        /* Caller-pinned duration overrides the Dosound-derived one.
           Signal value -1 means "use the auto-computed duration". */
        if (g_sfdur != -1)
                g_sfret = (long) g_sfdur;
}
