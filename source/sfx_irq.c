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


/* Ghidra CONCAT22: hi<<16 | lo. */
static long
concat22(hi, lo)
short   hi;
short   lo;
{
        return ((long) (unsigned short) hi << 16) |
               (long) (unsigned short) lo;
}

/* Read 200 Hz counter via Super mode. */
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

/* addr: sf_irqp() */
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

        /* Only preempt for a strictly higher priority (lower number). */
        if (g_sfplf != NO) {
                if (g_sfcup < new_priority)
                        return;
                sf_so();
        }
        g_sfcup = new_priority;
        g_sfplf     = YES;

        /* SFX layout: +0..1 size, +2..N Dosound stream,
           trailing 4 bytes = duration hi/lo words. */
        effectPtr = mi_ntLp[g_sfcur];
        size      = *(short *) effectPtr;
        effectPtr = effectPtr + 2;

        dosound_ptr = g_sfDoB;
        for (i = 0; i < size; i = i + 1) {
                *dosound_ptr = (char) *effectPtr;
                effectPtr    = effectPtr    + 1;
                dosound_ptr  = dosound_ptr  + 1;
        }

        /* Overwrite last 4 bytes with Dosound terminator (0,0,0,0). */
        dosound_ptr[-4] = 0;
        dosound_ptr[-3] = 0;
        dosound_ptr[-2] = 0;
        dosound_ptr[-1] = 0;

        g_sfddh = *(short *) (effectPtr - 4);
        g_sfddl = *(short *) (effectPtr - 2);
        g_sfpli          = g_sfcur;

        Dosound(g_sfDoB);

        /* Convert Dosound envelope time (200 Hz) to 8 Hz game ticks. */
        g_sfHz2 = (long) (unsigned short) rd_hz();
        raw_ticks = concat22(g_sfddh,
                             g_sfddl);
        g_sfret = raw_ticks / 25L;

        /* -1 = use the auto-computed duration. */
        if (g_sfdur != -1)
                g_sfret = (long) g_sfdur;
}
