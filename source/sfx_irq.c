/*
 * sfx_irq.c -- SFX playback tick (XBIOS Dosound driver).
 * addr: sf_irqp()
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "sfx_irq.h"
#include "sound.h"


/* STX has neither a concat22 helper nor rd_hz: sf_irqp writes the
   32-bit duration out of its two halves and keeps its own Super
   block.  Both helpers are gone. */

/* addr: sf_irqp() */
void
sf_irqp()
{
        char *          effectPtr;      /* -4 */
        char *          dosound_ptr;    /* -8 */
        short           size;           /* -10 */
        short           i;              /* -12 */
        long            raw_lo;         /* -16 */
        short *         hz_ptr;         /* -20 */
        short           unused;         /* -22, never read or written */
        unsigned short  hz;             /* -24 */
        long            ssp;            /* -28 */

        /* MIDI has exclusive PSG access. */
        if (mi_play != NO)
                return;

        /* Only preempt for a strictly higher priority (lower number);
           the priority table is re-read for the store. */
        if (g_sfplf != NO) {
                if (sf_pri[g_sfcur] > g_sfcup)
                        return;
                sf_so();
        }
        g_sfcup = sf_pri[g_sfcur];
        g_sfplf = YES;

        /* SFX layout: +0..1 size, +2..N Dosound stream,
           trailing 4 bytes = duration hi/lo words. */
        size      = *(short *) mi_ntLp[g_sfcur];
        effectPtr = mi_ntLp[g_sfcur] + 2;

        dosound_ptr = g_sfDoB;
        for (i = 0; i < size; i++) {
                *dosound_ptr = *effectPtr;
                effectPtr++;
                dosound_ptr++;
        }

        /* Overwrite last 4 bytes with Dosound terminator (0,0,0,0). */
        dosound_ptr -= 4;
        *dosound_ptr++ = 0;
        *dosound_ptr++ = 0;
        *dosound_ptr++ = 0;
        *dosound_ptr = 0;

        effectPtr -= 4;
        g_sfddh = *(short *) effectPtr;
        effectPtr += 2;
        g_sfddl = *(short *) effectPtr;
        g_sfpli = g_sfcur;

        Dosound(g_sfDoB);

        /* Convert Dosound envelope time (200 Hz) to 8 Hz game ticks;
           the 200 Hz counter is read inline under Super. */
        hz_ptr = (short *) 0x4bcL;
        ssp = Super(0L);
        hz = *hz_ptr;
        Super(ssp);
        g_sfHz2 = hz & 0xffffL;

        g_sfret = g_sfddh;
        g_sfret = (g_sfret << 16) & 0xffff0000L;
        raw_lo = (long) g_sfddl & 0xffffL;
        g_sfret |= raw_lo;
        g_sfret = g_sfret / 25L;

        /* -1 = use the auto-computed duration. */
        if (g_sfdur != -1)
                g_sfret = g_sfdur;
}
