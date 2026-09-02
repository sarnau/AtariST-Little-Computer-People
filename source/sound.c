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

/* Lower priority value wins.
   addr: sf_sele() */
void
sf_sele(sound_id, duration)
short   sound_id;
long    duration;
{
        if (g_sfacf == NO ||
            sf_pri[sound_id] <=
            sf_pri[g_sfcur]) {
                g_sfcur     = sound_id;
                g_sfdur    = (short) duration;
                g_sfacf = YES;
        }
}

/* addr: sf_so() (ROM 0xb122) */
void
sf_so()
{
        Giaccess(0L, 0x88L);
        Giaccess(0L, 0x89L);
        Giaccess(0L, 0x8aL);
        g_sfdos  = 0xff;
        g_sfdoc = 0;
        g_sfplf    = NO;
}

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
void p_dobls() { sf_sele(SFX_DOORBELL,  4L); }

/* addr: lt_sets(), sfClick() */
void
lt_sets()
{
        sf_sele(SFX_TYPEWRITER_KEY, 4L);
}

void
sfClick()
{
        sf_sele(SFX_CLICK, 2L);
}

/* sgPlay: load a .sng/.org from disk (10-byte Music Studio 2.0 header,
   then up to 20000 bytes of sequence data) and hand it to mq_inis.
   addr: sgPlay() */


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

void
sgPlay(filename)
char *  filename;
{
        _DTA *   dta_ptr;
        short           fhnd;
        unsigned char   temp[10];

        g_molof = YES;
        mi_varR          = YES;

        if (mi_play != NO) {
                mq_inis(mi_sbuf, g_momap);
                while (mi_play != NO)
                        ;
        }
        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }

        Fsfirst(filename, 0L);
        dta_ptr = (_DTA *) Fgetdta();
        mi_sbuf = (char *) Malloc(dta_ptr->d_length);
        if (mi_sbuf == (char *) 0)
                er_nomem();

        fhnd = fOpen(filename, 0);
        if (fhnd >= 0) {
                fr_read(fhnd, 10L, temp);
                fr_read(fhnd, 20000L, mi_sbuf);
                Fclose(fhnd);
        }
        mq_inis(mi_sbuf, g_momap);
}
