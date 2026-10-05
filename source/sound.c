/*
 * sound.c -- SFX queue + Dosound driver + .SNG song loader.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "protos.h"
#include "globals.h"

/* One-line SFX wrappers.  K&R style (Alcyon 4.14). */

/* The four SFX wrappers sit right after sayHello, in the order tvc,
   spe, hnd, grt -- stx_u2.c includes them there.  playDoorbell, typeKeySound
   and sfxClick live in parts/ too. */

/* loadSounds: the SOUNDS.LCP block loader.  Each block is a 2-byte size
   followed by its payload; a size of 0 ends the file.  Every block
   gets its own Malloc of size + 4, stored in sfxData[index], with the
   size in its first word and the payload behind it.  The details --
   the result stored into sfxData and read back, `(long) size + 4`
   widening, block++ before the payload read -- are the original's
   shape and must stay. */
void
loadSounds()
{
        short           fhandle;
        short           size;
        short *         block;
        short           index;

        fhandle = openFile("sounds.lcp", RMODE_RD);
        for (index = 0; index < 500; index++) {
                readFile(fhandle, 2L, &size);
                if (size == 0)
                        break;
                sfxData[index] = (unsigned char *) Malloc((long) size + 4);
                block = (short *) sfxData[index];
                if (block == (short *) 0)
                        outOfMemory();
                *block = size;
                block++;
                readFile(fhandle, (long) size, block);
        }
        Fclose(fhandle);
}

/* Lower priority value wins. */
void
sfxSelect(sound_id, duration)
short   sound_id;
long    duration;
{
        if (sfxPending == NO ||
            sfxPriority[sfxReqId] >=
            sfxPriority[sound_id]) {
                sfxReqId = sound_id;
                sfxReqDur = (short) duration;
                sfxPending = YES;
        }
}

/* Silence the three PSG channels and mark no effect playing. */
void
stopSfx()
{
        Giaccess(0, PSG_WRITE | PSG_VOL_A);
        Giaccess(0, PSG_WRITE | PSG_VOL_B);
        Giaccess(0, PSG_WRITE | PSG_VOL_C);
        sfxDosStat = 0xff;
        sfxDosCtl = 0;
        sfxPlaying = NO;
}

