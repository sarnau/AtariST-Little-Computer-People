# Little Computer People — Sound & Music System

The game produces sound through three independent subsystems that share the
Atari ST's Yamaha YM2149 PSG (Programmable Sound Generator) chip:

1. **MIDI Sequencer** — plays `.sng` and `.org` music files with 3-channel PSG synthesis and optional external MIDI output
2. **PSG Envelope Processor** — software ADSR envelopes for the 3 PSG tone channels during music playback
3. **Sound Effect Engine** — plays ambient DoSound sequences from `sounds.lcp` with priority-based preemption

All three systems are interrupt-driven and run independently of the main game loop.

## Hardware: Yamaha YM2149 PSG

The Atari ST's sound chip provides 3 square-wave tone channels (A, B, C), 1 noise
generator, and a hardware envelope generator. Each channel has a 12-bit period
register (fine + coarse) and a 4-bit volume register (0–15). The game accesses
registers via XBIOS `Giaccess` calls (registers 0–13) and uses the XBIOS `Dosound`
interrupt-driven command interpreter for sound effects.

| Register | Function |
|---|---|
| 0–1 | Channel A period (fine/coarse) |
| 2–3 | Channel B period (fine/coarse) |
| 4–5 | Channel C period (fine/coarse) |
| 6 | Noise period |
| 7 | Mixer control (tone/noise enable per channel) |
| 8 | Channel A amplitude (0–15, or bit 4 = use hardware envelope) |
| 9 | Channel B amplitude |
| 10 | Channel C amplitude |
| 11–12 | Hardware envelope period |
| 13 | Hardware envelope shape |

The game does **not** use the hardware envelope generator. Instead, it implements
software ADSR envelopes in `psg_process_envelopes()`, giving much finer control
over volume shaping.

## Data Files

| File | Format | Purpose |
|---|---|---|
| `sounds.lcp` | DoSound sequences | 26 sound effects (footsteps, doors, bells, etc.) |
| `*.sng` | Custom MIDI + PSG envelope | Background music songs |
| `*.org` | Same format as .sng | Organ/piano music for record player and piano actions |

---

## 1. MIDI Sequencer Engine

### Overview

A custom MIDI-like sequencer engine (`midi_seq_*` functions, 24 total) plays
`.sng` song files. It supports dual output: external MIDI via the Atari ST's
ACIA port, and internal PSG synthesis via direct YM2149 register writes. Both
outputs can operate simultaneously or independently, controlled by
`midi_output_enabled` and `psg_output_enabled` flags.

### Timing Architecture

The sequencer is driven by the **MFP Timer A** interrupt at 200 Hz:

```
midi_seq_tick_handler (200 Hz IRQ)
  |-- midi_tick_counter++
  |-- midi_tick_prescaler-- (tempo-scaled subdivider)
  |-- midi_tick_divider-- (event timing)
  +-- if prescaler expired and not re-entrant:
        midi_seq_advance_sequencer()
          |-- midi_seq_expire_notes() — release expired notes
          |-- midi_seq_parse_events() — read next events from stream
          +-- midi_seq_dispatch_event() — send note-on/off to PSG/MIDI
```

The `midi_tick_prescaler` controls the effective tempo. It counts down from
`midi_tempo` and triggers sequencer advancement when it reaches zero. The
`midi_tick_divider` provides finer event-level timing within each prescaler
period.

A re-entrancy lock (`midi_reentrant_lock`) prevents the sequencer from being
interrupted by another timer tick while still processing the previous one.

### Sequencer State Machine (MIDI_SEQ_PHASE enum)

| Phase | Value | Meaning |
|---|---|---|
| `SEQ_PHASE_WAIT_NOTE_EXPIRE` | 0 | Waiting for current note duration to expire |
| `SEQ_PHASE_PARSE_NEXT_EVENT` | 1 | Ready to parse next event from stream |
| `SEQ_PHASE_SONG_ENDING` | 2 | Song stop requested, finishing current notes |

### Event Parsing (`midi_seq_parse_events`)

The event stream is a compact bytecode format (not standard MIDI):

| Byte Value | Event Type | Size | Description |
|---|---|---|---|
| 0x00 | Padding | 1 | End-of-bar marker, advance to next position |
| 0x01–0x7F | Note event | 3 | Packed note with duration, accent, transpose |
| 0x82 | Bar marker | 1 | Sync/position marker |
| 0x85 | Loop start | 5 | Push {return_addr, count} to loop stack |
| 0x86 | Loop end | 1 | Pop stack, decrement count, jump if > 0 |
| 0xFF | End of song | 1 | Stop sequencer or loop to beginning |

### Note Event Encoding (3 bytes)

```
Byte 0: [bit 7: always 0] [bit 6: note-on trigger] [bits 0-5: note index]
Byte 1: [bits 6-7: transpose mode] [bit 5: accent flag] [bits 0-4: duration index]
Byte 2: [bit 7: always 0] [bits 0-6: MIDI note number 0-127]
```

| Field | Bits | Values |
|---|---|---|
| Note-on trigger | byte0 bit 6 | 1 = play note, 0 = rest/continuation |
| Duration index | byte1 bits 0–4 | Index into `midi_note_duration_table[]` (0–25) |
| Accent flag | byte1 bit 5 | 1 = force velocity 0x7F (maximum), 0 = use default |
| Transpose mode | byte1 bits 6–7 | 0 = apply scale table, 1–3 = raw note |
| MIDI note | byte2 bits 0–6 | Standard MIDI note number (0–127) |

### Scale/Transpose System (`midi_seq_build_scale_table`)

The sequencer includes a scale quantization system that constrains notes to
specific musical scales:

1. Initialize 132-entry identity table (note N -> N)
2. Mark 5 chromatic notes as 0xFF (skip): C#, D#, F#, G#, A# (indices 1,3,6,8,10)
3. Apply a 7-bit chord mask from `midi_scale_mask_table[]` to enable/disable
   specific scale degrees within each octave
4. For scale values < 9: shift active notes up (+1 semitone)
5. For scale values >= 9: shift active notes down (-1 semitone)

This allows the sequencer to quantize melodies to pentatonic, blues, major, minor,
and other scales by remapping notes through `midi_scale_transpose_table[]`.

### Loop System

The sequencer supports nested loops via a stack (`midi_loop_stack[]`):

- **0x85 (Loop start)**: pushes the current position and a repeat count onto the stack
- **0x86 (Loop end)**: pops the stack, decrements count; if > 0, jumps back to saved position
- Used for repeating musical phrases within a song

### Note Queue

Notes are queued in `midi_note_event_queue[]` (capacity 58 entries, 3 shorts per entry)
via `midi_seq_queue_note_event()`. Each entry stores the note duration, note value
(combined with timing info), and mapped channel. Notes are expired by
`midi_seq_expire_notes()` which sends note-off events when their duration completes.

### Event Dispatch (`midi_seq_dispatch_event`)

The largest sound function (154 lines). Routes MIDI events to both outputs:

**External MIDI** (`midi_output_enabled`):
- Applies octave transposition based on channel mapping
- Sends via `_xbios(XBIOS_Midiws)` or direct ACIA byte writes (`midi_out_write_byte`)

**Internal PSG** (`psg_output_enabled`):
- Note-on (status 0x90): looks up frequency in `psg_frequency_table[]`, writes to PSG
  period registers, triggers ADSR envelope
- Note-off (status 0x90 with velocity 0): finds matching channel, triggers envelope release
- Program change (status 0xC0): loads PSG envelope parameters from the song data
- Allocates notes to the 3 available PSG channels, tracking active notes in
  `psg_channel_notes[]`

### Channel Mapping (`midi_seq_parse_channel_map`)

A 90-byte block preceding the song header defines how MIDI channels map to
PSG channels and what program (instrument) each uses. Parsed into
`midi_channel_map[]` and `midi_channel_program[]` arrays.

### Song Playback Flow

```
song_play("filename.sng")
  1. Stop any currently playing song
  2. Free previous song buffer
  3. Get file size via GEMDOS Fsfirst
  4. Allocate buffer via GEMDOS Malloc
  5. Open file, skip 10-byte header, read up to 20,000 bytes
  6. Call midi_seq_init_song(buffer, max_position)
       a. Set midi_data_base_ptr = buffer + 0x1FE
       b. Parse header commands (midi_seq_parse_header)
       c. Reset all MIDI programs (midi_seq_reset_programs)
       d. Skip leading padding (midi_seq_skip_padding)
       e. Set playback position
       f. Start interrupt-driven sequencer (midi_seq_start_playback)
       g. Set midi_is_playing = true
```

### Record Player / Piano Playback

When the LCP plays the record player (`action_play_with_record`) or piano
(`action_play_piano`), the game:

1. Scans the data directory for `*.sng` or `*.org` files using GEMDOS Fsfirst/Fsnext
2. Selects a random file from those found
3. Calls `song_play()` to start playback
4. Animates the LCP based on real-time PSG volume levels:
   - Reads PSG registers 8, 9, 10 (channel A/B/C amplitude) via `XBIOS Giaccess`
   - If any channel exceeds the previous frame's volume, switch to an active
     dance/playing pose
   - Otherwise switch to idle pose
5. Loops until `midi_is_playing` becomes false (song ends)

---

## 2. PSG Envelope Processor

### Overview

Since the YM2149's hardware envelope generator can only control one channel at a
time with limited shapes, the game implements full software ADSR envelopes for all
3 PSG channels. The processor runs at 50 Hz (every 4th call of the 200 Hz timer)
via `psg_process_envelopes()`.

### PSG_ENVELOPE Struct (14 bytes per channel)

```c
typedef struct {
    byte  phase;              // Current envelope phase (ENV_PHASE enum)
    byte  attack_start_vol;   // Initial volume at note-on
    byte  attack_duration;    // Ticks to reach attack target
    byte  attack_target_vol;  // Peak volume after attack
    byte  decay_duration;     // Ticks from peak to sustain level
    byte  decay_target_vol;   // Volume at end of decay
    byte  sustain_duration;   // Ticks to hold sustain
    byte  sustain_target_vol; // Volume during sustain
    byte  release_duration;   // Ticks to fade to silence
    byte  max_volume;         // Clamp ceiling for output
    byte  phase_timer;        // Countdown within current phase
    byte  current_volume;     // Current output volume (0-15)
    byte  ramp_direction;     // +1 or -1 for volume interpolation
} PSG_ENVELOPE;
```

Three instances: `psg_envelope[0]`, `psg_envelope[1]`, `psg_envelope[2]` for
channels A, B, C respectively.

### Envelope Phases (ENV_PHASE enum)

| Phase | Value | Behavior |
|---|---|---|
| `ENV_IDLE` | 0 | Channel silent, no processing |
| `ENV_ATTACK` | 1 | Set start volume, immediate transition to DECAY |
| `ENV_DECAY` | 2 | Ramp from attack_target_vol toward decay_target_vol |
| `ENV_SUSTAIN` | 3 | Ramp to sustain_target_vol, hold for sustain_duration |
| `ENV_RELEASE` | 4 | Ramp down to silence |
| `ENV_FADEOUT` | 5 | Final fadeout (post-release cleanup) |

### Volume Interpolation

The envelope processor uses **Bresenham-style integer interpolation** for smooth
volume transitions between phase endpoints:

```
delta = abs(target_volume - current_volume)
scale_factor = 360 / phase_duration
psg_channel_ramp_accum[ch] += delta * scale_factor

while accumulator >= 360:
    accumulator -= 360
    current_volume += ramp_direction  (+1 or -1)

output = min(current_volume, max_volume)
psg_write_register(8 + channel, output)
```

This avoids floating-point arithmetic while providing smooth 50 Hz volume ramping
across the 0–15 PSG amplitude range.

### Envelope Triggering

Envelopes are triggered from `midi_seq_dispatch_event()`:
- **Note-on**: loads envelope parameters from the song data's instrument definition
  block, sets `phase = ENV_ATTACK`
- **Note-off**: sets `phase = ENV_RELEASE` (begins fadeout)
- **Program change**: updates the envelope parameter source for subsequent notes

---

## 3. Sound Effect Engine

### Overview

A separate priority-based system plays ambient sound effects through the YM2149
using the Atari ST's built-in XBIOS `Dosound` command interpreter. Sound effects
and music are mutually exclusive — they share the PSG hardware.

### Data Source: `sounds.lcp`

The file contains 26 DoSound command sequences, loaded into `midi_note_length_params[]`
at startup. Each effect is a variable-length byte array in the Atari ST DoSound
format: a sequence of register-write commands that the OS executes at 50 Hz.

Each entry has a 2-byte size prefix followed by the DoSound command bytes and a
4-byte duration suffix.

### Sound Effect IDs (SOUND_EFFECT_ID enum, 26 values)

| ID | Name | Usage |
|---|---|---|
| 0 | `SFX_FOOTSTEP_CARPET` | Walking on carpeted floors |
| 1 | `SFX_FOOTSTEP_WOOD` | Walking on wooden floors |
| 2 | `SFX_FOOTSTEP_STAIRS` | Walking on stairs |
| 3 | `SFX_DOOR_OPEN` | Opening doors |
| 4 | `SFX_DOOR_CLOSE` | Closing doors |
| 5 | `SFX_DOORBELL` | Front doorbell ring |
| 6 | `SFX_DOORBELL_ECHO` | Doorbell follow-up echo |
| 7 | `SFX_WATER_TAP` | Turning on water (kitchen/bathroom) |
| 8 | `SFX_WATER_RUNNING` | Water running continuously |
| 9 | `SFX_TOILET_FLUSH` | Toilet flushing |
| 10 | `SFX_TOILET_REFILL` | Toilet tank refilling |
| 11 | `SFX_TYPEWRITER_KEY` | Typewriter key press |
| 12 | `SFX_TYPEWRITER_RETURN` | Typewriter carriage return |
| 13 | `SFX_PHONE_RING` | Phone ringing |
| 14 | `SFX_PHONE_PICKUP` | Picking up phone receiver |
| 15 | `SFX_SPEECH_MURMUR` | LCP speaking/mumbling |
| 16 | `SFX_SNORING` | Sleeping sound |
| 17 | `SFX_ALARM_CLOCK` | Alarm clock ringing |
| 18 | `SFX_TV_STATIC` | TV static noise |
| 19 | `SFX_FIRE_CRACKLE` | Fireplace crackling |
| 20 | `SFX_FOOD_CRUNCH` | Eating sounds |
| 21 | `SFX_EXERCISE_GRUNT` | Exercise/effort sounds |
| 22 | `SFX_APPLAUSE` | Applause (after performance) |
| 23 | `SFX_BOOK_PAGE_TURN` | Turning book pages |
| 24 | `SFX_DRINK_SIP` | Drinking water |
| 25 | `SFX_YAWN` | Yawning (when tired) |

Note: The exact enum values and their mappings are reconstructed from usage
context in action functions and the `SOUND_EFFECT_ID` enum in Ghidra.

### Priority System

Each sound effect has a priority value stored in `_soundeffect_priority_table[]`.
When a new effect is requested via `soundeffect_select()`:

- If no effect is currently playing: play immediately
- If a higher-priority effect is playing: ignore the new request
- If a lower-or-equal priority effect is playing: preempt with the new one

This prevents footstep sounds from interrupting doorbells, and prevents
multiple concurrent effects from producing cacophony.

### Playback Flow

```
soundeffect_select(SFX_ID, duration)
  1. Check priority against current playing effect
  2. Store effect ID and duration in globals
  3. Set soundeffect_active_flag = YES

soundeffect_irq_play() — called from screen_render_8hz at 8 Hz
  1. If music is playing: skip (SFX and music share PSG)
  2. If another SFX playing with higher priority: skip
  3. Silence current SFX via soundeffects_off()
  4. Copy DoSound command data from midi_note_length_params[SFX_ID]
     into soundeffect_DoSound_Buffer
  5. Call _xbios(XBIOS_Dosound, buffer) to start OS-driven playback
  6. Calculate duration in 200 Hz ticks from the 4-byte suffix
  7. If explicit duration provided: override with caller's value

screen_render_8hz() — handles SFX expiration
  if soundeffect_remaining_ticks > 0:
      soundeffect_remaining_ticks -= 1
      if expired:
          soundeffects_off()
          if was SFX_DOORBELL:  play SFX_DOORBELL_ECHO
          if was SFX_TOILET_FLUSH:  play SFX_TOILET_REFILL
```

### Chained Sound Effects

Some effects automatically trigger follow-up effects when they expire:
- `SFX_DOORBELL` -> `SFX_DOORBELL_ECHO` (the ring echoes)
- `SFX_TOILET_FLUSH` -> `SFX_TOILET_REFILL` (tank refills after flush)

This chaining is handled in `screen_render_8hz()` when the duration timer expires.

### Mutual Exclusion with Music

Sound effects and music cannot play simultaneously because they share the
YM2149 PSG hardware. `soundeffect_irq_play()` checks `midi_is_playing` first
and returns immediately if music is active. During record player and piano
actions, no ambient SFX are heard.

---

## File Formats

### .SNG / .ORG Song File Format

Both file types share the same internal format. `.sng` is used for background
music, `.org` for organ/piano music the LCP plays on the record player or piano.

```
Offset    Size    Content
0x0000    10      File header (0xCD + 'Mstudio' + 0xCD + 0x02, skipped by loader)
0x000A    15*10   Instrument names in ASCII
0x00A0    124     Instrument envelope data (PSG ADSR params)
0x011C    90      Channel/program mapping (30 entries x 3 bytes)
0x0176    ...     Header configuration events (tempo, scale, volume)
0x01E0    30      Title of the song in ASCII
0x01FE+   ...     MIDI event stream (compact bytecode, up to 20,000 bytes)
```

The loader reads the file into a single buffer, then sets `midi_data_base_ptr`
to buffer + 0x1FE (the start of the MIDI event stream). The channel map and
envelope data are accessed at negative offsets from this pointer:
- `midi_data_base_ptr - 90`: channel/program mapping (30 bytes)
- `midi_data_base_ptr - 450`: envelope ADSR parameters (360 bytes)

### Header Configuration Events

The header section (between channel map and MIDI stream) contains
configuration commands parsed by `midi_seq_parse_header()`:

| Command | Bytes | Purpose |
|---|---|---|
| 0x80 | 2 | Set MIDI channel count |
| 0x81 | 4 | Set tempo (`midi_tempo`, `midi_ticks_per_beat`) |
| 0x83 | 2 | Set default volume/velocity |
| 0x84 | 2 | Build scale/transpose table |
| 0xC0 | 3 | Program change event (channel assignment) |
| 0xFF | 1 | End of header |
| 0x00 | 1 | Padding/skip |
| 0x01-0x7F | 3 | Note events in header (skipped) |

### sounds.lcp File Format

Contains 26 sound effect entries stored sequentially. Each entry:

```
Offset  Size    Content
0       2       Size of DoSound command data (in bytes)
2       N       DoSound command bytes (Atari ST XBIOS format)
N+2     4       Default duration (2 shorts: high/low, in 200 Hz ticks)
```

The DoSound command format is the Atari ST native format: pairs of
(register_number, value) bytes written to the YM2149 at 50 Hz by the OS
interrupt handler, with special control codes for delays and loops.

---

## Footstep Sound Mapping

Footstep sounds play when the LCP walks, based on surface type:

| Floor | X Range | Sound Effect |
|---|---|---|
| 1 (bottom) | X < 166 | `SFX_FOOTSTEP_CARPET` |
| 1 (bottom) | X >= 166 | `SFX_FOOTSTEP_WOOD` |
| 2 (middle) | 146 < X < 234 | `SFX_FOOTSTEP_CARPET` |
| 2 (middle) | other | (silent) |
| 3 (top) | X > 136 | `SFX_FOOTSTEP_WOOD` |
| 3 (top) | X <= 136 | (silent) |
| Stairs | any | `SFX_FOOTSTEP_STAIRS` |

Footsteps trigger on walk animation frames 3 and 7 (two steps per 8-frame
walk cycle), controlled by `footstep_trigger_flag`.

---

## Function Reference

### MIDI Sequencer (20 functions)

| Address | Function | Purpose |
|---|---|---|
| 0x10028 | `midi_seq_init_song` | Initialize and start song playback |
| 0x10082 | `midi_seq_start_playback` | Reset timing, enable sequencer |
| 0x100B4 | `midi_seq_set_position` | Set playback position in event stream |
| 0x1012A | `midi_seq_skip_padding` | Skip leading 0x00/0xFF padding bytes |
| 0x1026A | `midi_seq_push_loop` | Push loop context onto stack |
| 0x10338 | `midi_seq_parse_events` | Parse next event(s) from stream |
| 0x105CA | `midi_seq_read_note_duration` | Read note duration from duration table |
| 0x10628 | `midi_seq_queue_note_event` | Queue note into event queue |
| 0x107B0 | `midi_seq_send_note_off` | Send note-off for a queued note |
| 0x10918 | `midi_seq_dispatch_event` | Route event to PSG/MIDI outputs (154 lines) |
| 0x10E88 | `midi_seq_expire_notes` | Release notes whose duration expired |
| 0x10EC2 | `midi_seq_advance_sequencer` | Main sequencer tick advance |
| 0x1103C | `midi_seq_stop` | Stop sequencer, silence all notes |
| 0x11184 | `midi_seq_reset_programs` | Reset all MIDI channel programs |
| 0x111FA | `midi_seq_parse_header` | Parse header configuration commands |
| 0x1135C | `midi_seq_parse_channel_map` | Parse 90-byte channel/program map |
| 0x113B4 | `midi_seq_build_scale_table` | Build scale/transpose lookup table |
| 0x11494 | `midi_out_write_byte` | Write single byte to MIDI ACIA |
| 0x114BC | `psg_write_register` | Write value to YM2149 register |
| 0x1219A | `midi_seq_tick_handler` | 200 Hz MFP Timer A interrupt handler |

### PSG Envelope (3 functions)

| Address | Function | Purpose |
|---|---|---|
| 0x115AE | `psg_process_envelopes` | Software ADSR envelope processor (50 Hz) |
| 0x11A0E | `psg_set_note_frequency` | Set PSG channel period from note number |
| 0x119C6 | `psg_set_mixer_and_volume` | Configure mixer and volume registers |

### Sound Effects (5 functions)

| Address | Function | Purpose |
|---|---|---|
| 0x1DA0A | `soundeffect_select` | Queue a sound effect by ID and duration |
| 0x1DAFC | `soundeffect_irq_play` | Play queued effect via XBIOS Dosound |
| 0x1DC3C | `soundeffects_off` | Silence all PSG channels, clear SFX state |
| 0x14FEC | `lcp_play_footstep_sound` | Surface-dependent footstep SFX selection |
| 0x1D904 | `play_doorbell_sound` | Queue doorbell ring effect |

### Song Playback (2 functions)

| Address | Function | Purpose |
|---|---|---|
| 0x11D00 | `song_play` | Load .sng/.org file and start playback |
| 0x11462 | `song_active` | Check if a song is currently playing |

### Key Global Variables

| Variable | Type | Purpose |
|---|---|---|
| `midi_is_playing` | bool | True when sequencer is active |
| `midi_sequencer_active` | bool | True when timer interrupt drives sequencer |
| `midi_data_base_ptr` | uint8_t* | Pointer to start of MIDI event stream |
| `midi_seq_position` | uint8_t* | Current read position in event stream |
| `midi_seq_phase` | MIDI_SEQ_PHASE | Current sequencer state |
| `midi_tempo` | short | Ticks per beat (controls playback speed) |
| `midi_tick_counter` | short | Raw 200 Hz tick counter |
| `midi_tick_prescaler` | short | Tempo-scaled tick subdivider |
| `midi_output_enabled` | BOOL16 | Enable external MIDI output |
| `psg_output_enabled` | BOOL16 | Enable internal PSG synthesis |
| `midi_song_loop_flag` | bool | True = loop song, false = play once |
| `midi_song_buffer` | uint8_t* | Heap buffer for current song data |
| `midi_note_event_queue[]` | short[] | Note queue (58 entries x 3 shorts) |
| `midi_scale_transpose_table[]` | uint8_t[] | 132-entry scale quantization table |
| `midi_channel_map[]` | short[] | MIDI channel to PSG channel mapping |
| `psg_envelope[3]` | PSG_ENVELOPE | Software ADSR state per PSG channel |
| `psg_channel_notes[3]` | byte | Currently sounding note per PSG channel |
| `psg_frequency_table[]` | short[] | Note-to-PSG-period lookup table |
| `soundeffect_current` | SOUND_EFFECT_ID | Currently queued effect |
| `soundeffect_playing_id` | SOUND_EFFECT_ID | Currently playing effect |
| `soundeffect_playing_flag` | BOOL16 | True when a DoSound effect is active |
| `soundeffect_remaining_ticks` | long | Frames remaining for current effect |
| `_soundeffect_priority_table[]` | short[] | Priority value per effect ID |
