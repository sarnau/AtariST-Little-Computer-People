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
 * addr: soundeffect_select(), soundeffects_off(),
 *       play_soundeffect_*(), record_player_animate_needle()
 */

#include "types.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

/* midi_song_max_position declared in globals.h */

/* soundeffect_select: priority-based SFX queue insertion.  A new SFX
   only wins if no SFX is currently active OR the new priority is <=
   the current priority (lower value = higher priority in the 1985
   convention -- confirmed by the phone-ring-preempts-footstep observed
   behaviour).
   addr: soundeffect_select() */

void
soundeffect_select(sound_id, duration)
short   sound_id;
long    duration;
{
        if (soundeffect_active_flag == NO ||
            _soundeffect_priority_table[sound_id] <=
            _soundeffect_priority_table[soundeffect_current]) {
                soundeffect_current     = sound_id;
                soundeffect_duration    = (short) duration;
                soundeffect_active_flag = YES;
        }
}

/* soundeffects_off: silence all 3 PSG channels via XBIOS Giaccess (regs
   0x08/0x09/0x0a with high bit set = write-mode) and reset the Dosound
   sequencer state.
   addr: soundeffects_off() */

void
soundeffects_off()
{
        _xbios(XBIOS_Giaccess, 0L, 0x88L, 0L);
        _xbios(XBIOS_Giaccess, 0L, 0x89L, 0L);
        _xbios(XBIOS_Giaccess, 0L, 0x8aL, 0L);
        soundeffect_dosound_status  = 0xff;
        soundeffect_dosound_control = 0;
        soundeffect_playing_flag    = NO;
}

/* One-line SFX wrappers used by animation code.  Each just picks the
   right (id, duration) pair from Ghidra and hands off to
   soundeffect_select.  Duration units are 8Hz ticks. */

void play_soundeffect_tv_click(void)    { soundeffect_select(SFX_TV_CLICK,  2L); }
void play_soundeffect_greeting(void)    { soundeffect_select(SFX_GREETING,  2L); }
void play_soundeffect_speech(void)      { soundeffect_select(SFX_SPEECH,    3L); }
void play_soundeffect_head_nod(void)    { soundeffect_select(SFX_HEAD_NOD,  2L); }
void play_doorbell_sound(void)          { soundeffect_select(SFX_DOORBELL,  4L); }

/* Small SFX wrappers used by the write-letter routine.  Both are
   1-line trampolines into soundeffect_select with per-effect duration.
   addr: letter_select_typewriter_sound(), select_random_click_sound() */

void
letter_select_typewriter_sound()
{
        soundeffect_select(SFX_TYPEWRITER_KEY, 4L);
}

void
select_random_click_sound()
{
        soundeffect_select(SFX_CLICK, 2L);
}

/* song_play: load and start a .sng / .org song file from disk.
   1. If a song is already playing, wait for it to end.
   2. Free any previously-allocated buffer.
   3. Use GEMDOS Fsfirst to fetch the file's DTA (d_length gives size).
   4. Malloc a buffer of that size.
   5. Open the file, skip the 10-byte header, read up to 20000 bytes.
   6. Kick off the sequencer via midi_seq_init_song.

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
   addr: song_play() */

extern void     midi_seq_init_song();
extern short    file_open();
extern void     file_read();
extern void     error_not_enough_memory();

/* soundeffects_load: load the SOUNDS.LCP sound-effect data file.
   Format: a sequence of records, each `{size:short, dosound_bytes[size]}`,
   terminated by a size=0 record.  Up to 500 records total (Ghidra's
   safety cap; the real file has ~30).

   Each SFX gets its own GEMDOS_Malloc'd block laid out as:
     [0..1]     size (repeated inside the block so soundeffect_irq_play
                can read it back via `*(short *)midi_note_length_params[id]`)
     [2..2+N]   Dosound register-command stream, ending in a 4-byte
                duration trailer that soundeffect_irq_play reads as
                the SFX's playback length
   The pointer is stashed in midi_note_length_params[id] where the
   dispatch layer picks it up.

   addr: soundeffects_load() */

void
soundeffects_load()
{
        short           fhandle;
        short           index;
        short           size;
        short *         block;

        fhandle = file_open("sounds.lcp", 0);
        for (index = 0; index < 500; index = index + 1) {
                unsigned char   sizebuf[2];

                file_read(fhandle, 2L, sizebuf);
                /* On-disk size word is big-endian (68k native).  On the
                   ST the file_read would splice it into `size` in the
                   right order; on the host we reassemble explicitly. */
                size = ((short) sizebuf[0] << 8) | sizebuf[1];
                if (size == 0)
                        break;
                block = (short *) _gemdos(GEMDOS_Malloc,
                                          (long) (size + 4), 0L, 0L);
                midi_note_length_params[index] = (unsigned char *) block;
                if (block == (short *) 0)
                        error_not_enough_memory();
                *block = size;
                file_read(fhandle, (long) size, block + 1);
        }
        _gemdos(GEMDOS_Fclose, (long) fhandle, 0L, 0L);
}

/* DTA layout matches actions_leisure.c's local typedef; kept lightweight
   here so we don't have to pull the whole file-directory abstraction in. */
struct DTA_hdr {
        char    _reserved[21];
        char    d_attrib;
        long    d_time;
        long    d_length;
        char    d_fname[14];
};

void
song_play(filename)
char *  filename;
{
        struct DTA_hdr *dta_ptr;
        short           fileHandle;
        unsigned char   temp[10];

        midi_song_loop_flag = YES;
        midi_var_r          = YES;

        if (midi_is_playing != NO) {
                midi_seq_init_song(midi_song_buffer, midi_song_max_position);
                while (midi_is_playing != NO)
                        ;
        }
        if (midi_song_buffer != (char *) 0) {
                _gemdos(GEMDOS_Mfree, (long) midi_song_buffer, 0L, 0L);
                midi_song_buffer = (char *) 0;
        }

        _gemdos(GEMDOS_Fsfirst, (long) filename, 0L, 0L);
        dta_ptr = (struct DTA_hdr *) _gemdos(GEMDOS_Fgetdta, 0L, 0L, 0L);
        midi_song_buffer = (char *) _gemdos(GEMDOS_Malloc,
                                            dta_ptr->d_length, 0L, 0L);
        if (midi_song_buffer == (char *) 0)
                error_not_enough_memory();

        fileHandle = file_open(filename, 0);
        if (fileHandle >= 0) {
                file_read(fileHandle, 10L, temp);
                file_read(fileHandle, 20000L, midi_song_buffer);
                _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);
        }
        midi_seq_init_song(midi_song_buffer, midi_song_max_position);
}
