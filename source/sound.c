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

/* STX orders these p_sftvc, p_sfgrt, p_sfhnd, p_sfspe immediately
   after a_hello in the 0xdece object (a_hello reaches each with a
   bsr) -- see parts/p_sfx.c.  FAITHFUL keeps them here. */
#ifdef FAITHFUL
#include "parts/p_sftvc.c"
#include "parts/p_sfgrt.c"
#include "parts/p_sfspe.c"
#include "parts/p_sfhnd.c"
#endif
/* p_dobls -> parts/p_dobls.c (STX: 0x15f9a, after deal_kc). */
#ifdef FAITHFUL
#include "parts/p_dobls.c"
#endif

/* lt_sets -> parts/lt_sets.c (STX: 0x1476c, immediately before sfClick). */
#ifdef FAITHFUL
#include "parts/lt_sets.c"
#endif

/* sfClick -> parts/sfClick.c (STX: 0xdece object, 0x14786, just before lcp_wkD). */
#ifdef FAITHFUL
#include "parts/sfClick.c"
#endif

/* sf_sl (ROM 0xb234): opens SOUNDS.LCP and immediately closes it --
   nothing in the ROM ever writes the mi_ntLp effect table (its only
   reference is the read in sf_irqp).  That makes the ROM's first
   sound effect a LATENT CRASH: sf_irqp dereferences the NULL
   mi_ntLp[g_sfcur] and bus-errors reading address $0.  Verified
   2026-09-01 by running DATA/LCP_ORG.PRG itself under Hatari
   (--auto, TOS 1.04): identical bus error, op 3d50 at its own
   sf_irqp+0x64.  Presumably the pre-crack cp_main path loaded the
   effect blocks; the cracked dump lost that.

   FAITHFUL keeps the ROM's (broken) shape for byte-identity.  The
   kept build restores the block loader so SFX actually play. */
#ifdef FAITHFUL
void
sf_sl()
{
        short           fhandle;

        fhandle = fOpen("sounds.lcp", 0);
        Fclose(fhandle);
}
#else
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
#endif  /* FAITHFUL */

/* Lower priority value wins.
   addr: sf_sele() */
void
sf_sele(sound_id, duration)
short   sound_id;
long    duration;
{
#ifdef FAITHFUL
        if (g_sfacf == NO ||
            sf_pri[sound_id] <=
            sf_pri[g_sfcur]) {
#else
        if (g_sfacf == NO ||
            sf_pri[g_sfcur] >=
            sf_pri[sound_id]) {
#endif
                g_sfcur     = sound_id;
                g_sfdur    = (short) duration;
                g_sfacf = YES;
        }
}

/* addr: sf_so() (ROM 0xb122) */
void
sf_so()
{
#ifdef FAITHFUL
        Giaccess(0L, 0x88L);
        Giaccess(0L, 0x89L);
        Giaccess(0L, 0x8aL);
#else
        Giaccess(0, 0x88);
        Giaccess(0, 0x89);
        Giaccess(0, 0x8a);
#endif
        g_sfdos  = 0xff;
        g_sfdoc = 0;
        g_sfplf    = NO;
}


/* sgPlay -> parts/sgPlay.c (STX: 0xd9ea, ahead of sf_irqp). */
#ifdef FAITHFUL
#include "parts/sgPlay.c"
#endif
