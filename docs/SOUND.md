# Little Computer People — Sound

How the game makes sound: the music player, which plays the resident's
records and organ pieces, and the sound effects.  What triggers them is in
[PEOPLE.md](PEOPLE.md); the song file format in [SNG_FORMAT.md](SNG_FORMAT.md);
how the player relates to Activision's *The Music Studio* in
[MUSIC_PLAYER_COMPARISON.md](MUSIC_PLAYER_COMPARISON.md).

Names are those of the C port in `source/`.  Ghidra addresses (base 0x10000)
are given in the function reference.

## The sound chip

The ST's Yamaha YM2149 has three square-wave tone channels (A, B, C), a noise
generator and a hardware envelope generator, which the game does not use.

| Register | Function |
|---|---|
| 0–5 | tone period of channels A, B, C (fine, coarse) |
| 6 | noise period |
| 7 | mixer: tone and noise enables per channel |
| 8–10 | volume of channels A, B, C |
| 11–13 | hardware envelope (unused) |

Music and sound effects share it, and **music wins**: while a song plays,
`startSfx` starts no effect at all.  The player can also drive an external
MIDI instrument through the ST's MIDI port.

## The music player

The music player descends from the one in Activision's *The Music Studio*,
whose files the game plays unchanged.  It lives in
[`midi_seq.c`](../source/midi_seq.c), with its interrupt in `mq_tick.s` and
its hardware pokes in `psg_asm.s`.

### Who plays music

- **Records** (`playRecord`): one of the first n `.SNG` files on the disk,
  n being the size of his record collection (4 at the start, plus one per
  record delivery).  With more records than files, the extra picks all play
  the last file found.  `danceToMusic` starts a record if none is playing;
  `stopRecord` stops it.
- **Organ** (`playOrgan`): a random `.ORG` file.  While it plays, the
  resident moves his hands whenever a channel's volume rises.

Both go through `playSongFile`, which stops a song that is still playing,
frees the old song buffer, allocates one the size of the new file, skips the
10-byte Music Studio signature, reads up to 20 000 bytes and calls
`startSong`.

### Starting a song

`startSong` points `songEvents` 0x1FE bytes into the buffer, where the event
stream starts, and then:

1. `parseSongHeader` reads the header commands, after `unpackChanMap` has
   read the 30-byte channel map 90 bytes before the stream (MIDI channel and
   program for logical channels 1..15);
2. `resetPrograms` sends a program change for every channel in use;
3. `initSongState` sets the read position, the end of the song, the default
   velocity and volume, and empties the note queue and the loop stack;
4. `armSequencer` seeds the timer counters (100 ticks of grace before the
   first event) and `songPlaying` is set.

Header commands (`MIDI_HDR_*`):

| Command | Effect |
|---|---|
| `0x80 xx kk` | key: `buildNoteMap(kk)` rebuilds the note translation table (sharps or flats per scale degree) |
| `0x81 tt` | tempo: `ticksPerBeat = 2400 / tt` |
| `0x83 xx` | volume: skipped |
| `0x84 xx vv` | default velocity `vv`, and from it the default PSG volume, 5..15 |
| `0xC0 xx xx` | program change: skipped here (`resetPrograms` handles programs) |
| `0xFF` | end of the header |
| others below 0x20 (masked with 0x9F) | 3-byte note events, skipped |

### The clock

The MFP's Timer A, installed by `hookTimerA`, interrupts 960 times a second
(2.4576 MHz / 64 / 40).  The handler, `timerAIsr` in `mq_tick.s`:

- counts `timerTicks`;
- every fourth tick (`envDivider`) steps the PSG envelopes, `stepEnvelopes`
  -- 240 Hz -- while a song plays or a PSG note is still sounding;
- while a song plays, counts `seqCountdown` down and steps the sequencer,
  `seqAdvance`, when it reaches zero.

Both run at interrupt level 5 so that the keyboard and MIDI interrupts are
still served, each guarded by a busy flag (`envBusy`, `seqBusy`) so a slow
step is never re-entered.

### The sequencer

`seqAdvance` is a three-state machine (`seqPhase`, `SEQ_PHASE_*`):

- **wait** (`WAIT_NOTE_EXPIRE`): age the queued notes by the time elapsed
  (`expireNotes`), sending note-offs for those that have run out;
- **parse** (`PARSE_NEXT_EVENT`): read the next batch of events
  (`parseEvents`) and load the time to the next one;
- **ending** (`SONG_ENDING`): keep expiring notes until the queue is empty,
  then silence the PSG and clear `songPlaying`.

The event stream (`parseEvents`):

| Bytes | Event |
|---|---|
| `0x00` | end of a time step |
| `0x01`..`0x7F` + 2 | a note: logical channel and note-on/sustain/note-off flags, a duration index (with accent and transpose bits), the MIDI note |
| `0x82` | bar marker |
| `0x85 n` | loop start, repeat n times (`pushLoop`) |
| `0x86` | loop end (`popLoop`) |
| `0xFF` | end of the song |

A note is translated through `noteMap` (the key), queued in `noteQueue` with
its duration (`queueNote`, up to 20 notes) and sent as a note-on by
`sendMidiEvent`; when its time is up, `sendNoteOff` releases it.

### Output: the PSG and MIDI

`sendMidiEvent` sends every event to both outputs, each switchable
(`midiOutOn`, `psgOutOn`):

- **MIDI out:** the note is shifted by octaves according to the channel map
  and sent through the ACIA -- directly with `aciaWrite` while running in
  the interrupt (`seqBusy`), otherwise with XBIOS `Midiws`.  Program
  changes are sent once per physical channel (`sendProgChange`).
- **PSG:** a note-on takes a silent channel, or steals the one furthest
  through its envelope; the instrument's 8-byte envelope is copied from the
  song (`copyEnvelope`), the tone period comes from `psgPeriod`, and the
  mixer and noise are set.  A note-off finds the channel playing that note
  and starts its release.  `noteOwner` records which notes are sounding.

### PSG envelopes

The volume of each PSG channel is shaped in software by `stepEnvelopes`,
240 times a second.  Each channel's `PSG_ENVELOPE`
([`include/structs.h`](../source/include/structs.h)) runs through attack,
decay, sustain, release and fade-out (`ENV_*`): every step adds a rate from
`envRateTab` to an accumulator (`rampAccum`) and moves the volume one step
each time it passes 360 -- integer interpolation without division.  The volume
is capped at the note's maximum and written to registers 8..10 with
`psgWrite`.

## Sound effects

### The effects

`SOUNDS.LCP` holds 23 effects, XBIOS `Dosound` scripts loaded at start-up by
`loadSounds` ([`sound.c`](../source/sound.c)) into `sfxData`.

| Id | Effect (`SFX_*`) | Played by |
|---|---|---|
| 0..2 | `FOOTSTEP_STAIRS`, `_CARPET`, `_WOOD` | the resident's steps (`playFootstep`) |
| 3..5 | `FOOTSTEP_3`..`5` | nothing |
| 6 | `TV_CLICK` | switching the TV on; one of his chatter sounds |
| 7, 9 | `SPEECH`, `GREETING` | his chatter: on the phone, waving hello |
| 8 | `HEAD_NOD` | his chatter on the phone and when waving |
| 10 | `CLICK` | typing on the computer and the typewriter |
| 11 | `TYPEWRITER_KEY` | the typewriter's carriage return |
| 12, 13 | `DOORBELL`, `DOORBELL_ECHO` | deliveries; the echo follows the bell |
| 14, 15 | `DOOR_OPEN`, `DOOR_CLOSE` | doors, cabinets, the closet, the fridge |
| 16, 17 | `TOILET_FLUSH`, `TOILET_REFILL` | the toilet; the refill follows the flush |
| 18 | `WATER_RUNNING` | washing at a sink |
| 19 | `WATER_TAP` | Ctrl-W, adding water to the tank |
| 20 | `ALARM_CLOCK` | the alarm, repeated while it rings |
| 21 | `PHONE_RING` | the phone, repeated while it rings |
| 22 | `SNORING` | dozing off |

### Priority

Each effect has a priority in `sfxPriority`; **lower is more important**:
doorbell 0, phone 1, alarm 16, footsteps 30, most others 15.

- `sfxSelect(id, duration)` stores a request (`sfxReqId`, `sfxReqDur`); a
  pending request is replaced only by one at least as important.
- `startSfx` ([`sfx_irq.c`](../source/sfx_irq.c)), run by the compositor
  once per frame while a request is pending, starts it -- unless a song is
  playing, or an effect is playing that is more important; an equal or more
  important request cuts it off (`stopSfx`).

### Playing one

`startSfx` copies the effect into `sfxBuffer`, replaces its last four bytes
-- the effect's duration -- with zeros, and hands it to XBIOS `Dosound`,
which plays it from TOS's own 50 Hz interrupt.  The duration, in 200 Hz units,
is divided by 25 into frames (`sfxTicksLeft`); a caller can give its own
duration instead, and -1 keeps the effect's.

The compositor counts `sfxTicksLeft` down every frame and silences the PSG
when it reaches zero (`stopSfx`).  Two effects then chain: the doorbell is
followed by its echo, the toilet flush by the refill.

`sfxBuffer` is 56 bytes, and three effects are longer -- the head nod and
the toilet refill at 148 bytes, the water tap at 60 -- so copying them runs
past its end.  In the shipped game this lands in unused memory; see
`docs/history.md` ("Is the 56 real?").

### Footsteps

`playFootstep` picks the sound by where he walks:

| Where | Sound |
|---|---|
| stairs | `SFX_FOOTSTEP_STAIRS` |
| ground floor, x < 166 (kitchen) | `SFX_FOOTSTEP_CARPET` |
| ground floor, x >= 166 (living room) | `SFX_FOOTSTEP_WOOD` |
| middle floor, 146 < x < 234 (bathroom) | `SFX_FOOTSTEP_CARPET` |
| top floor, x > 136 | `SFX_FOOTSTEP_WOOD` |
| elsewhere | silent |

The three are the same effect except for the noise period (register 6):
stairs 17, carpet 1, wood 7.

## File formats

The song files are documented in [SNG_FORMAT.md](SNG_FORMAT.md).

### SOUNDS.LCP

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
  `(hi << 16) | lo`, a time in 200 Hz units; divided by 25 it gives frames

`startSfx` copies the entire S-byte content into `sfxBuffer`,
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


## Function reference

| Address | Function | Purpose |
|---|---|---|
| 0x1016A | `startSong` | start a song in a buffer |
| 0x101BC | `initSongState` | read position, end, defaults |
| 0x10224 | `armSequencer` | seed the counters and start the sequencer |
| 0x1012A | `skipTextField` | skip a text field in the song data |
| 0x1026A, 0x102B6 | `pushLoop`, `popLoop` | the loop stack |
| 0x10338 | `parseEvents` | read the next events |
| 0x105CE | `peekNoteDur` | look ahead at the next note's duration |
| 0x10628 | `queueNote` | queue a note and send its note-on |
| 0x107B0 | `sendNoteOff` | release a queued note |
| 0x1084A | `sendProgChange` | program change to MIDI |
| 0x10918 | `sendMidiEvent` | one event to MIDI out and the PSG |
| 0x10E02, 0x10E64 | `expireNotes`, `removeQueued` | age and drop queued notes |
| 0x10EC2 | `seqAdvance` | the sequencer step |
| 0x1103C | `stopSequencer` | stop the sequencer (never called) |
| 0x11112, 0x11162 | `hookTimerA`, `unhookTimerA` | install and remove the Timer-A interrupt (the second is never called) |
| 0x11184 | `resetPrograms` | program changes for all channels |
| 0x111FA | `parseSongHeader` | the header commands |
| 0x1135C | `unpackChanMap` | the channel and program map |
| 0x113B4 | `buildNoteMap` | the note translation table for a key |
| 0x11586 | `copyEnvelope` | copy an instrument envelope |
| 0x115AE | `stepEnvelopes` | the PSG envelopes |
| 0x1219A | `timerAIsr` | the Timer-A interrupt (`mq_tick.s`) |
| 0x12272, 0x12284 | `psgWrite`, `psgMixer` | write a PSG register, update the mixer (`psg_asm.s`) |
| 0x122A6 | `aciaWrite` | send a byte to MIDI out (`psg_asm.s`) |
| 0x1D9EA | `playSongFile` | load a song file and start it |
| 0x1DAFC | `startSfx` | start the requested effect |
| 0x1DCC4 | `loadSounds` | load `SOUNDS.LCP` |
| 0x1DD88 | `sfxSelect` | request an effect |
| 0x1DDD8 | `stopSfx` | silence the PSG |
| 0x14FEC | `playFootstep` | footsteps |
| 0x1F904..0x1F952 | `sfxTvClick`, `sfxSpeech`, `sfxHeadNod`, `sfxGreeting` | chatter |
| 0x24786, 0x2476C | `sfxClick`, `typeKeySound` | typing |
| 0x25F9A | `playDoorbell` | the doorbell |

## Song catalogue

The disk holds 16 pieces, mostly by **Ed Bogas**: 11 `.SNG` files, the records
the resident plays on his record player, and 5 `.ORG` files, the pieces he
plays on the organ.

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
