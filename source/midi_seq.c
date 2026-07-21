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


/* Forward decls for the file's own functions -- our K&R style would
   normally rely on default-int declarations, but Clang under -Werror
   complains about the mixed short/long signatures below. */

/* Sequencer parse state -- see globals.c comment block. */

/* Header-command handlers.  mq_parh dispatches to these
   by matching the command byte against 0x80/0x81/0x83/0x84/0xC0/0xFF.
   The 1985 code used a jump table for the dispatch (which Ghidra
   couldn't recover); we use a plain switch, which the compiler
   naturally emits as a jump table under -O2.

   Individual handlers are stubs -- the actual per-command logic
   (parsing tempo bytes, walking the scale table, kicking a program
   change) lives in the deferred audio driver.  For now the header
   parser advances the pointer past each command, matching the original
   3-byte-per-command stride. */

/* mh_chac: MIDI header 0x80 -- set channel count.
   Ghidra 0x11246: reads p[2] into g_mchcn (0x298f0) and
   calls midi_seq_build_scale_table with that same value.  Advances
   the header pointer by 3 bytes. */
static unsigned char *
mh_chac(p)
unsigned char * p;
{
        g_mchcn = p[2];
        mq_bust(g_mchcn);
        return p + 3;
}
/* mh_temp: MIDI header 0x81 -- set tempo.
   Ghidra 0x11264: reads p[1] into mi_temp (0x298f2), then computes
   g_mtspb (0x298f4) = 2400 / mi_temp.  Advances p by ONLY 2 bytes
   (this command has one payload byte, not two). */
static unsigned char *
mh_temp(p)
unsigned char * p;
{
        mi_temp = p[1];
        g_mtspb    = 2400 / mi_temp;
        return p + 2;
}
/* mh_volu: MIDI header 0x83 -- volume.
   Ghidra 0x1129c: pure pointer advance by 2, no side effects.
   The port previously read p[1] into mi_dvel, which
   the Ghidra binary does NOT do here (any velocity handling lives
   in the event stream, not the header). */
static unsigned char *
mh_volu(p)
unsigned char * p;
{
        (void) p;
        return p + 2;
}
/* mh_scat: MIDI header 0x84 -- cache the raw velocity byte + bucketed
   PSG-volume threshold.  Ghidra decompile at 0x112a4 names these
   "midi_current_scale_value" and "midi_scale_bucket" respectively but
   the port already had them as mi_dvel (0x29a24) and psg_dvol
   (0x29a26) from their consumer-side usage in mq_setp
   (mi_vel <- mi_dvel; psg_cvol <- psg_dvol).  Same addresses, more
   descriptive names.

   Raw disasm (0x112a4..0x1131e): sets mi_dvel = p[2], then a chained
   `cmpi.b` / `bge` ladder against 0x17/0x27/0x37/0x57/0x67/-0x80
   picks psg_dvol from {5, 7, 9, 11, 13, [15]}.  The final #-0x80
   branch is DEAD in the ROM (signed compare vs -128 is trivially
   true for every byte), so psg_dvol is never assigned 15 through
   this path -- values >= 0x67 leave the old bucket untouched.  Port
   replicates the ROM byte-for-byte, including the unreachable
   15-branch omission. */
static unsigned char *
mh_scat(p)
unsigned char * p;
{
        mi_dvel = p[2];
        if      (mi_dvel < 0x17) psg_dvol = 5;
        else if (mi_dvel < 0x27) psg_dvol = 7;
        else if (mi_dvel < 0x37) psg_dvol = 9;
        else if (mi_dvel < 0x57) psg_dvol = 11;
        else if (mi_dvel < 0x67) psg_dvol = 13;
        /* mi_dvel >= 0x67: ROM's `cmpi.b #-0x80; bge` is a dead
           branch; psg_dvol is left unchanged. */
        return p + 3;
}
static unsigned char *
mh_proc(p)
unsigned char * p;
{
        return p + 3;
}

/* mq_inis: song-lifecycle entry point.  When called with a
   song already playing, signals the current one to stop (the audio
   driver picks up the SEQ_PHASE_SONG_ENDING transition on its next
   interrupt) and returns without starting the new song -- the caller
   is expected to spin until mi_play goes false, then call
   again.

   When idle, walks the full startup sequence:
     1. Position mi_dbase at buffer + 0x1FE, the start of
        the MIDI event stream (which puts the 90-byte channel-map
        block + 360-byte envelope-parameter block behind it).
     2. Parse the song header configuration (tempo, channel count,
        etc.) via mq_parh.
     3. Reset all 16 MIDI program assignments so the current song
        starts each channel with the right instrument voice.
     4. Skip any leading 0x00/0xFF padding in the event stream.
     5. Store the current + max playback position for the audio
        driver's interrupt loop.
     6. Reset timing counters and kick the sequencer.

   addr: mq_inis() */

void
mq_inis(param_1, maxPos)
unsigned char * param_1;
long            maxPos;
{
        unsigned char * current_position;

        if (mi_play != NO) {
                g_mspha = SEQ_PHASE_SONG_ENDING;
                return;
        }

        mi_dbase = param_1 + 0x1fe;
        mq_parh(mi_dbase);
        mq_resp();
        current_position = mq_skip(mi_dbase,
                                                 maxPos);
        mq_setp(current_position, maxPos);
        mq_stap();
        mi_play = YES;
}

/* mq_parh: walk the song configuration commands.  The
   header runs from mi_dbase until the first 0xFF byte;
   commands come in three flavours: config commands (0x80/0x81/0x83/
   0x84) that update sequencer state, program-change events (0xC0),
   and note-event stride skips (any byte in range 0x01..0x7F, treated
   as a 3-byte MIDI-style event for the purpose of walking past it).

   Also parses the 90-byte channel/program-map block that precedes the
   header events.

   addr: mq_parh() */

void
mq_parh(p)
unsigned char * p;
{
        mq_pacm(p - 90);

        /* Skip a leading zero byte (used in .sng files where the
           channel-map block is padded to an even boundary). */
        if (*p == 0)
                p = p + 1;

        for (;;) {
                if (*p == 0)
                        return;

                /* Bytes in the note-event range 0x01..0x7F -- and
                   0xA0..0xFE via the & 0x9f mask that the 1985 code
                   used -- are 3-byte note events.  Skip past them. */
                if ((*p & 0x9f) < 0x20 && (*p & 0x9f) != 0) {
                        p = p + 3;
                        continue;
                }

                /* Config-command dispatch. */
                switch (*p) {
                case MIDI_HDR_SET_CHANNEL_COUNT:
                        p = mh_chac(p);
                        break;
                case MIDI_HDR_SET_TEMPO:
                        p = mh_temp(p);
                        break;
                case MIDI_HDR_SET_VOLUME:
                        p = mh_volu(p);
                        break;
                case MIDI_HDR_BUILD_SCALE_TABLE:
                        p = mh_scat(p);
                        break;
                case MIDI_HDR_PROGRAM_CHANGE:
                        p = mh_proc(p);
                        break;
                case MIDI_HDR_END:
                        return;
                default:
                        p = p + 1;
                        break;
                }
        }
}

/* mq_resp: pre-flight the 16 MIDI channels.  For each
   physical channel 0..15, finds the first logical channel in the map
   that references it, marks its current program as unset (-1), and
   dispatches a Program Change message to select the configured
   instrument.

   The inner-loop `chIndex = 15` trick (right before the `+ 1` step)
   is a clever 1985 mini-break: it forces the outer `for` to advance
   past 15 and terminate the inner iteration after finding the first
   match.  Preserved as an explicit `break`.

   addr: mq_resp() */

void
mq_resp()
{
        short   channel;
        short   ch_index;

        for (channel = 0; channel < 16; channel = channel + 1) {
                for (ch_index = 1; ch_index < 16;
                     ch_index = ch_index + 1) {
                        if ((mi_chmap[ch_index] & 0xf) == channel) {
                                g_mcpro[ch_index] = -1;
                                mq_sepc(ch_index);
                                break;
                        }
                }
        }
}

/* mq_skip: advance past leading 0x00 and the
   0x00-followed-by-0xFF pair used to mark "empty song start".  If
   neither prefix matches, walk forward until the first 0x00 -- this
   corner covers the "song already trimmed" path where the caller has
   already snipped the padding.

   addr: mq_skip() */

unsigned char *
mq_skip(ptr, position)
unsigned char * ptr;
long            position;
{
        (void) position;

        if (ptr == (unsigned char *) 0)
                return (unsigned char *) 0;

        /* The 1985 code's logic:
             if (*p == 0 && *(p+1) == 0xff)  -> return p (start-of-song marker)
             else                             -> walk until next 0x00.
           The condition is expressed via short-circuit assignment in
           the decompile; unpacked here for clarity. */
        if (ptr[0] == 0 && ptr[1] == 0xff)
                return ptr;

        while (*ptr != 0)
                ptr = ptr + 1;
        return ptr;
}

/* mq_setp: stash the read cursor + end-of-song marker,
   initialise the per-song audio-driver state (envelope base, velocity,
   PSG volume, event queue depth), and publish the tick-per-beat to
   the interrupt handler via aes_intO[7].

   The envelope base is exactly 360 bytes (0x168) behind the MIDI data
   base, matching the ADSR parameter block layout described at the top
   of this file.

   addr: mq_setp() */

void
mq_setp(curPos, maxPos)
unsigned char * curPos;
long            maxPos;
{
        mi_sqpos     = curPos;
        g_msmap = (maxPos == 0) ? -1 : maxPos;

        mi_env = (long) (mi_dbase - 0x168);
        mi_vel           = mi_dvel;
        psg_cvol      = psg_dvol;
        mi_evi    = 0;
        mi_evcn   = 9;
        aes_intO[7]          = g_mtspb;
}

/* mq_stap: initialise timer counters + arm the
   sequencer.  All 4 tick counters (divider, prescaler, event
   duration, next-event tick, last-processed tick) are seeded to 100,
   which gives the audio driver 100 200Hz ticks (~ 500 ms) of grace
   time before the first event fires -- enough for the caller's
   walk-into-the-dance-floor animation to catch up.

   mi_dwrm = 0 selects the "route MIDI bytes through
   XBIOS Midiws" path (as opposed to the direct-write ACIA register
   path used by a few speed-critical hot loops).

   addr: mq_stap() */

void
mq_stap()
{
        g_mtcou       = 0;
        mi_dwrm  = 0;
        g_mtdiv       = 100;
        g_mtpre     = 100;
        mi_nlp0    = 100;
        mi_nxTk    = 100;
        mi_lpTk= 100;
        g_msmsa   = YES;
        g_mspha          = SEQ_PHASE_PARSE_NEXT_EVENT;
}

/* mq_pacm: unpack the 30-byte channel/program map
   block sitting 90 bytes before mi_dbase.  The block is
   laid out as:
     bytes  0..14  MIDI channel assignment for logical channels 1..15
     bytes 15..29  MIDI program number for each logical channel 1..15
   All values are stored as 1-based on disk (so the file can use 0 as
   a "no-op" sentinel); we decrement on load.  Logical channel 0 is
   reserved for game SFX and is not touched here.

   addr: mq_pacm() */

void
mq_pacm(p)
unsigned char * p;
{
        short   i;

        for (i = 1; i < 16; i = i + 1) {
                mi_chmap[i] = p[i - 1]  - 1;
                mi_pgmap[i] = p[i + 14] - 1;
        }
}

/* mq_bust: (re)build the 132-note transpose LUT.
   Starts identity, then blanks (0xFF = skip) the 5 chromatic non-
   diatonic notes in the octave.  If the scale parameter is anything
   other than 1 (chromatic), applies a 7-bit chord mask from
   g_msmk[scale] per octave, shifting missing degrees
   by +1 (scale < 9) or -1 (scale >= 9) toward the nearest present
   degree.  This is how the game constrains melodies to pentatonic /
   blues / other scale flavours.

   The mask bits are ordered so bit 0 controls scale-degree-7 (the
   leading tone), bit 6 controls the root, matching how the 1985 code
   walks the octave from the top down.

   addr: mq_bust() */

void
mq_bust(value)
short   value;
{
        short           i;
        unsigned char   chord_mask;
        char            note_shift;

        /* Identity map first, then blank the chromatic-only notes in
           the first octave (indices 1, 3, 6, 8, 10 = C#, D#, F#, G#, A#). */
        for (i = 0; i < 0x84; i = i + 1)
                g_mstr[i] = (unsigned char) i;
        g_mstr[1]  = 0xff;
        g_mstr[3]  = 0xff;
        g_mstr[6]  = 0xff;
        g_mstr[8]  = 0xff;
        g_mstr[10] = 0xff;

        if (value == 1)
                return;

        note_shift = (value < 9) ? 1 : -1;
        chord_mask = g_msmk[value];

        /* Walk one octave per iteration and apply the mask to the 7
           diatonic-plus-one degree slots (indices 0, 2, 4, 5, 7, 9, 11
           within each 12-note octave).  Bit order matches the 1985
           source (bit 0 = degree 7 at offset +11, bit 6 = root at
           offset +0). */
        for (i = 0; i < 0x84; i = i + 12) {
                if ((chord_mask & 0x01) == 0)
                        g_mstr[i + 11] += note_shift;
                if ((chord_mask & 0x02) == 0)
                        g_mstr[i + 9]  += note_shift;
                if ((chord_mask & 0x04) == 0)
                        g_mstr[i + 7]  += note_shift;
                if ((chord_mask & 0x08) == 0)
                        g_mstr[i + 5]  += note_shift;
                if ((chord_mask & 0x10) == 0)
                        g_mstr[i + 4]  += note_shift;
                if ((chord_mask & 0x20) == 0)
                        g_mstr[i + 2]  += note_shift;
                if ((chord_mask & 0x40) == 0)
                        g_mstr[i]      += note_shift;
        }
}

/* mq_sepc: dispatch a Program Change (MIDI status
   byte 0xCn) for logical channel `index`.  Only fires if the logical
   channel's currently-cached program differs from the newly-mapped
   one AND MIDI output is enabled.  Note that current-program is keyed
   by the *physical* channel (via mi_chmap & 0x0f) so multiple
   logical channels sharing a physical MIDI channel only get one
   Program Change per song load.

   The 2-byte event {0xCn, program} is passed to the event dispatcher
   (which the audio driver later routes through XBIOS Midiws or direct
   ACIA writes).

   addr: mq_sepc() */

void
mq_sepc(index)
short   index;
{
        short   physical;

        physical = mi_chmap[index] & 0xf;
        if (g_mcpro[physical] == mi_pgmap[index])
                return;
        if (g_moen == NO)
                return;

        g_meve[0] = (mi_chmap[index] & 0xf) | 0xc0;
        g_meve[1] = (unsigned char) mi_pgmap[index];
        g_mcpro[physical] = mi_pgmap[index];
        mq_dise(g_meve, (short) 2, (short) 0);
}

/* ---- mq_dise ---------------------------------------- */

/* Send one MIDI event to both the external MIDI OUT port (via XBIOS
   Midiws) and the internal YM2149 PSG (which the game uses as its
   fallback tone source when no external MIDI device is connected).
   Both output paths are gated by their respective enabled flags, so
   the same event can go to one, both, or neither.

   MIDI OUT path:
     Apply an octave transposition (env_val - upper-nibble-of-
     midi_ch) * -12 semitones to the note byte, then either
     stream the bytes one-at-a-time through mowrit (when
     mi_dwrm is 1, used by speed-critical hot loops)
     or hand the whole event to Midiws.  The note byte is restored
     to its original value after the write so the PSG path below sees
     the untransposed note.

   PSG path (Note-On messages only, MIDI status 0x9n):
     Velocity 0 -> Note-Off: find the PSG channel currently playing
       that note (linear search 0..2), mark it silent, transition its
       envelope to ENV_RELEASE.
     Velocity > 0 -> Note-On:
       1. Try to allocate a silent PSG channel (linear search).
       2. If all 3 are busy: voice-steal the one furthest along in its
          envelope (highest phase number - closest to release).
       3. Guard against notes outside [g_mnlol,
          g_mnhil].
       4. memcpy 8 bytes of ADSR envelope parameters from the .SNG
          block at mi_env + (channel-1)*8 into the
          chosen channel's envelope struct.
       5. Compute the frequency-table octave offset: (2 - attack_
          duration.high_nibble) * 12 semitones.
       6. Write PSG tone period + mixer + noise-mask via either the
          direct psg_wr path or the XBIOS Giaccess path.
       7. If the resulting note is below the freq table's lowest
          playable entry (< 0x17), enter ENV_FADEOUT instead of the
          normal ENV_ATTACK.
       8. Wire the new state: max_volume from psg_cvol,
          phase_timer=1, psg_ntAc=YES.

   Returns 1 on a successful dispatch, 0 on a non-Note-On event that
   the PSG path can't handle (falls through to MIDI OUT only), or 0
   on a Note-Off miss (note wasn't playing on any channel).

   addr: mq_dise() */

short
mq_dise(midiEvP, midiEvS, midi_ch)
unsigned char * midiEvP;
short           midiEvS;
short           midi_ch;
{
        unsigned char * saved_ptr = midiEvP;
        unsigned char   saved_note;
        unsigned char * note_ptr;
        short           channel_idx;
        short           chosen;
        short           i;
        short           envelope_phase;
        char            cVar4;
        unsigned char   attack_hi;
        unsigned short  period;
        unsigned short  period_hi_nibble;
        unsigned short  freq_index;
        short           ret;

        /* ---- MIDI OUT path ---- */
        if (g_moen != NO) {
                saved_note = midiEvP[1];
                if (midi_ch != 0) {
                        short   octave_delta = env_val -
                                        ((midi_ch >> 4) & 0xf);
                        midiEvP[1] = (unsigned char)
                                (midiEvP[1] + (short) octave_delta * -12);
                }
                if (mi_dwrm == 1) {
                        while (midiEvS != 0) {
                                mowrit(*midiEvP);
                                midiEvP = midiEvP + 1;
                                midiEvS = midiEvS - 1;
                        }
                } else {
                        Midiws(midiEvS - 1, midiEvP);
                }
                saved_ptr[1] = saved_note;
        }

        /* ---- PSG path ---- */
        if (psg_out == NO)
                return 1;

        note_ptr = saved_ptr + 1;
        if ((*saved_ptr & 0xf0) != 0x90)
                return 0;

        /* ---- Note-Off (velocity == 0) ---- */
        if (saved_ptr[2] == 0) {
                for (channel_idx = 0; channel_idx < 3;
                     channel_idx = channel_idx + 1) {
                        if (psg_chNt[channel_idx] == *note_ptr)
                                break;
                }
                if (channel_idx >= 3)
                        return 0;
                psg_chNt[channel_idx] = 0;
                psg_envelope[channel_idx].phase       = ENV_RELEASE;
                psg_envelope[channel_idx].phase_timer = 0;
                return 1;
        }

        /* ---- Note-On: pick a channel ---- */
        for (chosen = 0; chosen < 3; chosen = chosen + 1) {
                if (psg_chNt[chosen] == 0)
                        break;
        }
        if (chosen == 3) {
                /* Voice-steal: pick the channel furthest along in its
                   envelope (highest phase index). */
                chosen = 0;
                for (i = 1; i < 3; i = i + 1) {
                        if (psg_envelope[i - 1].phase <
                            psg_envelope[i].phase)
                                chosen = i;
                }
        }

        /* Range guard. */
        if ((char) *note_ptr < g_mnlol ||
            (char) *note_ptr > g_mnhil)
                return 1;

        envelope_phase = ENV_ATTACK;

        /* Copy 8 bytes of ADSR params from the .SNG envelope block. */
        psg_cpE(
                (unsigned char *) (mi_env +
                        (long) (g_mccha - 1) * 8),
                (unsigned char *) &psg_envelope[chosen].attack_start_vol,
                8);

        /* Split the packed nibbles: attack_start_vol keeps its low
           4 bits (start volume), high 4 bits stash the mixer flags;
           attack_duration keeps its low 4 bits, high 4 bits encode
           the octave shift (2 - N) * 12 semitones. */
        attack_hi = psg_envelope[chosen].attack_start_vol;
        psg_envelope[chosen].attack_start_vol =
                psg_envelope[chosen].attack_start_vol & 0xf;
        cVar4 = (char) ((2 - ((psg_envelope[chosen].attack_duration >> 4) & 0xf)) * 12);
        psg_envelope[chosen].attack_duration =
                psg_envelope[chosen].attack_duration & 0xf;

        {
                unsigned short  mixer_bits =
                        (unsigned short) (((attack_hi >> 4) & 0xf) << chosen);
                unsigned short  noise_mask =
                        (unsigned short) ~(9 << chosen);
                freq_index = (unsigned short)
                        ((short) cVar4 + (short) (char) *note_ptr);

                if (mi_dwrm == 1) {
                        psg_wr((char) (psg_freq[freq_index] / 0x3c),
                                           (char) 6);
                        psg_mix((char) mixer_bits,
                                      (unsigned char) noise_mask | 0xc0);
                } else {
                        long    mixer_prev;
                        long    combined;
                        Giaccess((psg_freq[freq_index] / 0x3c), 0x86);
                        mixer_prev = Giaccess(0, 7);
                        combined = (long) mixer_bits |
                                   ((long) (noise_mask | 0xc0) & mixer_prev);
                        Giaccess((combined >> 16), combined);
                }
        }

        ret = chosen * 2;

        if ((short) ((short) cVar4 + (short) (char) *note_ptr) < 0x17) {
                envelope_phase = ENV_FADEOUT;
        } else {
                period = psg_freq[freq_index];
                period_hi_nibble = (period >> 8) & 0xf;
                if (mi_dwrm == 1) {
                        psg_wr((char) period,
                                           (char) ret);
                        psg_wr((char) period_hi_nibble,
                                           (char) (ret + 1));
                } else {
                        Giaccess((period & 0xff), (ret + 0x80));
                        Giaccess(period_hi_nibble, (ret + 0x81));
                }
        }

        psg_chNt[chosen] = *note_ptr;
        if (envelope_phase == ENV_FADEOUT)
                psg_envelope[chosen].current_volume = 0;
        psg_envelope[chosen].max_volume  = (unsigned char) psg_cvol;
        psg_envelope[chosen].phase_timer = 1;
        psg_ntAc                 = YES;
        psg_envelope[chosen].phase       = (char) envelope_phase;

        return 1;
}

/* ---- Timer-A interrupt dispatch ---------------------------------- */
/* mq_tick lives in source/tools/dk/mq_tick.s -- byte-faithful port of
   Ghidra 0x1219a.  It's assembly because the ROM version uses
   privileged move-sr instructions that Alcyon C 4.14 can't emit,
   and terminates in `rte` (not `rts`) so it's installed by Xbtimer
   directly, without a C wrapper. */

/* Forward declarations for the sequencer helpers ported below. */

/* mq_advs: full sequencer state-machine advance.  Runs from mq_tick
   when the prescaler expires.  Ghidra 0x111b0.

   Three phases:
     * WAIT_NOTE_EXPIRE (0):
         expire any queued notes whose duration has elapsed, then
         reload the prescaler with aes_intO[7] and move to
         PARSE_NEXT_EVENT.
     * PARSE_NEXT_EVENT (1):
         call mq_pars() to walk the next batch of events.  Returns
         0 on end-of-song (transition to SONG_ENDING) or non-zero
         with mi_nlp0 = ticks until the next event.
     * SONG_ENDING (2):
         expire remaining notes; if queue is now empty, kill the
         PSG channels and clear all sequencer-active flags.

   addr: midi_seq_advance_sequencer() */

void
mq_advs()
{
        short   res;

        if (g_mspha == SEQ_PHASE_WAIT_NOTE_EXPIRE) {
                mq_expN((short) g_mtcou - (short) mi_lpTk);
                mi_lpTk    = g_mtcou;
                g_mtpre    = aes_intO[7];
                g_mspha    = SEQ_PHASE_PARSE_NEXT_EVENT;
                mi_nxTk    = aes_intO[7] + mi_nxTk;
        } else if (g_mspha == SEQ_PHASE_PARSE_NEXT_EVENT) {
                g_mspha    = SEQ_PHASE_WAIT_NOTE_EXPIRE;
                mi_nlp0    = -1;
                res        = mq_pars();
                if (res == 0) {
                        g_mspha    = SEQ_PHASE_SONG_ENDING;
                        g_mtpre    = aes_intO[7];
                        mi_nxTk    = aes_intO[7] + mi_nxTk;
                } else {
                        mi_nxTk    = mi_nlp0 + mi_nxTk;
                        mi_nlp0    = (short) mi_nxTk - (short) g_mtcou;
                        if (mi_nlp0 > 0)
                                g_mtpre = mi_nlp0;
                }
        } else {
                mq_expN((short) g_mtcou - (short) mi_lpTk);
                mi_lpTk    = g_mtcou;
                g_mtpre    = aes_intO[7];
                mi_nxTk    = aes_intO[7] + mi_nxTk;
                if (mi_evi == 0) {
                        psg_envelope[2].phase = ENV_IDLE;
                        psg_envelope[1].phase = ENV_IDLE;
                        psg_envelope[0].phase = ENV_IDLE;
                        g_msmsa    = NO;
                        psg_ntAc   = NO;
                        mi_play    = NO;
                        psg_wr((char) 0, (char) 8);
                        psg_wr((char) 0, (char) 9);
                        psg_wr((char) 0, (char) 10);
                }
        }
}


/* psg_upEn: PSG software ADSR envelope processor.  Called at 50 Hz
   from mq_tick.  Steps each of the 3 PSG channels through its
   attack -> decay -> sustain -> release -> fadeout state machine.

   Each phase uses a Bresenham-style integer accumulator:
     ramp_delta = (target_vol - current_vol) * rate_table[timer]
     phase_timer = time_table[timer]  or  sustain_table[sd] / release_table[sd]
     each tick: accum += delta; while accum > 360, current_volume += direction; accum -= 360

   When phase_timer decrements to 0, the phase advances to the next
   step; if the phase's duration byte is 0, the transition happens
   immediately (Ghidra fall-through via goto to the next case).

   After computing current_volume, clamp to max_volume and write
   the result to the PSG amp register (8/9/10) via psg_wr.

   Preserves the Ghidra switch(fallthrough) as C gotos so the port
   stays byte-comparable.

   addr: psg_process_envelopes() */

void
psg_upEn()
{
        char    i;
        short   vol;
        short   out;

        i = 0;
        for (;;) {
                if (i > 2)
                        return;
                if (psg_envelope[(short) i].phase == ENV_IDLE)
                        goto next;

                switch (psg_envelope[(short) i].phase) {
                case ENV_ATTACK:
                        psg_envelope[(short) i].current_volume =
                                                 psg_envelope[(short) i].attack_start_vol;
                        psg_envelope[(short) i].phase = ENV_DECAY;
                        if (psg_envelope[(short) i].attack_duration == 0) {
                                psg_envelope[(short) i].current_volume =
                                                                 psg_envelope[(short) i].attack_target_vol;
                                psg_envelope[(short) i].phase_timer = 0;
                                goto do_decay;
                        }
                        psg_envelope[(short) i].phase_timer =
                                                 (short) psg_envelope[(short) i].attack_duration;
                        if (psg_envelope[(short) i].attack_target_vol <
                            psg_envelope[(short) i].attack_start_vol) {
                                psg_rmpD[(short) i] =
                                                          (short) psg_envelope[(short) i].attack_start_vol -
                                                          (short) psg_envelope[(short) i].attack_target_vol;
                                psg_envelope[(short) i].ramp_direction = -1;
                        } else {
                                psg_envelope[(short) i].ramp_direction = 1;
                                psg_rmpD[(short) i] =
                                                          (short) psg_envelope[(short) i].attack_target_vol -
                                                          (short) psg_envelope[(short) i].attack_start_vol;
                        }
                        psg_rmpD[(short) i] = psg_rmpD[(short) i] *
                                             mi_evrt[psg_envelope[(short) i].phase_timer];
                        psg_envelope[(short) i].phase_timer =
                                             mi_evtt[psg_envelope[(short) i].phase_timer];
                        psg_rmpA[(short) i] = 0;
                        break;

                case ENV_DECAY:
do_decay:
                        vol = psg_envelope[(short) i].phase_timer;
                        psg_envelope[(short) i].phase_timer =
                                                 psg_envelope[(short) i].phase_timer - 1;
                        if (vol < 1) {
                                if (psg_envelope[(short) i].decay_duration == 0) {
                                        psg_envelope[(short) i].current_volume =
                                                                          psg_envelope[(short) i].decay_target_vol;
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_sustain;
                                }
                                psg_envelope[(short) i].phase = ENV_SUSTAIN;
                                psg_envelope[(short) i].phase_timer =
                                                                 (short) psg_envelope[(short) i].decay_duration;
                                if (psg_envelope[(short) i].decay_target_vol <
                                    psg_envelope[(short) i].attack_target_vol) {
                                        psg_rmpD[(short) i] =
                                                                  (short) psg_envelope[(short) i].attack_target_vol -
                                                                  (short) psg_envelope[(short) i].decay_target_vol;
                                        psg_envelope[(short) i].ramp_direction = -1;
                                } else {
                                        psg_envelope[(short) i].ramp_direction = 1;
                                        psg_rmpD[(short) i] =
                                                                  (short) psg_envelope[(short) i].decay_target_vol -
                                                                  (short) psg_envelope[(short) i].attack_target_vol;
                                }
                                psg_rmpD[(short) i] = psg_rmpD[(short) i] *
                                                                          mi_evrt[psg_envelope[(short) i].phase_timer];
                                psg_envelope[(short) i].phase_timer =
                                                                          mi_evtt[psg_envelope[(short) i].phase_timer];
                                psg_rmpA[(short) i] = 0;
                        } else {
                                psg_rmpA[(short) i] = psg_rmpD[(short) i] +
                                                                          psg_rmpA[(short) i];
                                while (0x168 < psg_rmpA[(short) i]) {
                                        psg_envelope[(short) i].current_volume =
                                                                                  psg_envelope[(short) i].ramp_direction +
                                                                                  psg_envelope[(short) i].current_volume;
                                        psg_rmpA[(short) i] = psg_rmpA[(short) i] - 0x168;
                                }
                        }
                        break;

                case ENV_SUSTAIN:
do_sustain:
                        vol = psg_envelope[(short) i].phase_timer;
                        psg_envelope[(short) i].phase_timer =
                                                 psg_envelope[(short) i].phase_timer - 1;
                        if (vol < 1) {
                                if (psg_envelope[(short) i].sustain_duration == 0) {
                                        psg_envelope[(short) i].current_volume =
                                                                          psg_envelope[(short) i].sustain_target_vol;
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_release;
                                }
                                psg_envelope[(short) i].phase = ENV_RELEASE;
                                psg_envelope[(short) i].phase_timer =
                                                                 mi_evst[(short) psg_envelope[(short) i].sustain_duration];
                                if (psg_envelope[(short) i].sustain_target_vol <
                                    psg_envelope[(short) i].decay_target_vol) {
                                        psg_rmpD[(short) i] =
                                                                  (short) psg_envelope[(short) i].decay_target_vol -
                                                                  (short) psg_envelope[(short) i].sustain_target_vol;
                                        psg_envelope[(short) i].ramp_direction = -1;
                                } else {
                                        psg_envelope[(short) i].ramp_direction = 1;
                                        psg_rmpD[(short) i] =
                                                                  (short) psg_envelope[(short) i].sustain_target_vol -
                                                                  (short) psg_envelope[(short) i].decay_target_vol;
                                }
                                psg_rmpD[(short) i] = psg_rmpD[(short) i] *
                                                                          mi_evrl[(short) psg_envelope[(short) i].sustain_duration];
                                psg_rmpA[(short) i] = 0;
                        } else {
                                psg_rmpA[(short) i] = psg_rmpD[(short) i] +
                                                                          psg_rmpA[(short) i];
                                while (0x168 < psg_rmpA[(short) i]) {
                                        psg_envelope[(short) i].current_volume =
                                                                                  psg_envelope[(short) i].ramp_direction +
                                                                                  psg_envelope[(short) i].current_volume;
                                        psg_rmpA[(short) i] = psg_rmpA[(short) i] - 0x168;
                                }
                        }
                        break;

                case ENV_RELEASE:
do_release:
                        vol = psg_envelope[(short) i].phase_timer;
                        psg_envelope[(short) i].phase_timer =
                                                 psg_envelope[(short) i].phase_timer - 1;
                        if (vol < 1) {
                                if (psg_envelope[(short) i].release_duration == 0) {
                                        psg_envelope[(short) i].phase_timer = 0;
                                        goto do_fadeout;
                                }
                                psg_envelope[(short) i].phase = ENV_FADEOUT;
                                psg_envelope[(short) i].phase_timer =
                                                                 (short) psg_envelope[(short) i].release_duration;
                                psg_rmpD[(short) i] =
                                                          (short) psg_envelope[(short) i].current_volume;
                                psg_envelope[(short) i].ramp_direction = -1;
                                psg_rmpD[(short) i] = psg_rmpD[(short) i] *
                                                                          mi_evrt[psg_envelope[(short) i].phase_timer];
                                psg_envelope[(short) i].phase_timer =
                                                                          mi_evtt[psg_envelope[(short) i].phase_timer];
                                psg_rmpA[(short) i] = 0;
                        } else {
                                psg_rmpA[(short) i] = psg_rmpD[(short) i] +
                                                                          psg_rmpA[(short) i];
                                while (0x168 < psg_rmpA[(short) i]) {
                                        psg_envelope[(short) i].current_volume =
                                                                                  psg_envelope[(short) i].ramp_direction +
                                                                                  psg_envelope[(short) i].current_volume;
                                        psg_rmpA[(short) i] = psg_rmpA[(short) i] - 0x168;
                                }
                        }
                        break;

                case ENV_FADEOUT:
do_fadeout:
                        vol = psg_envelope[(short) i].phase_timer;
                        psg_envelope[(short) i].phase_timer =
                                                 psg_envelope[(short) i].phase_timer - 1;
                        if (vol < 1 ||
                            psg_envelope[(short) i].current_volume == 0) {
                                psg_envelope[(short) i].phase = ENV_IDLE;
                                psg_envelope[(short) i].current_volume = 0;
                        } else {
                                psg_rmpA[(short) i] = psg_rmpD[(short) i] +
                                                                          psg_rmpA[(short) i];
                                while (0x168 < psg_rmpA[(short) i]) {
                                        psg_envelope[(short) i].current_volume =
                                                                                  psg_envelope[(short) i].ramp_direction +
                                                                                  psg_envelope[(short) i].current_volume;
                                        psg_rmpA[(short) i] = psg_rmpA[(short) i] - 0x168;
                                }
                        }
                        break;
                }

                if (psg_envelope[(short) i].max_volume <
                    psg_envelope[(short) i].current_volume)
                        out = psg_envelope[(short) i].max_volume;
                else
                        out = psg_envelope[(short) i].current_volume;
                psg_wr((char) out, (char)(psg_rot[(short) i] - 0x80));

next:
                i = i + 1;
        }
}

/* ---- Sequencer helpers (called from mq_advs / mq_pars) ------------ */

/* mq_rdur: skip past 0x00 padding bytes at mi_sqpos, then peek at the
   next event's duration-index nibble.  If the next byte is a normal
   note (high bit clear), stash the tick-count for it into mi_nlp0;
   if it's a control byte (high bit set), zero out mi_nlp0.
   Preserves the pointer at mi_sqpos in-place.
   addr: midi_seq_read_note_duration() */

void
mq_rdur()
{
        for (; *mi_sqpos == 0; mi_sqpos = mi_sqpos + 1) ;
        if ((*mi_sqpos & 0x80) == 0)
                mi_nlp0 = (short)(mi_ndt[(short)(char) mi_sqpos[1] & 0x1f]
                                                          - 1) * g_mtspb;
        else
                mi_nlp0 = 0;
}

/* mq_pshl: push a loop marker onto mi_lstk.  Each level uses two
   entries: {return_addr, remaining_count = b - 1}.  Cap at 49
   entries (24 nested loops + slack).
   addr: midi_seq_push_loop() */

void
mq_pshl(a, b)
void *  a;
short   b;
{
        if (mi_evcn < 49) {
                mi_lstk[mi_evcn] = (long) a;
                mi_evcn = mi_evcn + 1;
                mi_lstk[mi_evcn] = (long)(short)(b - 1);
                mi_evcn = mi_evcn + 1;
        }
}

/* mq_popl: pop / decrement the top of the loop stack.  Returns the
   loop-start pointer if the count is non-zero (caller jumps back),
   else NULL (caller falls through past the loop end).
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
        mi_lstk[mi_evcn - 1] = mi_lstk[mi_evcn - 1] - 1;
        if (cnt == 0) {
                mi_evcn = mi_evcn - 2;
                ret = (unsigned char *) 0;
        }
        return ret;
}

/* mq_rmev: remove a 3-word entry from mi_evq at index `val`.  Shifts
   later entries down to close the gap.  Returns 1 if more events
   remain, 0 if the queue is now empty.
   addr: midi_seq_remove_event() */

short
mq_rmev(val)
short   val;
{
        short   res;
        short   i;

        if ((short)(val + 3) == mi_evi)
                res = 0;
        else {
                for (i = val; i < (short)(mi_evi - 3); i = i + 1)
                        mi_evq[i] = mi_evq[i + 3];
                res = 1;
        }
        mi_evi = mi_evi - 3;
        return res;
}

/* mq_snof: send a MIDI Note-Off (velocity 0) for a queued note.
   The `nptr` argument is a pointer into mi_evq[i+1] where slot i+1
   holds {note|flags} and slot i+2 holds the physical channel byte.
   Only fires if the note is within [g_mnlol, g_mnhil] and non-zero.
   addr: midi_seq_send_note_off() */

void
mq_snof(nptr)
short * nptr;
{
        if ((nptr[0] & 0x80) == 0) {
                g_meve[1] = (unsigned char) nptr[0];
                if ((char) g_meve[1] >= g_mnlol &&
                    (char) g_meve[1] <= g_mnhil && g_meve[1] != 0) {
                        g_meve[0] = ((unsigned char) nptr[1] & 0xf) + 0x90;
                        g_meve[2] = 0;
                        mq_dise(g_meve, (short) 3, (short) nptr[1]);
                }
        }
}

/* mq_expN: subtract `val` from each queued event's remaining
   duration; when it drops to zero or below, fire mq_snof for that
   note and remove it from the queue.  Called from mq_advs each
   time the sequencer-tick divider expires.
   addr: midi_seq_expire_notes() */

void
mq_expN(val)
short   val;
{
        short   r;
        short   i;

        for (i = 0; i < mi_evi; i = i + 3) {
                mi_evq[i] = mi_evq[i] - val;
                if (mi_evq[i] < 1) {
                        mq_snof(&mi_evq[i + 1]);
                        r = mq_rmev(i);
                        if (r != 0)
                                i = i - 3;
                }
        }
}

/* mq_spgm: send a MIDI Program Change (0xCn) if the target physical
   channel doesn't already have that program cached AND MIDI output
   is enabled.
   addr: midi_seq_send_program_change() */

void
mq_spgm(idx)
char    idx;
{
        short   physical;

        physical = (short) mi_chmap[(short) idx] & 0xf;
        if (g_mcpro[physical] == mi_pgmap[(short) idx] || g_moen == NO)
                return;
        g_meve[0] = (mi_chmap[(short) idx] & 0xf) | 0xc0;
        g_meve[1] = (unsigned char) mi_pgmap[(short) idx];
        g_mcpro[physical] = mi_pgmap[(short) idx];
        mq_dise(g_meve, (short) 2, (short) 0);
}

/* mq_qnne: queue a note-on event into mi_evq[] as three shorts
   {remaining_duration, note|sustain_flag, physical_channel}, and
   simultaneously dispatch the Note-On through mq_dise so the
   audio path hears it immediately.  The queue entry is what
   later fires the paired Note-Off via mq_expN + mq_snof.
   addr: midi_seq_queue_note_event() */

void
mq_qnne()
{
        if (mi_evi < 58) {
                mi_evq[mi_evi] = mi_nlp0;
                mi_evi = mi_evi + 1;
                if (mi_nnOn == 0)
                        mi_evq[mi_evi] = 0;
                else
                        mi_evq[mi_evi] = (short)(char) mi_cnot |
                                                          ((short) mi_lasT << 1);
                mi_evi = mi_evi + 1;
                mi_evq[mi_evi] = (short) mi_chmap[(short)(char) mi_ccha];
                mi_evi = mi_evi + 1;

                if ((char) mi_cnot <= g_mnhil &&
                    g_mnlol <= (char) mi_cnot) {
                        if (mi_slop == NO)
                                mi_ccha = (char) mi_varR;
                        else
                                mq_spgm(mi_ccha);

                        if (mi_nnOf != 0)
                                mi_nOS[(short)(char) mi_cnot] = 0;
                        if (mi_lasT != 0)
                                mi_nOS[(short)(char) mi_cnot] =
                                                          (unsigned char) mi_ccha;
                        if (mi_nnOf == 0) {
                                g_meve[0] = (mi_chmap[(short)(char) mi_ccha]
                                                                          & 0xf) | 0x90;
                                g_meve[1] = (unsigned char) mi_cnot;
                                g_meve[2] = (unsigned char) mi_vel;
                                mq_dise(g_meve, (short) 3,
                                                            (short) mi_chmap[(short)(char) mi_ccha]);
                        }
                }
        }
}

/* mq_pars: walk the compact event stream at mi_sqpos.
   Byte forms:
     0x00       -> tick separator; returns 1 with mi_nlp0 loaded
     0x01..0x7F -> note event, 3 bytes
                          byte0: bits 0-3 = logical channel,
                                       bit 4 = note-on trigger (inverted),
                                       bit 5 = sustain flag,
                                       bit 6 = note-off flag
                          byte1: bits 0-4 = duration index,
                                       bit 5 = accent (force velocity 0x7F),
                                       bits 6-7 = transpose mode
                          byte2: MIDI note number
     0x82       -> bar/position marker (1-byte, no payload)
     0x85 <n>   -> loop start, count=n
     0x86       -> loop end (jump back if count > 0)
     0xFF       -> end of song, returns 0

   Returns 1 if a timed event was decoded, 0 on end-of-song / stream
   exhausted.  Ghidra 0x10388 with all the nested while-loops preserved
   as gotos for byte-for-byte fidelity with the 1985 source.
   addr: midi_seq_parse_events() */

short
mq_pars()
{
        unsigned char * a;
        unsigned char   cmd;
        unsigned char   ebyte;
        unsigned char * next;
        unsigned char * pop_ptr;

        /* Prologue: skip a leading 0x00 (tick separator), refresh
           mi_nlp0 with the next note's duration, then sanity-check
           we haven't fallen off the end. */
        if (*mi_sqpos == 0) {
                mi_sqpos = mi_sqpos + 1;
                if (mi_sqpos < mi_seqE) {
                        mi_evTf = 0;
                        mq_rdur();
                        if (mi_sqpos >= mi_seqE)
                                return 0;
                } else {
                        return 0;
                }
        } else {
                return 0;
        }

        for (;;) {
top:
                if (*mi_sqpos == 0)
                        return 1;

                if ((*mi_sqpos & 0x80) == 0) {
                        /* Note event: unpack byte0, byte1, byte2 into
                           the per-event scratch globals, advance 3
                           bytes, then queue via mq_qnne.  Bit 5 of
                           byte1 (accent) forces max velocity + max
                           PSG volume; else copy the current defaults. */
                        mi_evTf  = 1;
                        mi_nnOn  = (char)(16 - (*mi_sqpos & 0x10));
                        mi_lasT  = (char)(*mi_sqpos & 0x40);
                        mi_nnOf  = (char)(*mi_sqpos & 0x20);
                        mi_ccha  = (char)(*mi_sqpos & 0x0f);

                        {
                                unsigned char * loop_ptr = mi_sqpos + 1;
                                mi_nlpA = (char)(*loop_ptr & 0x20);
                                if (mi_nlpA == 0) {
                                        mi_vel  = mi_dvel;
                                        psg_cvol = psg_dvol;
                                } else {
                                        mi_vel  = 0x7f;
                                        psg_cvol = 0xf;
                                }
                                ebyte    = *loop_ptr;
                                mi_nmof  = (char)(ebyte & 0xc0);
                                mi_nlp0  = (short)(mi_ndt[(short)(char) *loop_ptr & 0x1f]
                                                                          - 1) * g_mtspb;

                                if (((short)(char) mi_nmof & 0xc0) == 0) {
                                        mi_cnot = g_mstr[(short)(char) mi_sqpos[2] & 0x7f];
                                } else {
                                        mi_cnot = mi_sqpos[2] & 0x7f;
                                        if ((ebyte & 0x80) != 0) {
                                                if ((ebyte & 0x40) == 0)
                                                        mi_cnot = mi_cnot + 1;
                                                else
                                                        mi_cnot = mi_cnot - 1;
                                        }
                                }
                        }

                        mi_sqpos = mi_sqpos + 3;
                        if (mi_nnOn != 0)
                                mq_qnne();
                        goto top;
                }

                cmd  = *mi_sqpos;
                next = mi_sqpos + 1;

                if (cmd == 0x82) {
                        /* Bar marker: 1-byte, refresh mi_nlp0 for the
                           NEXT event's duration only if we haven't
                           already decoded one this pass. */
                        mi_sqpos = next;
                        if (mi_evTf == 0) {
                                mq_rdur();
                                if (mi_sqpos >= mi_seqE)
                                        return 0;
                        }
                        goto top;
                }
                if (cmd == 0x85) {
                        /* Loop start: mi_sqpos[1] is the repeat count,
                           push the return-address for the body. */
                        a = mi_sqpos + 2;
                        mi_sqpos = next;
                        mq_pshl((void *) a, (short)(char) *next);
                        mi_sqpos = mi_sqpos + 1;
                        mq_rdur();
                        if (mi_sqpos >= mi_seqE)
                                return 0;
                        goto top;
                }
                if (cmd == 0x86) {
                        /* Loop end: pop; if nonzero, jump back. */
                        mi_sqpos = next;
                        pop_ptr = mq_popl();
                        mi_dptr = pop_ptr;
                        if (pop_ptr != (unsigned char *) 0)
                                mi_sqpos = pop_ptr;
                        mq_rdur();
                        if (mi_sqpos >= mi_seqE)
                                return 0;
                        goto top;
                }
                if (cmd == 0xff)
                        return 0;

                mi_sqpos = next;
        }
}

/* mq_stop (Ghidra midi_seq_stop @ 0x1103c): stop MIDI sequencer
   playback.  Drains all pending events in the queue by advancing
   the tick counter past every scheduled event, then sends note-off
   (velocity=0) messages for every note flagged in mi_noSt[].
   Finally clears g_msmsa so mq_tick stops processing sequencer
   events on the next tick.
   Not called yet -- present for parity with the ROM.
   addr: mq_stop() */

void
mq_stop()
{
        short   note;
        BOOL16  hadPend;

        hadPend = (mi_evi > 0);

        while (mi_evi > 0) {
                mi_nlp0 = g_mtcou - mi_nxTk;
                if (mi_nlp0 > 0) {
                        mq_expN(mi_nlp0);
                        mi_nxTk = mi_nxTk + mi_nlp0;
                }
        }

        if (hadPend != NO) {
                g_meve[2] = 0;
                for (note = 0; note < 0x80; note = note + 1) {
                        if ((char) mi_noSt[note] != 0) {
                                g_meve[0] = (mi_chmap[(short)(char) mi_noSt[note]] & 0x0f) | 0x90;
                                g_meve[1] = (unsigned char) note;
                                mq_dise(g_meve, 3,
                                        mi_chmap[(short)(char) mi_noSt[note]]);
                        }
                }
        }

        g_msmsa = NO;
}

/* mq_extm (Ghidra midi_seq_exit_timer @ 0x11162): tear down the
   MFP Timer-A hook installed by mq_intim.  Restores the original
   MFP Timer-A interrupt vector saved into mi_svtv at boot.
   Xbtimer with ctrl=0 disables the timer and reinstalls the
   previous ISR from mi_svtv.
   Not called yet -- present for parity with the ROM.
   addr: mq_extm() */

void
mq_extm()
{
        Xbtimer(0, 0, 0x1c, mi_svtv);
}
