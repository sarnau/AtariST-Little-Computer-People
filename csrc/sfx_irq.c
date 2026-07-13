/*
 * sfx_irq.c -- SFX playback tick (XBIOS Dosound driver).
 *
 * soundeffect_irq_play is called from the 8Hz game loop (in
 * screen_render_8hz) whenever soundeffect_active_flag has been raised
 * by soundeffect_select.  It's the "commit" step of the queued-SFX
 * pipeline:
 *
 *   soundeffect_select   -> queue a candidate + set active_flag
 *   soundeffect_irq_play -> if allowed by priority + MIDI state:
 *                             copy Dosound bytes into the DMA buffer,
 *                             fire XBIOS Dosound, arm the countdown
 *
 * Playback is gated three ways:
 *   1. midi_is_playing -- MIDI takes exclusive PSG when running.
 *   2. soundeffect_playing_flag -- if another SFX is currently on,
 *      only preempt if the new one has higher priority (lower value).
 *   3. soundeffect_current_priority tracks the priority of whatever's
 *      currently playing, so priority-tied SFX don't fight.
 *
 * Duration is either taken from the SFX's own 4-byte trailer (last
 * word=hi, second-last=lo of a 32-bit tick count divided by 25 to
 * convert Dosound envelope time to game 8Hz ticks) or overridden by
 * soundeffect_duration when the caller pinned it to a specific value.
 *
 * addr: soundeffect_irq_play()
 */

#include "types.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern void     soundeffects_off();

/* CONCAT22: pack two 16-bit shorts into a 32-bit long, high-word
   first.  Ghidra emits this as a macro; we express it as a small
   inline for readability. */
static long
concat22(hi, lo)
short   hi;
short   lo;
{
        return ((long) (unsigned short) hi << 16) |
               (long) (unsigned short) lo;
}

/* Read the 200 Hz clock via GEMDOS Super mode.  Shared by
   screen_render_8hz -- but each file keeps its own copy to avoid
   pulling in an internal-linkage helper across translation units.  On
   host builds _hz_200_lo is 0 so the read is a no-op. */

static short
read_hz_200()
{
        void *  saveSSP;
        short   lo;

        saveSSP = (void *) _gemdos(GEMDOS_Super, 0L, 0L, 0L);
        lo = _hz_200_lo;
        _gemdos(GEMDOS_Super, (long) saveSSP, 0L, 0L);
        return lo;
}

/* soundeffect_irq_play: commit the queued SFX to the Dosound driver.
   addr: soundeffect_irq_play() */

void
soundeffect_irq_play()
{
        short           size;
        short           i;
        unsigned char * effectPtr;
        char *          dosound_ptr;
        short           new_priority;
        long            raw_ticks;

        /* MIDI has exclusive PSG access. */
        if (midi_is_playing != NO)
                return;

        new_priority = _soundeffect_priority_table[soundeffect_current];

        /* If something's already playing, only interrupt for a strictly
           higher-priority effect (lower priority number). */
        if (soundeffect_playing_flag != NO) {
                if (soundeffect_current_priority < new_priority)
                        return;
                soundeffects_off();
        }
        soundeffect_current_priority = new_priority;
        soundeffect_playing_flag     = YES;

        /* Fetch the SFX's Dosound sequence.  Layout:
             +0..1     size (short): number of bytes that follow
             +2..N     Dosound register-command stream
             (trailer) last 4 bytes = duration high/low words */
        effectPtr = midi_note_length_params[soundeffect_current];
        size      = *(short *) effectPtr;
        effectPtr = effectPtr + 2;

        /* Copy the sequence into the DMA-friendly buffer.  The 1985
           code walks the pointer past the end of the copy, which is
           fine because we know the trailing 4 bytes are the duration
           block we're about to overwrite with the Dosound terminator. */
        dosound_ptr = soundeffect_DoSound_Buffer;
        for (i = 0; i < size; i = i + 1) {
                *dosound_ptr = (char) *effectPtr;
                effectPtr    = effectPtr    + 1;
                dosound_ptr  = dosound_ptr  + 1;
        }

        /* Overwrite the last 4 bytes with the Dosound terminator
           sequence (0x00 0x00 0x00 0x00 = "stop").  The 1985 duration
           bytes we just copied are then read back from *before* the
           terminator via effectPtr[-4..-1]. */
        dosound_ptr[-4] = 0;
        dosound_ptr[-3] = 0;
        dosound_ptr[-2] = 0;
        dosound_ptr[-1] = 0;

        soundeffect_default_duration_hi = *(short *) (effectPtr - 4);
        soundeffect_default_duration_lo = *(short *) (effectPtr - 2);
        soundeffect_playing_id          = soundeffect_current;

        /* Hand the sequence to the XBIOS Dosound player.  Runs from
           the VBL interrupt on the ST; no-op on the host. */
        _xbios(XBIOS_Dosound,
               (long) soundeffect_DoSound_Buffer, 0L, 0L);

        /* Snapshot the 200 Hz counter for the countdown loop and
           compute the remaining ticks in game 8Hz units (Dosound
           envelope time / 25 = game ticks, since 200 Hz / 25 = 8 Hz). */
        soundeffect_Hz200 = (long) (unsigned short) read_hz_200();
        raw_ticks = concat22(soundeffect_default_duration_hi,
                             soundeffect_default_duration_lo);
        soundeffect_remaining_ticks = raw_ticks / 25L;

        /* Caller-pinned duration overrides the Dosound-derived one.
           Signal value -1 means "use the auto-computed duration". */
        if (soundeffect_duration != -1)
                soundeffect_remaining_ticks = (long) soundeffect_duration;
}
