/* sfx_irq.c -- SFX playback tick (XBIOS Dosound driver). */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "protos.h"

/* startSfx builds the 32-bit duration from its two halves and keeps
   its own Super block inline, rather than calling helpers. */
void
startSfx()
{
        /* Declaration order fixes the stack frame; `unused` is never
           read or written but must stay for the same reason. */
        char *          effectPtr;
        char *          dosound_ptr;
        short           size;
        short           i;
        long            raw_lo;
        short *         hz_ptr;
        short           unused;
        unsigned short  hz;
        long            ssp;

        /* MIDI has exclusive PSG access. */
        if (songPlaying != NO)
                return;

        /* Only preempt for a strictly higher priority (lower number);
           the priority table is re-read for the store. */
        if (sfxPlaying != NO) {
                if (sfxPriority[sfxReqId] > sfxCurPrio)
                        return;
                stopSfx();
        }
        sfxCurPrio = sfxPriority[sfxReqId];
        sfxPlaying = YES;

        /* SFX layout: +0..1 size, +2..N Dosound stream,
           trailing 4 bytes = duration hi/lo words. */
        size      = *(short *) sfxData[sfxReqId];
        effectPtr = sfxData[sfxReqId] + 2;

        dosound_ptr = sfxBuffer;
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
        sfxDurHi = *(short *) effectPtr;
        effectPtr += 2;
        sfxDurLo = *(short *) effectPtr;
        sfxCurId = sfxReqId;

        Dosound(sfxBuffer);

        /* Convert Dosound envelope time (200 Hz) to 8 Hz game ticks;
           the 200 Hz counter is read inline under Super. */
        hz_ptr = (short *) 0x4bcL;
        ssp = Super(0L);
        hz = *hz_ptr;
        Super(ssp);
        sfxStartHz = hz & 0xffffL;

        sfxTicksLeft = sfxDurHi;
        sfxTicksLeft = (sfxTicksLeft << 16) & 0xffff0000L;
        raw_lo = (long) sfxDurLo & 0xffffL;
        sfxTicksLeft |= raw_lo;
        sfxTicksLeft = sfxTicksLeft / 25L;

        /* -1 = use the auto-computed duration. */
        if (sfxReqDur != -1)
                sfxTicksLeft = sfxReqDur;
}
