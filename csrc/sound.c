/*
 * sound.c -- MIDI sequencer + PSG envelope + XBIOS Dosound SFX (stubs).
 *
 * Real port hooks up:
 *   midi_seq_tick_handler   -- 1 kHz timer, plays back .SNG stream
 *   dosound_sfx_start/stop  -- XBIOS Dosound() driver for one-shot SFX
 *   psg_envelope_advance    -- YM2149 envelope stepper
 *
 * All stubs for now so callers link.
 *
 * addr: sf_sele(), sf_so(),
 *       play_soundeffect_*(), rp_anim()
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

/* g_momap declared in globals.h */

/* sf_sele: priority-based SFX queue insertion.  A new SFX
   only wins if no SFX is currently active OR the new priority is <=
   the current priority (lower value = higher priority in the 1985
   convention -- confirmed by the phone-ring-preempts-footstep observed
   behaviour).
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

/* sf_so: silence all 3 PSG channels via XBIOS Giaccess (regs
   0x08/0x09/0x0a with high bit set = write-mode) and reset the Dosound
   sequencer state.
   addr: sf_so() */

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

/* One-line SFX wrappers used by animation code.  Each just picks the
   right (id, duration) pair from Ghidra and hands off to sf_sele.
   Duration units are 8Hz ticks.  Written K&R-style (empty parens,
   no `void`) so Alcyon C 4.14 recognises the definitions and emits
   .globl symbols for the linker. */

void p_sftvc() { sf_sele(SFX_TV_CLICK,  2L); }
void p_sfgrt() { sf_sele(SFX_GREETING,  2L); }
void p_sfspe() { sf_sele(SFX_SPEECH,    3L); }
void p_sfhnd() { sf_sele(SFX_HEAD_NOD,  2L); }
void p_dobls() { sf_sele(SFX_DOORBELL,  4L); }

/* Small SFX wrappers used by the write-letter routine.  Both are
   1-line trampolines into sf_sele with per-effect duration.
   addr: lt_sets(), sfClick() */

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

/* sgPlay: load and start a .sng / .org song file from disk.
   1. If a song is already playing, wait for it to end.
   2. Free any previously-allocated buffer.
   3. Use GEMDOS Fsfirst to fetch the file's DTA (d_length gives size).
   4. Malloc a buffer of that size.
   5. Open the file, skip the 10-byte header, read up to 20000 bytes.
   6. Kick off the sequencer via mq_inis.

   The 20000-byte cap matches the Ghidra source verbatim -- the
   original assumes .sng/.org files stay under that ceiling.

   File-format provenance: the 10-byte header that this loader skips
   is Activision Music Studio's file signature:
     +0..7   "\xCD" + "Mstudio"   (0xCD 4D 73 74 75 64 69 6F)
     +8..9   "\xCD" + version     (0xCD 02 = Music Studio 2.0)
   Every .sng/.org file on the LCP disk is a byte-exact export from
   Activision Music Studio 2.0, verified against files on the Music
   Studio distribution disk -- 9 of 11 songs are bit-identical
   (MYSTERY / PRELUDE / CANON / REQUIEM / AISLEDAN / CALYPSO /
   COUNTRY2 / BALLAD / BOOGIE); BOSSA.SNG differs by one byte at
   offset 0x213 (a single-note edit); STARSPAN.ORG is a shortened
   arrangement of Music Studio's STARSPAN.SNG.  Music Studio was
   published by Activision in 1986 (Ed Bogas / Audio Light) for
   Atari ST / Apple II / C64, designed primarily for Casio's early
   MIDI keyboards (CZ-101, CT-6000).  The .ORG extension on some
   LCP files is cosmetic -- same format, likely renamed during disk
   mastering for the game's category system.
   addr: sgPlay() */


/* sf_sl: load the SOUNDS.LCP sound-effect data file.
   Format: a sequence of records, each `{size:short, dosound_bytes[size]}`,
   terminated by a size=0 record.  Up to 500 records total (Ghidra's
   safety cap; the real file has ~30).

   Each SFX gets its own GEMDOS_Malloc'd block laid out as:
     [0..1]     size (repeated inside the block so sf_irqp
                can read it back via `*(short *)mi_ntLp[id]`)
     [2..2+N]   Dosound register-command stream, ending in a 4-byte
                duration trailer that sf_irqp reads as
                the SFX's playback length
   The pointer is stashed in mi_ntLp[id] where the
   dispatch layer picks it up.

   addr: sf_sl() */

void
sf_sl()
{
        short           fhandle;
        short           index;
        short           size;
        short *         block;

        /* Ghidra soundeffects_load: for each entry, read the 2-byte
           size, Malloc(size + 4), store the block pointer in
           mi_ntLp[index], write size to the first word of the block,
           then read `size` bytes into block+1.  Terminator is size==0. */
        fhandle = fOpen("sounds.lcp", 0);
        for (index = 0; index < 500; index = index + 1) {
                fr_read(fhandle, 2L, &size);
                if (size == 0)
                        break;
                block = (short *) Malloc((long) (size + 4));
                mi_ntLp[index] = (unsigned char *) block;
                if (block == (short *) 0)
                        er_nomem();
                *block = size;
                fr_read(fhandle, (long) size, block + 1);
        }
        Fclose(fhandle);
}

void
sgPlay(filename)
char *  filename;
{
        DTA *   dta_ptr;
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
        dta_ptr = (DTA *) Fgetdta();
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
