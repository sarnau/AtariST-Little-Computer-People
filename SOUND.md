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
| `sounds.lcp` | DoSound sequences | 23 sound effects (footsteps, doors, bells, etc.) |
| `*.sng` | Custom MIDI + PSG envelope | Background music songs |
| `*.org` | Same format as .sng | Organ/piano music for record player and piano actions |

---

## 1. MIDI Sequencer Engine

### Overview

A custom MIDI-like sequencer engine (`midi_seq_*` functions, 24 total) plays
`.sng` song files. It supports dual output: external MIDI via the Atari ST's
ACIA port, and internal PSG synthesis via direct YM2149 register writes. Both
outputs can operate simultaneously or independently, controlled by
`g_moen` and `psg_out` flags.

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

The `g_mtpre` controls the effective tempo. It counts down from
`mi_temp` and triggers sequencer advancement when it reaches zero. The
`g_mtdiv` provides finer event-level timing within each prescaler
period.

A re-entrancy lock (`mi_rlock`) prevents the sequencer from being
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

### Scale/Transpose System (`mq_bust`)

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

### Event Dispatch (`mq_dise`)

The largest sound function (154 lines). Routes MIDI events to both outputs:

**External MIDI** (`g_moen`):
- Applies octave transposition based on channel mapping
- Sends via `_xbios(XBIOS_Midiws)` or direct ACIA byte writes (`mowrit`)

**Internal PSG** (`psg_out`):
- Note-on (status 0x90): looks up frequency in `psg_frequency_table[]`, writes to PSG
  period registers, triggers ADSR envelope
- Note-off (status 0x90 with velocity 0): finds matching channel, triggers envelope release
- Program change (status 0xC0): loads PSG envelope parameters from the song data
- Allocates notes to the 3 available PSG channels, tracking active notes in
  `psg_channel_notes[]`

### Channel Mapping (`mq_pacm`)

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

When the LCP plays the record player (`a_plawr`) or piano
(`a_playp`), the game:

1. Scans the data directory for `*.sng` or `*.org` files using GEMDOS Fsfirst/Fsnext
2. Selects a random file from those found
3. Calls `song_play()` to start playback
4. Animates the LCP based on real-time PSG volume levels:
   - Reads PSG registers 8, 9, 10 (channel A/B/C amplitude) via `XBIOS Giaccess`
   - If any channel exceeds the previous frame's volume, switch to an active
     dance/playing pose
   - Otherwise switch to idle pose
5. Loops until `mi_play` becomes false (song ends)

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

The file is 1,156 bytes and contains 23 DoSound command sequences terminated
by a 2-byte `0x0000` sentinel, loaded into `mi_ntLp[]` at
startup. Each effect is a variable-length byte array in the Atari ST DoSound
format: a sequence of register-write commands that the OS executes at 50 Hz.

Each entry consists of a 2-byte big-endian size word followed by that many
bytes of content. The content contains the DoSound command bytes (ending with
an `0xFF` terminator), optional padding zeros, and a 4-byte duration suffix
(two big-endian shorts). The game zeroes out the duration bytes after
extracting them, since the DoSound interpreter would try to execute them
as register writes.

The 23 effects fall into three complexity tiers based on DoSound data size:

- **Simple (34 bytes, 13 effects)**: All 14 YM2149 registers set once + `0xFF`.
  Single-shot sounds that decay via hardware envelope. Used for footsteps,
  doors, typewriter, phone, food crunch, applause.
- **Looping (52 bytes, 5 effects)**: Base registers + 3 pitch-sweep stages
  using DoSound's `0x80+reg` loop-target mechanism. Used for doorbell echo,
  water tap, toilet flush, snoring.
- **Complex (60–148 bytes, 5 effects)**: Multi-stage sequences with many
  sweep steps. Water running (148 bytes, 15 pitch steps), alarm clock
  (148 bytes, cycles through all 3 channels), fire crackle variant (60 bytes,
  4 sweep stages).

### Sound Effect IDs (SOUND_EFFECT_ID enum, 23 values)

| ID | Name | Size | Usage |
|---|---|---|---|
| 0 | `SFX_FOOTSTEP_STAIRS` | 34 | Walking on stairs |
| 1 | `SFX_FOOTSTEP_CARPET` | 34 | Walking on carpet |
| 2 | `SFX_FOOTSTEP_WOOD` | 34 | Walking on wood floors |
| 3 | `SFX_FOOTSTEP_3` | 34 | Footstep variant (not referenced in code) |
| 4 | `SFX_FOOTSTEP_4` | 34 | Footstep variant (not referenced in code) |
| 5 | `SFX_FOOTSTEP_5` | 34 | Footstep variant (not referenced in code) |
| 6 | `SFX_TV_CLICK` | 52 | TV on/off click |
| 7 | `SFX_SPEECH` | 52 | LCP speaking/mumbling |
| 8 | `SFX_HEAD_NOD` | 148 | LCP head nod acknowledgment |
| 9 | `SFX_GREETING` | 52 | LCP greeting/wave |
| 10 | `SFX_CLICK` | 34 | Random UI click |
| 11 | `SFX_TYPEWRITER_KEY` | 34 | Typewriter key press |
| 12 | `SFX_DOORBELL` | 34 | Front doorbell ring |
| 13 | `SFX_DOORBELL_ECHO` | 34 | Doorbell follow-up echo (chained from 12) |
| 14 | `SFX_DOOR_OPEN` | 34 | Opening doors, cabinets, fridge, closets |
| 15 | `SFX_DOOR_CLOSE` | 34 | Closing doors, cabinets, closets |
| 16 | `SFX_TOILET_FLUSH` | 52 | Toilet flushing |
| 17 | `SFX_TOILET_REFILL` | 148 | Toilet tank refilling (chained from 16) |
| 18 | `SFX_WATER_RUNNING` | 34 | Water running (drinking, washing hands) |
| 19 | `SFX_WATER_TAP` | 60 | Water tap on |
| 20 | `SFX_ALARM_CLOCK` | 34 | Alarm clock ringing |
| 21 | `SFX_PHONE_RING` | 34 | Phone ringing |
| 22 | `SFX_SNORING` | 34 | Snoring (sleeping) |

IDs 3–5 (`SFX_FOOTSTEP_3/4/5`) have valid DoSound data in SOUNDS.LCP but
are never referenced by any `soundeffect_select()` call in the game code.
They may be unused variants or reserved for future use.

The three active footstep effects (IDs 0–2) are identical except for the
noise period register (R06): stairs=17, carpet=1, wood=7. The same `\_`
decay envelope shape creates distinct surface textures through noise
frequency alone.

### Priority System

Each sound effect has a priority value stored in `sf_pri[]`.
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
YM2149 PSG hardware. `soundeffect_irq_play()` checks `mi_play` first
and returns immediately if music is active. During record player and piano
actions, no ambient SFX are heard.

---

## File Formats

### .SNG / .ORG Song File Format ("The Music Studio" by Activision)

Both file types share the same internal format, created by **The Music Studio**
(Activision, 1986) for the Atari ST. The original music was composed by
**Ed Bogas** (Activision staff composer). The `.sng` extension is used for
background music songs, while `.org` is used for organ/piano pieces that the
LCP plays on the record player or piano.

The file signature is `0xCD "Mstudio" 0xCD`, format version `0x02`.

```
Offset  Size  Content
------  ----  --------------------------------------------------
0x000     1   Sentinel: 0xCD
0x001     7   ASCII signature: "Mstudio"
0x008     1   Sentinel: 0xCD
0x009     1   Format version: 0x02

0x00A   150   Instrument names block 1 (15 x 10 bytes, null-padded ASCII)
              Active instrument set used for PSG synthesis.
              Names like: Blocks, Harmonica, Guitar, Flute, Clarinet,
              Baritone, Hihat, Snare, B.Fiddle, Sax, Piano, Bass, Vibes, Bells

0x0A0   120   Instrument envelope parameters (15 x 8 bytes)
              Per-instrument PSG envelope shape definitions.
              Byte 0: mixer/config flags (high nibble: tone/noise enable bits)
              Bytes 1-7: ADSR-like parameters loaded into PSG_ENVELOPE
              when a program change selects this instrument.

0x118   150   Instrument names block 2 (15 x 10 bytes)
              Alternate instrument bank. May be identical to block 1
              (Ed Bogas originals) or contain a different set
              (classical pieces use different instruments like Accordian,
              Soprano, Congas, Trumphet).

0x1AE    30   Channel/program map (15 x 2 bytes)
              Parsed by midi_seq_parse_channel_map() at
              midi_data_base_ptr - 90.
              Each 2-byte entry configures instrument-to-channel routing.
              Value 0x01 = default/identity mapping.

0x1CC    20   Extended data (usually zeros)
              Some files contain 4 x 4-byte section repeat markers.

0x1E0    32   Song name (null-terminated ASCII, padded to 32 bytes)

0x200     8   Pre-stream area
              Usually: FF FF 00 00 00 00 00 00
              (FF FF serves as skip-padding target for midi_seq_skip_padding)

0x208     -   midi_data_base_ptr target (= buffer + 0x1FE in game engine)
              Header configuration commands followed by note event stream.
```

The game's `song_play()` function skips the 10-byte file header, reads up to
20,000 bytes into a heap buffer, then sets `midi_data_base_ptr = buffer + 0x1FE`
(file offset 0x208). The channel map and envelope data are accessed at
negative offsets: `midi_data_base_ptr - 90` for the channel map (file 0x1AE),
and earlier offsets for envelope parameters.

### Header Configuration Commands

Parsed by `midi_seq_parse_header()` starting at `mi_dbase`.
The parser uses the mask `(byte & 0x9F) < 0x20` to distinguish note events
(3-byte groups, skipped) from configuration commands (dispatched via jump table).
The header ends when a 0x00 byte is encountered after the initial skip.

| Command | Size | Purpose |
|---|---|---|
| 0x80 NN | 2 | Set MIDI channel count |
| 0x81 NN | 2 | Set tempo: `midi_ticks_per_beat = 2400 / NN` |
| 0x83 NN | 2 | Set default volume/velocity |
| 0x84 NN | 2 | Set scale (1=chromatic passthrough) |
| 0x85 | 1+ | Loop start marker |
| 0xC0 CC PP | 3 | Program change (channel, program) |
| 0x00 | 1 | End of header (return) |
| 0xFF | 1 | End marker (return) |
| 0x01-0x7F | 3 | Note events in header (skipped) |

Typical header: `CHANNELS=1, TEMPO=128, SCALE=1` (all songs use SCALE=1).
Tempo values range from 78 (Bossa Nova, slowest) to 171 (Five Four, fastest).

### Note Event Encoding (3 bytes)

```
Byte 0 (voice/instrument):
  Bits 0-3: instrument index (0-14, into instrument block 1)
  Bits 4-7: voice channel / modifier flags
    0x0N = primary voice
    0x1N = secondary voice
    0x2N = third voice
    0x4N = accent / note-on emphasis
    0x8N = special modifier

Byte 1 (duration + flags):
  Bits 0-4: duration index (into midi_note_duration_table[], 0-25)
  Bit 5:    accent flag (force max velocity 0x7F)
  Bits 6-7: transpose mode (0=use scale table, 1-3=raw/bypass)

Byte 2 (pitch):
  Bits 0-6: MIDI note number (0-127)
  Bit 7:    unused
```

Between note events: 0x00 = time advance/rest, 0x82 = bar marker, 0xFF = end of song.

### sounds.lcp File Format

Contains 23 sound effect entries stored sequentially, terminated by a 2-byte
`0x0000` sentinel. Total file size: 1,156 bytes. Each entry:

```
Offset  Size    Content
0       2       Entry size S (big-endian short, byte count of content below)
2       S       Content: DoSound commands + 0xFF + padding + 4-byte duration
```

Within the S-byte content region:
- **DoSound commands**: pairs of (register_number, value) for YM2149 registers
  0–13, with special loop-control codes (`0x80+reg`, `0x81`, `0x82`)
- **`0xFF`**: end-of-sequence terminator
- **Padding**: zero bytes (0–1 byte, for alignment)
- **Duration**: last 4 bytes — two big-endian shorts (hi, lo) combined as
  `(hi << 16) | lo`, then divided by 25 to yield 200 Hz tick count

The game copies the entire S-byte content into `g_sfDoB`,
extracts the 4-byte duration from the end, then zeroes those 4 bytes so the
DoSound interpreter (XBIOS `Dosound`) won't try to execute them as commands.

The DoSound command format is the Atari ST native format: pairs of
(register_number, value) bytes written to the YM2149 at 50 Hz by the OS
interrupt handler. Special control codes create pitch sweeps:
- `0x80+N, val`: write `val` to register N and mark this as the loop target
- `0x81, count`: set internal counter to `count`
- `0x82, step`: subtract `step` from counter; if counter > 0, jump back
  to the last `0x80+N` command (creating a timed loop)

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
| 0x10028 | `mq_inis` | Initialize and start song playback |
| 0x10082 | `mq_stap` | Reset timing, enable sequencer |
| 0x100B4 | `mq_setp` | Set playback position in event stream |
| 0x1012A | `mq_skip` | Skip leading 0x00/0xFF padding bytes |
| 0x1026A | `midi_seq_push_loop` | Push loop context onto stack |
| 0x10338 | `midi_seq_parse_events` | Parse next event(s) from stream |
| 0x105CA | `midi_seq_read_note_duration` | Read note duration from duration table |
| 0x10628 | `midi_seq_queue_note_event` | Queue note into event queue |
| 0x107B0 | `midi_seq_send_note_off` | Send note-off for a queued note |
| 0x10918 | `mq_dise` | Route event to PSG/MIDI outputs (154 lines) |
| 0x10E88 | `midi_seq_expire_notes` | Release notes whose duration expired |
| 0x10EC2 | `midi_seq_advance_sequencer` | Main sequencer tick advance |
| 0x1103C | `midi_seq_stop` | Stop sequencer, silence all notes |
| 0x11184 | `mq_resp` | Reset all MIDI channel programs |
| 0x111FA | `mq_parh` | Parse header configuration commands |
| 0x1135C | `mq_pacm` | Parse 90-byte channel/program map |
| 0x113B4 | `mq_bust` | Build scale/transpose lookup table |
| 0x11494 | `mowrit` | Write single byte to MIDI ACIA |
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
| 0x1DA0A | `sf_sele` | Queue a sound effect by ID and duration |
| 0x1DAFC | `sf_irqp` | Play queued effect via XBIOS Dosound |
| 0x1DC3C | `sf_so` | Silence all PSG channels, clear SFX state |
| 0x14FEC | `lcp_play_footstep_sound` | Surface-dependent footstep SFX selection |
| 0x1D904 | `p_dobls` | Queue doorbell ring effect |

### Song Playback (2 functions)

| Address | Function | Purpose |
|---|---|---|
| 0x11D00 | `song_play` | Load .sng/.org file and start playback |
| 0x11462 | `song_active` | Check if a song is currently playing |

### Key Global Variables

| Variable | Type | Purpose |
|---|---|---|
| `mi_play` | bool | True when sequencer is active |
| `g_msmsa` | bool | True when timer interrupt drives sequencer |
| `mi_dbase` | uint8_t* | Pointer to start of MIDI event stream |
| `mi_sqpos` | uint8_t* | Current read position in event stream |
| `g_mspha` | MIDI_SEQ_PHASE | Current sequencer state |
| `mi_temp` | short | Ticks per beat (controls playback speed) |
| `g_mtcou` | short | Raw 200 Hz tick counter |
| `g_mtpre` | short | Tempo-scaled tick subdivider |
| `g_moen` | BOOL16 | Enable external MIDI output |
| `psg_out` | BOOL16 | Enable internal PSG synthesis |
| `mi_slop` | bool | True = loop song, false = play once |
| `mi_sbuf` | uint8_t* | Heap buffer for current song data |
| `midi_note_event_queue[]` | short[] | Note queue (58 entries x 3 shorts) |
| `midi_scale_transpose_table[]` | uint8_t[] | 132-entry scale quantization table |
| `midi_channel_map[]` | short[] | MIDI channel to PSG channel mapping |
| `psg_envelope[3]` | PSG_ENVELOPE | Software ADSR state per PSG channel |
| `psg_channel_notes[3]` | byte | Currently sounding note per PSG channel |
| `psg_frequency_table[]` | short[] | Note-to-PSG-period lookup table |
| `g_sfcur` | SOUND_EFFECT_ID | Currently queued effect |
| `g_sfpli` | SOUND_EFFECT_ID | Currently playing effect |
| `g_sfplf` | BOOL16 | True when a DoSound effect is active |
| `g_sfret` | long | Frames remaining for current effect |
| `sf_pri[]` | short[] | Priority value per effect ID |

---

## Song Catalog

The game ships with 16 songs: 10 original compositions by **Ed Bogas** (`.sng`),
and 6 classical/traditional arrangements (`.org`). The `.sng` files play as
background music; the `.org` files play when the LCP uses the record player or piano.

| File | Tempo | Events | Voices | Range | Title |
|---|---|---|---|---|---|
| AISLEDAN.SNG | 150 | 818 | 5 | C2–D6 | Aisle Dance by Ed Bogas |
| BALLAD.SNG | 141 | 536 | 10 | C-1–C6 | Ballad by Ed Bogas |
| BEBOP.SNG | 138 | 723 | 7 | C3–E6 | Bebop by Ed Bogas |
| BOOGIE.SNG | 160 | 544 | 3 | F2–F6 | Boogie by Ed Bogas |
| BOSSA.SNG | 78 | 624 | 4 | A2–D6 | Bossa Nova by Ed Bogas |
| CALYPSO.SNG | 133 | 602 | 5 | G2–C6 | Calypso by Ed Bogas |
| CANON.SNG | 160 | 676 | 4 | D2–E6 | Pachelbel's Canon in D |
| COUNTRY2.SNG | 130 | 750 | 8 | C-1–D6 | Country Too by Ed Bogas |
| FIVEFOUR.SNG | 171 | 671 | 8 | C-1–D6 | Five Four by Ed Bogas |
| MYSTERY.SNG | 104 | 387 | 5 | C-1–D6 | Mystery by Ed Bogas |
| TANGO.SNG | 133 | 842 | 6 | C2–F6 | Tango by Ed Bogas |
| FOLKSONG.ORG | 100 | 550 | 3 | C-1–C6 | Folk Song by Ed Bogas |
| MAPLE.ORG | 109 | 3,715 | 2 | C2–C7 | Maple Leaf Rag by Scott Joplin |
| PRELUDE.ORG | 120 | 601 | 2 | B2–C6 | Prelude |
| REQUIEM.ORG | 128 | 1,227 | 1 | D2–C6 | Kyrie eleison – Mozart's Requiem |
| STARSPAN.ORG | 133 | 396 | 8 | C2–E6 | Star-Spangled Banner / F.S. Key |
