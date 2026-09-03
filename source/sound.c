/*
 * sound.c -- SFX queue + Dosound driver + .SNG song loader.
 * addr: sf_sele(), sf_so(), sf_sl(), sgPlay()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "globals.h"
#include "midi_seq.h"
#include "save.h"
#include "sound.h"

/* One-line SFX wrappers.  K&R style (Alcyon 4.14). */

/* The four SFX wrappers sit immediately after a_hello in the 0xdece
   object (a_hello reaches each with a bsr), in the order tvc, spe,
   hnd, grt -- stx_u2.c includes them there. */
/* p_dobls -> parts/p_dobls.c (STX: 0x15f9a, after deal_kc). */

/* lt_sets -> parts/lt_sets.c (STX: 0x1476c, immediately before sfClick). */

/* sfClick -> parts/sfClick.c (STX: 0xdece object, 0x14786, just before lcp_wkD). */

/* sf_sl (0xdcc4): the SOUNDS.LCP block loader -- fr_read sizes, a
   Malloc per block, the result stored into mi_ntLp[index] and read
   back, `(long) size + 4` widening, and the buffer walked with
   block++ before the payload read. */
void
sf_sl()
{
        short           fhandle;
        short           size;
        short *         block;
        short           index;

        fhandle = fOpen("sounds.lcp", 0);
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

/* Lower priority value wins.
   addr: sf_sele() */
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

/* addr: sf_so() (ROM 0xb122) */
void
sf_so()
{
        Giaccess(0, 0x88);
        Giaccess(0, 0x89);
        Giaccess(0, 0x8a);
        g_sfdos  = 0xff;
        g_sfdoc = 0;
        g_sfplf    = NO;
}


/* sgPlay -> parts/sgPlay.c (STX: 0xd9ea, ahead of sf_irqp). */
