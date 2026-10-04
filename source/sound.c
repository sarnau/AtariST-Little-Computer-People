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

/* The four SFX wrappers sit right after a_hello, in the order tvc,
   spe, hnd, grt -- stx_u2.c includes them there.  p_dobls, lt_sets
   and sfClick live in parts/ too. */

/* sf_sl: the SOUNDS.LCP block loader.  Each block is a 2-byte size
   followed by its payload; a size of 0 ends the file.  Every block
   gets its own Malloc of size + 4, stored in mi_ntLp[index], with the
   size in its first word and the payload behind it.  The details --
   the result stored into mi_ntLp and read back, `(long) size + 4`
   widening, block++ before the payload read -- are the original's
   shape and must stay. */
void
sf_sl()
{
        short           fhandle;
        short           size;
        short *         block;
        short           index;

        fhandle = fOpen("sounds.lcp", RMODE_RD);
        for (index = 0; index < 500; index++) {
                fr_read(fhandle, 2L, &size);
                if (size == 0)
                        break;
                mi_ntLp[index] = (unsigned char *) Malloc((long) size + 4);
                block = (short *) mi_ntLp[index];
                if (block == (short *) 0)
                        er_nomem();
                *block = size;
                block++;
                fr_read(fhandle, (long) size, block);
        }
        Fclose(fhandle);
}

/* Lower priority value wins. */
void
sf_sele(sound_id, duration)
short   sound_id;
long    duration;
{
        if (g_sfacf == NO ||
            sf_pri[g_sfcur] >=
            sf_pri[sound_id]) {
                g_sfcur     = sound_id;
                g_sfdur    = (short) duration;
                g_sfacf = YES;
        }
}

/* Silence the three PSG channels and mark no effect playing. */
void
sf_so()
{
        Giaccess(0, PSG_WRITE | PSG_VOL_A);
        Giaccess(0, PSG_WRITE | PSG_VOL_B);
        Giaccess(0, PSG_WRITE | PSG_VOL_C);
        g_sfdos  = 0xff;
        g_sfdoc = 0;
        g_sfplf    = NO;
}


/* sgPlay lives in parts/sgPlay.c. */
