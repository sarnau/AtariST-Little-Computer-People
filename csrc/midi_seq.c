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
 *      behind midi_seq_parse_channel_map, midi_seq_build_scale_table,
 *      midi_seq_send_program_change, and the interrupt-service loop
 *      that fires from the ST's 200 Hz timer.
 *
 *   3. XBIOS/BIOS:  Midiws (send raw MIDI bytes) and Giaccess (PSG
 *      register write).  Both already wired through host_xbios_trap.
 *
 * File-format provenance: .SNG and .ORG files are direct exports from
 * Activision Music Studio 2.0 (published 1986, Ed Bogas / Audio Light).
 * song_play strips a leading 10-byte Music Studio signature
 * (`\xCD` + "Mstudio" + `\xCD\x02`) before handing the rest of the file
 * to us; the layout below is offsets *inside the stripped body*, i.e.
 * inside the buffer song_play allocates.
 *
 * Stripped-body layout (relative to midi_data_base_ptr = start + 0x1FE):
 *
 *   body + 0x000..0x1A3    Music Studio config header:
 *                            +0x00..0x05  section tag "Blocks"
 *                            +0x1A..      instrument name list
 *                                         ("Harmonica", "Guitar", ...)
 *                            +0x??..      per-instrument ADSR envelope
 *                                         defaults, each 8 bytes
 *   body + 0x1A4..0x1FD    90-byte channel + program-change map
 *                          (15 logical channels x 2 bytes each; parsed
 *                          by midi_seq_parse_channel_map at p - 90)
 *   body + 0x1FE           MIDI event stream (this is midi_data_base_ptr)
 *
 * addr: midi_seq_init_song(), midi_seq_parse_header(),
 *       midi_seq_reset_programs(), midi_seq_skip_padding(),
 *       midi_seq_set_position(), midi_seq_start_playback()
 */

#include "types.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern void             midi_out_write_byte();
extern void             psg_copy_envelope_params();
extern void             psg_write_register();
extern void             psg_set_mixer();
extern short            midi_seq_dispatch_event();

/* Forward decls for the file's own functions -- our K&R style would
   normally rely on default-int declarations, but Clang under -Werror
   complains about the mixed short/long signatures below. */
extern void             midi_seq_parse_header();
extern void             midi_seq_parse_channel_map();
extern void             midi_seq_reset_programs();
extern void             midi_seq_send_program_change();
extern void             midi_seq_build_scale_table();
extern unsigned char *  midi_seq_skip_padding();
extern void             midi_seq_set_position();
extern void             midi_seq_start_playback();

/* Header-command handlers.  midi_seq_parse_header dispatches to these
   by matching the command byte against 0x80/0x81/0x83/0x84/0xC0/0xFF.
   The 1985 code used a jump table for the dispatch (which Ghidra
   couldn't recover); we use a plain switch, which the compiler
   naturally emits as a jump table under -O2.

   Individual handlers are stubs -- the actual per-command logic
   (parsing tempo bytes, walking the scale table, kicking a program
   change) lives in the deferred audio driver.  For now the header
   parser advances the pointer past each command, matching the original
   3-byte-per-command stride. */

static unsigned char *
midi_header_handle_channel_count(p)
unsigned char * p;
{
        (void) p;
        return p + 3;
}
static unsigned char *
midi_header_handle_tempo(p)
unsigned char * p;
{
        midi_tempo          = p[1];
        midi_ticks_per_beat = p[2];
        return p + 3;
}
static unsigned char *
midi_header_handle_volume(p)
unsigned char * p;
{
        midi_default_velocity = p[1];
        return p + 3;
}
static unsigned char *
midi_header_handle_scale_table(p)
unsigned char * p;
{
        midi_seq_build_scale_table();
        return p + 3;
}
static unsigned char *
midi_header_handle_program_change(p)
unsigned char * p;
{
        return p + 3;
}

/* midi_seq_init_song: song-lifecycle entry point.  When called with a
   song already playing, signals the current one to stop (the audio
   driver picks up the SEQ_PHASE_SONG_ENDING transition on its next
   interrupt) and returns without starting the new song -- the caller
   is expected to spin until midi_is_playing goes false, then call
   again.

   When idle, walks the full startup sequence:
     1. Position midi_data_base_ptr at buffer + 0x1FE, the start of
        the MIDI event stream (which puts the 90-byte channel-map
        block + 360-byte envelope-parameter block behind it).
     2. Parse the song header configuration (tempo, channel count,
        etc.) via midi_seq_parse_header.
     3. Reset all 16 MIDI program assignments so the current song
        starts each channel with the right instrument voice.
     4. Skip any leading 0x00/0xFF padding in the event stream.
     5. Store the current + max playback position for the audio
        driver's interrupt loop.
     6. Reset timing counters and kick the sequencer.

   addr: midi_seq_init_song() */

void
midi_seq_init_song(param_1, maxPosition)
unsigned char * param_1;
long            maxPosition;
{
        unsigned char * current_position;

        if (midi_is_playing != NO) {
                midi_seq_phase = SEQ_PHASE_SONG_ENDING;
                return;
        }

        midi_data_base_ptr = param_1 + 0x1fe;
        midi_seq_parse_header(midi_data_base_ptr);
        midi_seq_reset_programs();
        current_position = midi_seq_skip_padding(midi_data_base_ptr,
                                                 maxPosition);
        midi_seq_set_position(current_position, maxPosition);
        midi_seq_start_playback();
        midi_is_playing = YES;
}

/* midi_seq_parse_header: walk the song configuration commands.  The
   header runs from midi_data_base_ptr until the first 0xFF byte;
   commands come in three flavours: config commands (0x80/0x81/0x83/
   0x84) that update sequencer state, program-change events (0xC0),
   and note-event stride skips (any byte in range 0x01..0x7F, treated
   as a 3-byte MIDI-style event for the purpose of walking past it).

   Also parses the 90-byte channel/program-map block that precedes the
   header events.

   addr: midi_seq_parse_header() */

void
midi_seq_parse_header(p)
unsigned char * p;
{
        midi_seq_parse_channel_map(p - 90);

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
                        p = midi_header_handle_channel_count(p);
                        break;
                case MIDI_HDR_SET_TEMPO:
                        p = midi_header_handle_tempo(p);
                        break;
                case MIDI_HDR_SET_VOLUME:
                        p = midi_header_handle_volume(p);
                        break;
                case MIDI_HDR_BUILD_SCALE_TABLE:
                        p = midi_header_handle_scale_table(p);
                        break;
                case MIDI_HDR_PROGRAM_CHANGE:
                        p = midi_header_handle_program_change(p);
                        break;
                case MIDI_HDR_END:
                        return;
                default:
                        p = p + 1;
                        break;
                }
        }
}

/* midi_seq_reset_programs: pre-flight the 16 MIDI channels.  For each
   physical channel 0..15, finds the first logical channel in the map
   that references it, marks its current program as unset (-1), and
   dispatches a Program Change message to select the configured
   instrument.

   The inner-loop `chIndex = 15` trick (right before the `+ 1` step)
   is a clever 1985 mini-break: it forces the outer `for` to advance
   past 15 and terminate the inner iteration after finding the first
   match.  Preserved as an explicit `break`.

   addr: midi_seq_reset_programs() */

void
midi_seq_reset_programs()
{
        short   channel;
        short   ch_index;

        for (channel = 0; channel < 16; channel = channel + 1) {
                for (ch_index = 1; ch_index < 16;
                     ch_index = ch_index + 1) {
                        if ((midi_channel_map[ch_index] & 0xf) == channel) {
                                midi_current_program[ch_index] = -1;
                                midi_seq_send_program_change(ch_index);
                                break;
                        }
                }
        }
}

/* midi_seq_skip_padding: advance past leading 0x00 and the
   0x00-followed-by-0xFF pair used to mark "empty song start".  If
   neither prefix matches, walk forward until the first 0x00 -- this
   corner covers the "song already trimmed" path where the caller has
   already snipped the padding.

   addr: midi_seq_skip_padding() */

unsigned char *
midi_seq_skip_padding(ptr, position)
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

/* midi_seq_set_position: stash the read cursor + end-of-song marker,
   initialise the per-song audio-driver state (envelope base, velocity,
   PSG volume, event queue depth), and publish the tick-per-beat to
   the interrupt handler via aes_int_out[7].

   The envelope base is exactly 360 bytes (0x168) behind the MIDI data
   base, matching the ADSR parameter block layout described at the top
   of this file.

   addr: midi_seq_set_position() */

void
midi_seq_set_position(currentPosition, maxPosition)
unsigned char * currentPosition;
long            maxPosition;
{
        midi_seq_position     = currentPosition;
        midi_seq_max_position = (maxPosition == 0) ? -1 : maxPosition;

        midi_envelope_data_base = (long) (midi_data_base_ptr - 0x168);
        midi_velocity           = midi_default_velocity;
        psg_current_volume      = psg_default_volume;
        midi_note_event_index   = 0;
        midi_note_event_count   = 9;
        aes_int_out[7]          = midi_ticks_per_beat;
}

/* midi_seq_start_playback: initialise timer counters + arm the
   sequencer.  All 4 tick counters (divider, prescaler, event
   duration, next-event tick, last-processed tick) are seeded to 100,
   which gives the audio driver 100 200Hz ticks (~ 500 ms) of grace
   time before the first event fires -- enough for the caller's
   walk-into-the-dance-floor animation to catch up.

   midi_direct_write_mode = 0 selects the "route MIDI bytes through
   XBIOS Midiws" path (as opposed to the direct-write ACIA register
   path used by a few speed-critical hot loops).

   addr: midi_seq_start_playback() */

void
midi_seq_start_playback()
{
        midi_tick_counter       = 0;
        midi_direct_write_mode  = 0;
        midi_tick_divider       = 100;
        midi_tick_prescaler     = 100;
        midi_event_duration     = 100;
        midi_next_event_tick    = 100;
        midi_last_processed_tick= 100;
        midi_sequencer_active   = YES;
        midi_seq_phase          = SEQ_PHASE_PARSE_NEXT_EVENT;
}

/* midi_seq_parse_channel_map: unpack the 30-byte channel/program map
   block sitting 90 bytes before midi_data_base_ptr.  The block is
   laid out as:
     bytes  0..14  MIDI channel assignment for logical channels 1..15
     bytes 15..29  MIDI program number for each logical channel 1..15
   All values are stored as 1-based on disk (so the file can use 0 as
   a "no-op" sentinel); we decrement on load.  Logical channel 0 is
   reserved for game SFX and is not touched here.

   addr: midi_seq_parse_channel_map() */

void
midi_seq_parse_channel_map(p)
unsigned char * p;
{
        short   i;

        for (i = 1; i < 16; i = i + 1) {
                midi_channel_map[i] = p[i - 1]  - 1;
                midi_program_map[i] = p[i + 14] - 1;
        }
}

/* midi_seq_build_scale_table: (re)build the 132-note transpose LUT.
   Starts identity, then blanks (0xFF = skip) the 5 chromatic non-
   diatonic notes in the octave.  If the scale parameter is anything
   other than 1 (chromatic), applies a 7-bit chord mask from
   midi_scale_mask_table[scale] per octave, shifting missing degrees
   by +1 (scale < 9) or -1 (scale >= 9) toward the nearest present
   degree.  This is how the game constrains melodies to pentatonic /
   blues / other scale flavours.

   The mask bits are ordered so bit 0 controls scale-degree-7 (the
   leading tone), bit 6 controls the root, matching how the 1985 code
   walks the octave from the top down.

   addr: midi_seq_build_scale_table() */

void
midi_seq_build_scale_table(value)
short   value;
{
        short           i;
        unsigned char   chord_mask;
        char            note_shift;

        /* Identity map first, then blank the chromatic-only notes in
           the first octave (indices 1, 3, 6, 8, 10 = C#, D#, F#, G#, A#). */
        for (i = 0; i < 0x84; i = i + 1)
                midi_scale_transpose_table[i] = (unsigned char) i;
        midi_scale_transpose_table[1]  = 0xff;
        midi_scale_transpose_table[3]  = 0xff;
        midi_scale_transpose_table[6]  = 0xff;
        midi_scale_transpose_table[8]  = 0xff;
        midi_scale_transpose_table[10] = 0xff;

        if (value == 1)
                return;

        note_shift = (value < 9) ? 1 : -1;
        chord_mask = midi_scale_mask_table[value];

        /* Walk one octave per iteration and apply the mask to the 7
           diatonic-plus-one degree slots (indices 0, 2, 4, 5, 7, 9, 11
           within each 12-note octave).  Bit order matches the 1985
           source (bit 0 = degree 7 at offset +11, bit 6 = root at
           offset +0). */
        for (i = 0; i < 0x84; i = i + 12) {
                if ((chord_mask & 0x01) == 0)
                        midi_scale_transpose_table[i + 11] += note_shift;
                if ((chord_mask & 0x02) == 0)
                        midi_scale_transpose_table[i + 9]  += note_shift;
                if ((chord_mask & 0x04) == 0)
                        midi_scale_transpose_table[i + 7]  += note_shift;
                if ((chord_mask & 0x08) == 0)
                        midi_scale_transpose_table[i + 5]  += note_shift;
                if ((chord_mask & 0x10) == 0)
                        midi_scale_transpose_table[i + 4]  += note_shift;
                if ((chord_mask & 0x20) == 0)
                        midi_scale_transpose_table[i + 2]  += note_shift;
                if ((chord_mask & 0x40) == 0)
                        midi_scale_transpose_table[i]      += note_shift;
        }
}

/* midi_seq_send_program_change: dispatch a Program Change (MIDI status
   byte 0xCn) for logical channel `index`.  Only fires if the logical
   channel's currently-cached program differs from the newly-mapped
   one AND MIDI output is enabled.  Note that current-program is keyed
   by the *physical* channel (via midi_channel_map & 0x0f) so multiple
   logical channels sharing a physical MIDI channel only get one
   Program Change per song load.

   The 2-byte event {0xCn, program} is passed to the event dispatcher
   (which the audio driver later routes through XBIOS Midiws or direct
   ACIA writes).

   addr: midi_seq_send_program_change() */

void
midi_seq_send_program_change(index)
short   index;
{
        short   physical;

        physical = midi_channel_map[index] & 0xf;
        if (midi_current_program[physical] == midi_program_map[index])
                return;
        if (midi_output_enabled == NO)
                return;

        midi_event[0] = (midi_channel_map[index] & 0xf) | 0xc0;
        midi_event[1] = (unsigned char) midi_program_map[index];
        midi_current_program[physical] = midi_program_map[index];
        midi_seq_dispatch_event(midi_event, (short) 2, (short) 0);
}

/* ---- midi_seq_dispatch_event ---------------------------------------- */

/* Send one MIDI event to both the external MIDI OUT port (via XBIOS
   Midiws) and the internal YM2149 PSG (which the game uses as its
   fallback tone source when no external MIDI device is connected).
   Both output paths are gated by their respective enabled flags, so
   the same event can go to one, both, or neither.

   MIDI OUT path:
     Apply an octave transposition (envelope_val - upper-nibble-of-
     midiChannel) * -12 semitones to the note byte, then either
     stream the bytes one-at-a-time through midi_out_write_byte (when
     midi_direct_write_mode is 1, used by speed-critical hot loops)
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
       3. Guard against notes outside [midi_note_lo_limit,
          midi_note_hi_limit].
       4. memcpy 8 bytes of ADSR envelope parameters from the .SNG
          block at midi_envelope_data_base + (channel-1)*8 into the
          chosen channel's envelope struct.
       5. Compute the frequency-table octave offset: (2 - attack_
          duration.high_nibble) * 12 semitones.
       6. Write PSG tone period + mixer + noise-mask via either the
          direct psg_write_register path or the XBIOS Giaccess path.
       7. If the resulting note is below the freq table's lowest
          playable entry (< 0x17), enter ENV_FADEOUT instead of the
          normal ENV_ATTACK.
       8. Wire the new state: max_volume from psg_current_volume,
          phase_timer=1, psg_notes_active=YES.

   Returns 1 on a successful dispatch, 0 on a non-Note-On event that
   the PSG path can't handle (falls through to MIDI OUT only), or 0
   on a Note-Off miss (note wasn't playing on any channel).

   addr: midi_seq_dispatch_event() */

short
midi_seq_dispatch_event(midiEventPtr, midiEventSize, midiChannel)
unsigned char * midiEventPtr;
short           midiEventSize;
short           midiChannel;
{
        unsigned char * saved_ptr = midiEventPtr;
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
        if (midi_output_enabled != NO) {
                saved_note = midiEventPtr[1];
                if (midiChannel != 0) {
                        short   octave_delta = envelope_val -
                                        ((midiChannel >> 4) & 0xf);
                        midiEventPtr[1] = (unsigned char)
                                (midiEventPtr[1] + (short) octave_delta * -12);
                }
                if (midi_direct_write_mode == 1) {
                        while (midiEventSize != 0) {
                                midi_out_write_byte(*midiEventPtr);
                                midiEventPtr = midiEventPtr + 1;
                                midiEventSize = midiEventSize - 1;
                        }
                } else {
                        _xbios(XBIOS_Midiws,
                               (long) (midiEventSize - 1),
                               (long) midiEventPtr, 0L);
                }
                saved_ptr[1] = saved_note;
        }

        /* ---- PSG path ---- */
        if (psg_output_enabled == NO)
                return 1;

        note_ptr = saved_ptr + 1;
        if ((*saved_ptr & 0xf0) != 0x90)
                return 0;

        /* ---- Note-Off (velocity == 0) ---- */
        if (saved_ptr[2] == 0) {
                for (channel_idx = 0; channel_idx < 3;
                     channel_idx = channel_idx + 1) {
                        if (psg_channel_notes[channel_idx] == *note_ptr)
                                break;
                }
                if (channel_idx >= 3)
                        return 0;
                psg_channel_notes[channel_idx] = 0;
                psg_envelope[channel_idx].phase       = ENV_RELEASE;
                psg_envelope[channel_idx].phase_timer = 0;
                return 1;
        }

        /* ---- Note-On: pick a channel ---- */
        for (chosen = 0; chosen < 3; chosen = chosen + 1) {
                if (psg_channel_notes[chosen] == 0)
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
        if ((char) *note_ptr < midi_note_lo_limit ||
            (char) *note_ptr > midi_note_hi_limit)
                return 1;

        envelope_phase = ENV_ATTACK;

        /* Copy 8 bytes of ADSR params from the .SNG envelope block. */
        psg_copy_envelope_params(
                (unsigned char *) (midi_envelope_data_base +
                        (long) (midi_current_channel - 1) * 8),
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

                if (midi_direct_write_mode == 1) {
                        psg_write_register((char) (psg_freq_table[freq_index] / 0x3c),
                                           (char) 6);
                        psg_set_mixer((char) mixer_bits,
                                      (unsigned char) noise_mask | 0xc0);
                } else {
                        long    mixer_prev;
                        long    combined;
                        _xbios(XBIOS_Giaccess,
                               (long) (psg_freq_table[freq_index] / 0x3c),
                               0x86L, 0L);
                        mixer_prev = _xbios(XBIOS_Giaccess, 0L, 7L, 0L);
                        combined = (long) mixer_bits |
                                   ((long) (noise_mask | 0xc0) & mixer_prev);
                        _xbios(XBIOS_Giaccess,
                               (long) (short) (combined >> 16),
                               (long) (short) combined, 0L);
                }
        }

        ret = chosen * 2;

        if ((short) ((short) cVar4 + (short) (char) *note_ptr) < 0x17) {
                envelope_phase = ENV_FADEOUT;
        } else {
                period = psg_freq_table[freq_index];
                period_hi_nibble = (period >> 8) & 0xf;
                if (midi_direct_write_mode == 1) {
                        psg_write_register((char) period,
                                           (char) ret);
                        psg_write_register((char) period_hi_nibble,
                                           (char) (ret + 1));
                } else {
                        _xbios(XBIOS_Giaccess,
                               (long) (period & 0xff),
                               (long) (ret + 0x80), 0L);
                        _xbios(XBIOS_Giaccess,
                               (long) period_hi_nibble,
                               (long) (ret + 0x81), 0L);
                }
        }

        psg_channel_notes[chosen] = *note_ptr;
        if (envelope_phase == ENV_FADEOUT)
                psg_envelope[chosen].current_volume = 0;
        psg_envelope[chosen].max_volume  = (unsigned char) psg_current_volume;
        psg_envelope[chosen].phase_timer = 1;
        psg_notes_active                 = YES;
        psg_envelope[chosen].phase       = (char) envelope_phase;

        return 1;
}
