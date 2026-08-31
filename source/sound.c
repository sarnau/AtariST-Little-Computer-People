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

void p_sftvc() { sf_sele(SFX_TV_CLICK,  2L); }
void p_sfgrt() { sf_sele(SFX_GREETING,  2L); }
void p_sfspe() { sf_sele(SFX_SPEECH,    3L); }
void p_sfhnd() { sf_sele(SFX_HEAD_NOD,  2L); }
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
   the ST original never reads the file and nothing in the ROM ever
   writes the mi_ntLp effect table (its only reference is the read in
   sf_irqp).  The block-loading loop that used to live here was a
   port invention; in the original this call is effectively just a
   "SOUNDS.LCP must exist" check via fOpen's retry-alert loop.
   addr: sf_sl() */
void
sf_sl()
{
        short           fhandle;

        fhandle = fOpen("sounds.lcp", 0);
        Fclose(fhandle);
}

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
