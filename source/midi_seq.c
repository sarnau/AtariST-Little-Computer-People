/*
 * midi_seq.c -- MIDI sequencer control surface.
 *
 * The 1985 game's MIDI subsystem lives in three tiers:
 *
 *   1. This file: song-lifecycle control (init/reset/start), header
 *      parsing dispatch, and playback-position bookkeeping.  All six
 *      functions here are real ports.
 *
 *   2. Deferred (stubs): the per-event MIDI parser + PSG channel
 *      output driver (envelope stepping, note-on/off state, program
 *      change dispatch, tempo-derived tick divider).  These live
 *      behind mq_pacm, mq_bust,
 *      mq_sepc, and the interrupt-service loop
 *      that fires from the ST's 200 Hz timer.
 *
 *   3. XBIOS/BIOS:  Midiws (send raw MIDI bytes) and Giaccess (PSG
 *      register write).  Both routed via _xbios in osbind.h.
 *
 * File-format provenance: .SNG and .ORG files are direct exports from
 * Activision Music Studio 2.0 (published 1986, Ed Bogas / Audio Light).
 * sgPlay strips a leading 10-byte Music Studio signature
 * (`\xCD` + "Mstudio" + `\xCD\x02`) before handing the rest of the file
 * to us; the layout below is offsets *inside the stripped body*, i.e.
 * inside the buffer sgPlay allocates.
 *
 * Stripped-body layout (relative to mi_dbase = start + 0x1FE):
 *
 *   body + 0x000..0x1A3    Music Studio config header:
 *                            +0x00..0x05  section tag "Blocks"
 *                            +0x1A..      instrument name list
 *                                         ("Harmonica", "Guitar", ...)
 *                            +0x??..      per-instrument ADSR envelope
 *                                         defaults, each 8 bytes
 *   body + 0x1A4..0x1FD    90-byte channel + program-change map
 *                          (15 logical channels x 2 bytes each; parsed
 *                          by mq_pacm at p - 90)
 *   body + 0x1FE           MIDI event stream (this is mi_dbase)
 *
 * addr: mq_inis(), mq_parh(),
 *       mq_resp(), mq_skip(),
 *       mq_setp(), mq_stap()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "midi_seq.h"
#include "psg_io.h"
#include "psgfreq.h"


/* Header-command handlers, dispatched by mq_parh on
   0x80/0x81/0x83/0x84/0xC0/0xFF.  LCP_STX has no such helpers: their
   bodies are written out inside mq_parh's switch, so the default
   build must not emit them at all -- Alcyon still lays a static
   function down even when nothing calls it, and these five cost 244
   bytes the original does not have. */
/* STX links mq_skip first in this object (0x12a). */
#include "parts/mq_skip.c"

/* mq_inis: song-lifecycle entry point.
   If a song is playing: signal SEQ_PHASE_SONG_ENDING and return
   without starting the new one; caller spins until mi_play is false.
   Idle: position mi_dbase at buffer+0x1FE (event stream), parse header,
   reset programs, skip 0x00/0xFF padding, store playback bounds, kick.
   addr: mq_inis() */

void
mq_inis(param_1, maxPos)
unsigned char * param_1;
long            maxPos;
{

        if (mi_play != NO) {
                g_mspha = SEQ_PHASE_SONG_ENDING;
                return;
        }

        mq_parh(mi_dbase = param_1 + 0x1fe);
        mq_resp();
        mq_setp(mq_skip(mi_dbase), maxPos);
        mq_stap();
        mi_play = YES;
}

/* mq_setp: stash read cursor + end-of-song marker; init per-song
   driver state; publish ticks-per-beat via mi_tpb.
   Envelope base = mi_dbase - 0x168 (360 bytes, ADSR block).
   addr: mq_setp() */

void
mq_setp(curPos, maxPos)
unsigned char * curPos;
long            maxPos;
{
        mi_sqpos     = curPos;
        /* mi_seqE, not a separate g_msmap: the reference relocates
           both these stores and mq_pars' `mi_sqpos >= mi_seqE` tests
           to the SAME cell.  It is the end-of-sequence pointer, and
           -1 is how "no limit" is spelled -- every caller passes
           g_momap, which is 0. */
        if (maxPos == 0)
                mi_seqE = (unsigned char *) -1L;
        else
                mi_seqE = (unsigned char *) maxPos;

        mi_env = (long) (mi_dbase - 0x168);
        mi_vel           = mi_dvel;
        psg_cvol      = psg_dvol;
        mi_evi    = 0;
        mi_evcn   = 9;
        mi_tpb          = g_mtspb;
}

/* mq_stap: init timer counters + arm sequencer.
   All 4 tick counters seeded 100 (~500 ms grace before first event).
   mi_dwrm=0 selects XBIOS Midiws path (not direct ACIA).
   addr: mq_stap() */

void
mq_stap()
{
        /* STX writes these as chained assignments. */
        mi_dwrm = g_mtcou = 0;
        mi_lpTk = mi_nxTk = mi_nlp0 = g_mtpre = g_mtdiv = 100;
        g_mspha = g_msmsa = YES;
}


/* mq_pshl: push loop marker {return_addr, count-1} on mi_lstk (cap 49).
   addr: midi_seq_push_loop() */

void
mq_pshl(a, b)
void *  a;
short   b;
{
        if (mi_evcn < 49) {
                mi_lstk[mi_evcn] = (long) a;
                mi_evcn++;
                mi_lstk[mi_evcn] = (long)(short)(b - 1);
                mi_evcn++;
        }
}

/* mq_popl: pop/decrement top of loop stack.  Returns loop-start ptr
   if count nonzero, else NULL (fall through end).
   addr: midi_seq_pop_loop() */

unsigned char *
mq_popl()
{
        unsigned char * ret;
        long            cnt;

        if (mi_evcn == 9)
                return (unsigned char *) 0;
        ret = (unsigned char *) mi_lstk[mi_evcn - 2];
        cnt = mi_lstk[mi_evcn - 1];
        mi_lstk[mi_evcn - 1]--;
        if (cnt == 0) {
                mi_evcn -= 2;
                return (unsigned char *) 0;
        } else
                return ret;
}

/* mq_pars: walk compact event stream at mi_sqpos.
   Byte forms:
     0x00        tick separator; returns 1, mi_nlp0 loaded
     0x01..0x7F  note event, 3 bytes:
                   byte0: [0..3]=logical ch, [4]=note-on (inverted),
                          [5]=sustain, [6]=note-off
                   byte1: [0..4]=dur index, [5]=accent (vel 0x7F),
                          [6..7]=transpose mode
                   byte2: MIDI note number
     0x82        bar marker (1 byte)
     0x85 <n>    loop start, count=n
     0x86        loop end (jump back if count > 0)
     0xFF        end of song, returns 0
   Ghidra 0x10388, nested while-loops preserved as gotos.
   addr: midi_seq_parse_events() */

short
mq_pars()
{
        /* No locals at all: STX walks mi_sqpos with ++ in place and
           dispatches the command bytes through a switch. */

        /* Prologue: skip leading 0x00, refresh mi_nlp0, end-check. */
        if (*mi_sqpos != 0)
                return 0;
        mi_sqpos++;
        if (mi_sqpos >= mi_seqE)
                return 0;
        mi_evTf = 0;
        mq_rdur();
        if (mi_sqpos >= mi_seqE)
                return 0;

        while (*mi_sqpos != 0) {
                if ((*mi_sqpos & 0x80) == 0) {
                        /* Note event: unpack bytes 0..2, advance one
                           byte at a time, queue via mq_qnne.  byte1
                           bit 5 = accent (max vel + max PSG vol). */
                        mi_evTf = 1;
                        mi_nnOn = 16 - (*mi_sqpos & 0x10);
                        mi_lasT = *mi_sqpos & 0x40;
                        mi_nnOf = *mi_sqpos & 0x20;
                        mi_ccha = *mi_sqpos & 0x0f;
                        mi_sqpos++;

                        if ((mi_nlpA = *mi_sqpos & 0x20) != 0) {
                                mi_vel = 0x7f;
                                psg_cvol = 0xf;
                        } else {
                                mi_vel = mi_dvel;
                                psg_cvol = psg_dvol;
                        }
                        mi_nmof = *mi_sqpos & 0xc0;
                        /* mi_ndur, not mi_nlp0: the reference stores
                           this one in a second cell that only mq_qnne
                           reads.  mq_rdur computes the same expression
                           into mi_nlp0, which the tick counters use --
                           the port had merged the two. */
                        mi_ndur = (mi_ndt[*mi_sqpos & 0x1f] - 1) * g_mtspb;
                        mi_sqpos++;

                        if ((mi_nmof & 0xc0) != 0) {
                                mi_cnot = *mi_sqpos & 0x7f;
                                mi_sqpos++;
                                if (mi_nmof & 0x80) {
                                        if (mi_nmof & 0x40)
                                                mi_cnot--;
                                        else
                                                mi_cnot++;
                                }
                        } else {
                                mi_cnot = g_mstr[*mi_sqpos & 0x7f];
                                mi_sqpos++;
                        }

                        if (mi_nnOn != 0)
                                mq_qnne();
                } else {
                        switch (*mi_sqpos++ & 0xff) {
                        case 0x82:
                                /* Bar marker: refresh mi_nlp0 for the
                                   next event only if none was decoded
                                   this pass. */
                                if (mi_evTf == 0) {
                                        mq_rdur();
                                        if (mi_sqpos >= mi_seqE)
                                                return 0;
                                }
                                break;
                        case 0x85:
                                /* Loop start: byte = count, push the
                                   return address. */
                                mq_pshl(mi_sqpos + 1, *mi_sqpos);
                                mi_sqpos++;
                                mq_rdur();
                                if (mi_sqpos >= mi_seqE)
                                        return 0;
                                break;
                        case 0x86:
                                /* Loop end: pop, jump back if nonzero. */
                                if ((mi_dptr = mq_popl()) != 0)
                                        mi_sqpos = mi_dptr;
                                mq_rdur();
                                if (mi_sqpos >= mi_seqE)
                                        return 0;
                                break;
                        case 0xff:
                                return 0;
                                break;
                        }
                }
        }
        return 1;
}

/* mq_rdur: skip 0x00 pad at mi_sqpos; peek next event's dur-index nibble.
   High-bit-clear (note) -> mi_nlp0 = tick-count; else mi_nlp0 = 0.
   addr: midi_seq_read_note_duration() */

void
mq_rdur()
{
        for (; *mi_sqpos == 0; mi_sqpos++) ;
        if ((*mi_sqpos & 0x80) == 0)
                mi_nlp0 = (short)(mi_ndt[(short)(char) mi_sqpos[1] & 0x1f]
                                                          - 1) * g_mtspb;
        else
                mi_nlp0 = 0;
}

/* mq_spgm does not exist in LCP_STX: mq_sepc (0x84a) is the one
   program-change sender, and mq_qnne calls it directly. */

/* mq_qnne: queue Note-On in mi_evq as {duration, note|sustain, phys_ch}
   and dispatch Note-On via mq_dise.  Queue entry fires paired Note-Off
   later via mq_expN + mq_snof.
   addr: midi_seq_queue_note_event() */


void
mq_qnne()
{
        short   ch;

        if (mi_evi < 58) {
                mi_evq[mi_evi] = mi_ndur;
                mi_evi++;
                if (mi_nnOn != 0) {
                        mi_evq[mi_evi] = (mi_lasT << 1) | mi_cnot;
                        mi_evi++;
                } else {
                        mi_evq[mi_evi] = 0;
                        mi_evi++;
                }
                mi_evq[mi_evi] = mi_chmap[mi_ccha];
                mi_evi++;
        } else
                return;

        if (mi_cnot > g_mnhi)
                return;
        if (mi_cnot < g_mnlo)
                return;

        if (mi_slop != NO)
                mq_sepc(mi_ccha);
        else
                mi_ccha = mi_varR;

        if (mi_nnOf != 0)
                mi_noSt[mi_cnot] = 0;
        if (mi_lasT != 0)
                mi_noSt[mi_cnot] = mi_ccha;
        if (mi_nnOf != 0)
                return;

        ch = mi_chmap[mi_ccha];
        g_meve[0] = (ch & 0xf) | 0x90;
        g_meve[1] = mi_cnot;
        g_meve[2] = mi_vel;
        mq_dise(g_meve, (short) 3, ch);
}

/* mq_snof: send MIDI Note-Off (vel=0) for a queued note.
   nptr[0]={note|flags}, nptr[1]=physical channel byte.
   Fires only if note in [g_mnhi, g_mnlo] and non-zero.
   addr: midi_seq_send_note_off() */

void
mq_snof(nptr)
short * nptr;
{
        /* STX is called with &mi_evq[i] and walks the pointer forward;
           the range test is one bitwise OR of two comparisons and each
           rejection returns. */
        if ((nptr[1] & 0x80) != 0)
                return;
        nptr++;
        g_meve[1] = nptr[0];
        if ((char) g_meve[1] > g_mnhi | (char) g_meve[1] < g_mnlo)
                return 1;
        if (g_meve[1] == 0)
                return 1;
        nptr++;
        g_meve[0] = (nptr[0] & 0xf) + 0x90;
        g_meve[2] = 0;
        mq_dise(g_meve, (short) 3, (short) nptr[0]);
}

/* mq_sepc: dispatch Program Change (0xCn) for logical channel `index`.
   Fires only if cached program differs and MIDI output enabled.
   Current-program keyed by physical channel (mi_chmap & 0xf), so
   shared physical channels only get one PC per song load.
   addr: mq_sepc() */

void
mq_sepc(index)
char    index;
{
        if (g_mcpro[mi_chmap[index] & 0xf] == mi_pgmap[index])
                return;
        if (g_moen == NO)
                return;

        g_meve[0] = (mi_chmap[index] & 0xf) | 0xc0;
        g_meve[1] = mi_pgmap[index];
        g_mcpro[mi_chmap[index] & 0xf] = mi_pgmap[index];
        mq_dise(g_meve, (short) 2, (short) 0);
}

/* mq_dise: send one MIDI event to MIDI OUT (Midiws) + YM2149 PSG.
   Both paths gated by their enabled flags.
   MIDI OUT: octave-transpose note by (env_val - hi_nibble(midi_ch))
     * -12 semitones, write via mowrit (mi_dwrm=1) or Midiws; restore
     note before PSG path.
   PSG path (Note-On 0x9n only):
     vel=0 -> Note-Off: find channel by note, ENV_RELEASE.
     vel>0 -> Note-On: alloc silent channel, else voice-steal by
       highest phase; guard [g_mnhi, g_mnlo]; copy 8 ADSR bytes from
       mi_env + (g_mccha-1)*8; compute (2 - hi_nib(attack_dur))*12
       octave offset; write PSG tone/mixer/noise; if freq<0x17 use
       ENV_FADEOUT instead of ENV_ATTACK; set psg_ntAc.
   Returns 1 on success, 0 on non-Note-On or Note-Off miss.
   addr: mq_dise() */

short
mq_dise(midiEvP, midiEvS, midi_ch)
char *          midiEvP;
char            midiEvS;
char            midi_ch;
{
        /* Both byte arguments are saved and restored around the MIDI
           OUT path, which walks them destructively. */
        char            chosen;                 /* -2, also saved_note */
        char            unused;                 /* -4, never touched   */
        char            best;                   /* -6  */
        char            cVar4;                  /* -8  */
        unsigned char * saved_ptr;              /* -12 */
        char            saved_size;             /* -14 */
        long            env_ptr;                /* -18 */
        char            envelope_phase;         /* -20 */
        short           attack_hi;              /* -22 */
        short           noise_mask;             /* -24 */
        short           mixer_bits;             /* -26 */

        saved_ptr  = midiEvP;
        saved_size = midiEvS;

        /* ---- MIDI OUT path ---- */
        if (g_moen != NO) {
                chosen = saved_ptr[1];
                if (midi_ch != 0)
                        midiEvP[1] = (midiEvP[1] & 0xff) -
                                (((3 - ((midi_ch >> 4) & 0xf)) * 12) & 0xff);
                if (mi_dwrm == 1) {
                        while (midiEvS) {
                                mowrit(*midiEvP);
                                midiEvP++;
                                midiEvS--;
                        }
                } else {
                        Midiws(midiEvS - 1, midiEvP);
                }
                saved_ptr[1] = chosen;
        }

        /* ---- PSG path ---- */
        if (psg_out != NO) {

                midiEvP  = saved_ptr;
                midiEvS  = saved_size;

                if ((*midiEvP++ & 0xf0) != 0x90)
                        return 0;

                if (midiEvP[1] != 0) {

                /* ---- Note-On: pick a channel ---- */
                chosen = 0;
                while (psg_chNt[chosen++])
                        ;
                chosen--;
                if (chosen == 3) {
                        /* Voice-steal: pick the channel furthest along in its
                           envelope (highest phase index). */
                        best = chosen = 0;
                        while (chosen != 2) {
                                chosen++;
                                if (psg_envelope[chosen].phase >
                                    psg_envelope[chosen - 1].phase)
                                        best = chosen;
                        }
                        chosen = best;
                }

                /* Range guard: the whole note-on body is inside it.
                   LOW first -- the reference compares against 0x34e
                   (36) and branches lt, then against 0x34c (96) and
                   branches gt. */
                if (*midiEvP >= g_mnlo && *midiEvP <= g_mnhi) {

                /* Copy 8 bytes of ADSR params from the .SNG envelope
                   block; the source address lands in a local first. */
                env_ptr = (mi_ccha - 1) * 8 + mi_env;
                envelope_phase = ENV_ATTACK;
                psg_cpE(env_ptr,
                        (unsigned char *) &psg_envelope[chosen] + 1,
                        8L);

                /* Split the packed nibbles: attack_start_vol keeps its low
                   4 bits (start volume), high 4 bits stash the mixer flags;
                   attack_duration keeps its low 4 bits, high 4 bits encode
                   the octave shift (2 - N) * 12 semitones. */
                attack_hi = (psg_envelope[chosen].attack_start_vol >> 4) & 0xf;
                psg_envelope[chosen].attack_start_vol &= 0xf;
                cVar4 = (2 - ((psg_envelope[chosen].attack_duration >> 4) & 0xf)) * 12;
                psg_envelope[chosen].attack_duration &= 0xf;
                mixer_bits = attack_hi << chosen;
                noise_mask = ~(9 << chosen);

                /* The three scratch shorts are reused from here on:
                   attack_hi carries the period, noise_mask its high
                   nibble and mixer_bits the register number. */
                attack_hi = psg_freq[*midiEvP + cVar4] / 60;
                if (mi_dwrm == 1) {
                        psg_wr(attack_hi, 6);
                        psg_mix(mixer_bits, noise_mask | 0xc0);
                } else {
                        /* The PSG writes go straight to the trap:
                           the Giaccess macro's (char) cast on the data
                           argument is not in the original here. */
                        xbios(28, attack_hi, 0x86);
                        xbios(28, xbios(28, 0, 7) &
                                  (long) (noise_mask | 0xc0) |
                                  (long) mixer_bits, 0x87);
                }

                mixer_bits = chosen << 1;

                if (*midiEvP + cVar4 > 22) {
                        attack_hi  = psg_freq[*midiEvP + cVar4];
                        noise_mask = (attack_hi >> 8) & 0xf;
                        attack_hi = attack_hi & 0xff;
                        if (mi_dwrm == 1) {
                                psg_wr(attack_hi, mixer_bits);
                                psg_wr(noise_mask, mixer_bits + 1);
                        } else {
                                xbios(28, attack_hi, mixer_bits + 0x80);
                                xbios(28, noise_mask, mixer_bits + 0x81);
                        }
                } else {
                        envelope_phase = ENV_FADEOUT;
                }

                psg_chNt[chosen] = *midiEvP;
                if (envelope_phase == ENV_FADEOUT)
                        psg_envelope[chosen].current_volume = 0;
                psg_envelope[chosen].max_volume  = psg_cvol;
                psg_ntAc = psg_envelope[chosen].phase_timer = 1;
                psg_envelope[chosen].phase = envelope_phase;

                }       /* range guard */

                } else {

                /* ---- Note-Off (velocity == 0) ---- */
                chosen = 0;
                while (psg_chNt[chosen++] != *midiEvP && chosen < 4)
                        ;
                if (chosen == 4)
                        return 0;
                chosen--;
                psg_chNt[chosen] = 0;
                psg_envelope[chosen].phase       = ENV_RELEASE;
                psg_envelope[chosen].phase_timer = 0;

                }
                return 1;
        }

        return 1;
}

/* mq_expN: subtract val from each queued event's remaining duration;
   when <=0, mq_snof + mq_rmev.
   addr: midi_seq_expire_notes() */

void
mq_expN(val)
short   val;
{
        /* STX: one local, memory-direct steps, the removal result
           tested in place, and mq_snof gets &mi_evq[i]. */
        short   i;

        for (i = 0; i < mi_evi; i += 3) {
                mi_evq[i] -= val;
                if (mi_evq[i] <= 0) {
                        mq_snof(&mi_evq[i]);
                        if (mq_rmev(i) != 0)
                                i -= 3;
                }
        }
}

/* STX links mq_rmev immediately after mq_expN (0xe64). */
#include "parts/mq_rmev.c"

/* mq_tick lives in source/mq_tick.s -- privileged move-sr + rte
   terminator, installed by Xbtimer directly. */

/* ---- Timer-A sequencer engine (other-image; KEPT build only).
   The ROM never steps its sequencer -- no ISR exists and nothing
   below is referenced by ROM code. ---- */
/* mq_advs: sequencer state-machine advance from mq_tick.  Ghidra 0x111b0.
   WAIT_NOTE_EXPIRE (0): expire queued notes, reload prescaler, -> PARSE.
   PARSE_NEXT_EVENT (1): mq_pars() walks next batch; 0=end-of-song ->
     SONG_ENDING, else mi_nlp0 = ticks until next event.
   SONG_ENDING (2): expire remaining; when queue empty, kill PSG + flags.
   addr: midi_seq_advance_sequencer() */

void
mq_advs()
{
        short   res;

        if (g_mspha == SEQ_PHASE_WAIT_NOTE_EXPIRE) {
                res = g_mtcou - mi_lpTk;
                mq_expN(res);
                mi_lpTk    = g_mtcou;
                g_mtpre    = mi_tpb;
                g_mspha    = SEQ_PHASE_PARSE_NEXT_EVENT;
                mi_nxTk   += mi_tpb;
                return;                 /* STX: explicit return */
        } else if (g_mspha == SEQ_PHASE_PARSE_NEXT_EVENT) {
                g_mspha    = SEQ_PHASE_WAIT_NOTE_EXPIRE;
                mi_nlp0    = -1;
                /* STX wraps the parse in a loop and returns from both
                   arms. */
                while (mi_nlp0 < 0) {
                        if (mq_pars() != 0) {
                                mi_nxTk += mi_nlp0;
                                mi_nlp0 = mi_nxTk - g_mtcou;
                                if (mi_nlp0 > 0)
                                        g_mtpre = mi_nlp0;
                                return;
                        } else {
                                g_mspha = SEQ_PHASE_SONG_ENDING;
                                g_mtpre = mi_tpb;
                                mi_nxTk += g_mtpre;
                                return;
                        }
                }
        } else {
                res = g_mtcou - mi_lpTk;
                mq_expN(res);
                mi_lpTk    = g_mtcou;
                g_mtpre    = mi_tpb;
                mi_nxTk   += mi_tpb;
                if (mi_evi == 0) {
                        psg_envelope[0].phase =
                        psg_envelope[1].phase =
                        psg_envelope[2].phase = ENV_IDLE;
                        mi_play = psg_ntAc = g_msmsa = NO;
                        psg_wr(0, 8);
                        psg_wr(0, 9);
                        psg_wr(0, 10);
                }
        }
}

/* mq_stop (Ghidra midi_seq_stop @ 0x1103c): stop sequencer.
   Drain pending events, send Note-Off for every mi_noSt[] flag,
   clear g_msmsa.  Not called yet -- present for ROM parity.
   addr: mq_stop() */


void
mq_stop()
{
        short   note;
        short   hadPend;
        short   ch;

        if (mi_evi > 0)
                hadPend = 1;
        else
                hadPend = 0;

        while (mi_evi > 0) {
                mi_nlp0 = g_mtcou - mi_nxTk;
                if (mi_nlp0 > 0) {
                        mq_expN(mi_nlp0);
                        mi_nxTk += mi_nlp0;
                }
        }

        if (hadPend != NO) {
                g_meve[2] = 0;
                for (note = 0; note < 0x80; note++) {
                        if ((ch = mi_noSt[note]) != 0) {
                                ch = mi_chmap[ch];
                                g_meve[0] = (ch & 0x0f) | 0x90;
                                g_meve[1] = note;
                                mq_dise(g_meve, 3, ch);
                        }
                }
        }

        g_msmsa = NO;
}

/* mq_intim (0x1112) sits here -- between mq_stop and mq_extm. */
#include "parts/mq_intim.c"

/* mq_extm (Ghidra midi_seq_exit_timer @ 0x11162): tear down MFP
   Timer-A hook; Xbtimer(0,...) reinstalls saved ISR from mi_svtv.
   Not called yet -- present for ROM parity.
   addr: mq_extm() */

void
mq_extm()
{
        Xbtimer(0, 0, 0x1c, mi_svtv);
}


/* STX links these near the end of the object:
   mq_resp 0x1184, mq_parh 0x11fa. */
#include "parts/mq_resp.c"
#include "parts/mq_parh.c"

/* mq_pacm: unpack 30-byte channel/program map (90 bytes before mi_dbase).
   Bytes 0..14 = MIDI channel for logical 1..15; bytes 15..29 = program.
   Values are 1-based on disk (0 = no-op sentinel); decrement on load.
   Logical channel 0 reserved for game SFX.
   addr: mq_pacm() */

void
mq_pacm(p)
unsigned char * p;
{
        short   i;

        /* STX writes the offset arithmetic inside the dereference,
           which makes the pointer the INDEX register and the loop
           counter the base -- `p[i - 1]` gives the other way round. */
        for (i = 1; i < 16; i++) {
                mi_chmap[i] = *(p + i - 1)  - 1;
                mi_pgmap[i] = *(p + i + 14) - 1;
        }
}


void
mq_bust(value)
short   value;
{
        short           i;
        short           note_shift;
        char            chord_mask;

        for (i = 0; i < 0x84; i++)
                g_mstr[i] = i;
        g_mstr[1]  = -1;
        g_mstr[3]  = -1;
        g_mstr[6]  = -1;
        g_mstr[8]  = -1;
        g_mstr[10] = -1;

        if (value == 1)
                return 1;

        if (value > 8)
                note_shift = -1;
        else
                note_shift = 1;

        for (i = 0; i < 0x84; i += 12) {
                chord_mask = g_msmk[value];
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 11] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 9] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 7] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 5] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 4] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i + 2] += note_shift;
                chord_mask >>= 1;
                if ((chord_mask & 1) == 0)
                        g_mstr[i] += note_shift;
        }
}


/* psg_cpE -> parts/psg_cpE.c (STX: 0x1586, right before psg_upEn). */
#include "parts/psg_cpE.c"

/* psg_upEn: PSG software ADSR envelope processor.  50 Hz from mq_tick.
   3 channels through attack->decay->sustain->release->fadeout.
   Per phase: Bresenham accum, delta = (target-cur)*rate_table[t],
   accum += delta; while accum > 360, cur += dir; accum -= 360.
   phase_timer==0 with dur==0 -> immediate fall-through (gotos).
   Clamp cur to max_volume; write PSG amp reg 8/9/10 via psg_wr.
   Preserves Ghidra switch(fallthrough) shape as C gotos.
   addr: psg_process_envelopes() */

void
psg_upEn()
{
        char    i;

        for (i = 0; i < 3; i++) {
                if (!psg_envelope[i].phase)
                        continue;

                switch (psg_envelope[i].phase) {
                case ENV_ATTACK:
                        psg_envelope[(short) i].current_volume =
                                                 psg_envelope[(short) i].attack_start_vol;
                        psg_envelope[(short) i].phase = ENV_DECAY;
                        if (!psg_envelope[i].attack_duration) {
                                psg_envelope[(short) i].current_volume =
                                                                 psg_envelope[(short) i].attack_target_vol;
                                psg_envelope[(short) i].phase_timer = 0;
                                goto do_decay;
                        }
                        psg_envelope[(short) i].phase_timer =
                                                 (short) psg_envelope[(short) i].attack_duration;
                        if (psg_envelope[i].attack_start_vol >
                            psg_envelope[i].attack_target_vol) {
                                psg_rdel[(short) i] =
                                                          (short) psg_envelope[(short) i].attack_start_vol -
                                                          (short) psg_envelope[(short) i].attack_target_vol;
                                psg_envelope[(short) i].ramp_direction = -1;
                        } else {
                                psg_envelope[(short) i].ramp_direction = 1;
                                psg_rdel[(short) i] =
                                                          (short) psg_envelope[(short) i].attack_target_vol -
                                                          (short) psg_envelope[(short) i].attack_start_vol;
                        }
                        psg_rdel[(short) i] = psg_rdel[(short) i] *
                                             mi_evrt[psg_envelope[(short) i].phase_timer];
                        psg_envelope[(short) i].phase_timer =
                                             mi_evtt[psg_envelope[(short) i].phase_timer];
                        psg_racc[(short) i] = 0;
                        break;

                case ENV_DECAY:
do_decay:
                        if (psg_envelope[i].phase_timer-- > 0) {
                                psg_racc[i] += psg_rdel[i];
                                while (psg_racc[i] > 0x168) {
                                        psg_envelope[i].current_volume +=
                                                psg_envelope[i].ramp_direction;
                                        psg_racc[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (!psg_envelope[i].decay_duration) {
                                        psg_envelope[(short) i].current_volume =
                                                                          psg_envelope[(short) i].decay_target_vol;
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_sustain;
                                }
                                psg_envelope[(short) i].phase = ENV_SUSTAIN;
                                psg_envelope[(short) i].phase_timer =
                                                                 (short) psg_envelope[(short) i].decay_duration;
                                if (psg_envelope[i].attack_target_vol >
                                    psg_envelope[i].decay_target_vol) {
                                        psg_rdel[(short) i] =
                                                                  (short) psg_envelope[(short) i].attack_target_vol -
                                                                  (short) psg_envelope[(short) i].decay_target_vol;
                                        psg_envelope[(short) i].ramp_direction = -1;
                                } else {
                                        psg_envelope[(short) i].ramp_direction = 1;
                                        psg_rdel[(short) i] =
                                                                  (short) psg_envelope[(short) i].decay_target_vol -
                                                                  (short) psg_envelope[(short) i].attack_target_vol;
                                }
                                psg_rdel[(short) i] = psg_rdel[(short) i] *
                                                                          mi_evrt[psg_envelope[(short) i].phase_timer];
                                psg_envelope[(short) i].phase_timer =
                                                                          mi_evtt[psg_envelope[(short) i].phase_timer];
                                psg_racc[(short) i] = 0;
                                break;
                        }

                case ENV_SUSTAIN:
do_sustain:
                        if (psg_envelope[i].phase_timer-- > 0) {
                                psg_racc[i] += psg_rdel[i];
                                while (psg_racc[i] > 0x168) {
                                        psg_envelope[i].current_volume +=
                                                psg_envelope[i].ramp_direction;
                                        psg_racc[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (!psg_envelope[i].sustain_duration) {
                                        psg_envelope[(short) i].current_volume =
                                                                          psg_envelope[(short) i].sustain_target_vol;
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_release;
                                }
                                psg_envelope[(short) i].phase = ENV_RELEASE;
                                psg_envelope[(short) i].phase_timer =
                                                                 mi_evst[(short) psg_envelope[(short) i].sustain_duration];
                                if (psg_envelope[i].decay_target_vol >
                                    psg_envelope[i].sustain_target_vol) {
                                        psg_rdel[(short) i] =
                                                                  (short) psg_envelope[(short) i].decay_target_vol -
                                                                  (short) psg_envelope[(short) i].sustain_target_vol;
                                        psg_envelope[(short) i].ramp_direction = -1;
                                } else {
                                        psg_envelope[(short) i].ramp_direction = 1;
                                        psg_rdel[(short) i] =
                                                                  (short) psg_envelope[(short) i].sustain_target_vol -
                                                                  (short) psg_envelope[(short) i].decay_target_vol;
                                }
                                psg_rdel[(short) i] = psg_rdel[(short) i] *
                                                                          mi_evrl[(short) psg_envelope[(short) i].sustain_duration];
                                psg_racc[(short) i] = 0;
                                break;
                        }

                case ENV_RELEASE:
do_release:
                        if (psg_envelope[i].phase_timer-- > 0) {
                                psg_racc[i] += psg_rdel[i];
                                while (psg_racc[i] > 0x168) {
                                        psg_envelope[i].current_volume +=
                                                psg_envelope[i].ramp_direction;
                                        psg_racc[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (psg_envelope[i].release_duration) {
                                        psg_envelope[(short) i].phase = ENV_FADEOUT;
                                        psg_envelope[(short) i].phase_timer =
                                                         (short) psg_envelope[(short) i].release_duration;
                                        psg_rdel[(short) i] =
                                                  (short) psg_envelope[(short) i].current_volume;
                                        psg_envelope[(short) i].ramp_direction = -1;
                                        psg_rdel[(short) i] = psg_rdel[(short) i] *
                                                  mi_evrt[psg_envelope[(short) i].phase_timer];
                                        psg_envelope[(short) i].phase_timer =
                                                  mi_evtt[psg_envelope[(short) i].phase_timer];
                                        psg_racc[(short) i] = 0;
                                        break;
                                } else {
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_fadeout;
                                }
                        }
                        /* falls through into ENV_FADEOUT */

                case ENV_FADEOUT:
do_fadeout:
                        if (psg_envelope[i].phase_timer-- > 0 &&
                            psg_envelope[i].current_volume) {
                                psg_racc[i] += psg_rdel[i];
                                while (psg_racc[i] > 0x168) {
                                        psg_envelope[i].current_volume +=
                                                psg_envelope[i].ramp_direction;
                                        psg_racc[i] -= 0x168;
                                }
                        } else {
                                psg_envelope[i].current_volume =
                                        psg_envelope[i].phase = ENV_IDLE;
                        }
                        break;
                }

                /* The clamped volume goes through a global, not a
                   local, and the pick is a ternary (one store). */
                psg_ovol = psg_envelope[i].current_volume >
                           psg_envelope[i].max_volume
                         ? psg_envelope[i].max_volume
                         : psg_envelope[i].current_volume;
                psg_wr(psg_ovol, psg_rot[i] - 0x80);
        }
}
