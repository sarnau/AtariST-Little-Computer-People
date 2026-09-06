# The Music Studio `.SNG` / `.ORG` / `.SND` file format

Activision *The Music Studio* (Atari ST, `AUDIO.PRG`, 1985-11-20) and the sequencer it
shares with *Little Computer People* (see [LCP_ENGINE_COMPARISON.md](LCP_ENGINE_COMPARISON.md)).

Everything here comes from the playback code itself — `mq_parh`, `mq_pacm`, `mq_skip`,
`mq_setp`, `mq_pars`, `mq_rdur`, `mq_qnne`, `mq_snof`, `mq_sepc`, `mq_bust`, `mq_dise`,
`psg_cpE`, `psg_upE`, `mq_tick` — which is byte-identical or near-identical in both
programs. Decoder: [`../source/tools/sngdump.py`](../source/tools/sngdump.py).

All offsets are **absolute file offsets**. Multi-byte values are big-endian.

---

## 1. Container

| offset | size | field |
|---|---|---|
| `0x000` | 9 | signature `CD 4D 73 74 75 64 69 6F CD` = `\xCD` `"Mstudio"` `\xCD` |
| `0x009` | 1 | **file type**: `01` = instrument set (`.SND`), `02` = song (`.SNG`, `.ORG`) |
| `0x00A` | 150 | instrument name table A — 15 × 10-byte NUL-padded names |
| `0x0A0` | 120 | 15 × 8-byte ADSR envelope records, one per voice |
| `0x118` | 150 | instrument name table B — 15 × 10-byte names |
| `0x1AE` | 15 | channel map, one byte per voice |
| `0x1BD` | 15 | program map, one byte per voice |
| `0x1CC` | 20 | five longwords — lyric pointers / editor scratch (§8) |
| `0x1E0` | 32 | song title, NUL-padded |
| `0x200` | 8 | four 16-bit editor fields, `FFFF 0000 0000 0000` in almost every file (§9) |
| `0x208` | … | event stream — this is `mi_dbase` |
| after the `FF` end marker | … | optional lyric text (§8) |

A `.SND` instrument set stops after the program map: `STANDARD.SND` on this disk is
exactly `0x1CC` = 460 bytes.

### How the player finds all this

`sgPlay` strips the 10-byte signature into a buffer, then `mq_inis` sets
`mi_dbase = buffer + 0x1FE`, which is file offset `0x208`. Everything else is addressed
backwards from there, which is why the layout looks arbitrary until you write it as
offsets from `mi_dbase`:

| code | expression | file offset |
|---|---|---|
| `mq_setp` | `mi_env = mi_dbase - 0x168` | `0x0A0` — envelope block |
| `mq_parh` | `mq_pacm(p - 90)` | `0x1AE` — channel/program map |
| `mq_dise` | `mi_env + (voice - 1) * 8` | envelope record for voice 1..15 |

Two independent name tables exist because the editor keeps a "current instrument set"
and the set the song was written with; they differ in `STANDARD.SND` and in songs that
were re-instrumented (e.g. `WALTZ.SNG` list A has `Guitar\0t` — a fragment of the longer
name that used to be in that slot, never cleared).

---

## 2. Voices, channel map and program map

There are **15 logical voices**, numbered 1..15 (voice 0 is reserved; in LCP it is used
for game sound effects). Voice *v* uses:

- instrument name `A[v-1]`, `B[v-1]`
- envelope record `v-1` at `0x0A0 + (v-1)*8`
- `chanmap[v-1]` and `progmap[v-1]`

`mq_pacm` subtracts one from every byte as it loads them, so a stored `0x01` means 0:

```c
for (i = 1; i < 16; i++) {
    mi_chmap[i] = file[0x1AE + i - 1] - 1;
    mi_pgmap[i] = file[0x1BD + i - 1] - 1;
}
```

### Channel map byte (after the −1)

| bits | meaning |
|---|---|
| 0..3 | MIDI OUT channel (0..15) |
| 4..7 | octave group; `mq_dise` transposes MIDI OUT by `-(3 - hi_nibble) * 12` semitones |

A voice that is never played usually stores `0x01` → `0x00`.

### Program map byte (after the −1)

MIDI program number sent as `0xCn`/program by `mq_sepc` on the voice's first note, and
by `mq_resp` once per physical channel at song start. It only affects MIDI OUT; the PSG
uses the envelope record instead.

Example — `WALTZ.SNG`:

| voice | instrument | chanmap | MIDI ch | transpose | program |
|---|---|---|---|---|---|
| 2 | Accordian | `0x30` | 0 | 0 | 6 |
| 5 | Guitar | `0x21` | 1 | −12 | 8 |
| 7 | Hihat | `0x32` | 2 | 0 | 15 |
| 13 | Bass | `0x23` | 3 | −12 | 12 |

---

## 3. Envelope records (`0x0A0`, 15 × 8 bytes)

`psg_cpE` copies these eight bytes straight into the runtime `PSG_ENVELOPE` struct at
offsets 1..8, then `mq_dise` splits two of them into nibble pairs:

| byte | field | notes |
|---|---|---|
| 0 | `attack_start_vol` | low nibble = start volume 0..15; **high nibble = PSG mixer** |
| 1 | `attack_duration` | low nibble = attack time index; **high nibble = octave**, shift = `(2 - hi) * 12` semitones |
| 2 | `attack_target_vol` | peak volume 0..15 |
| 3 | `decay_duration` | time index |
| 4 | `decay_target_vol` | |
| 5 | `sustain_duration` | time index (into a different table, see below) |
| 6 | `sustain_target_vol` | |
| 7 | `release_duration` | time index |

**Mixer nibble.** `mq_dise` clears the tone- and noise-disable bits for the allocated PSG
channel, then ORs this nibble in (`bits: 0 = disable tone, 3 = disable noise`):

| nibble | result |
|---|---|
| `0x0` | tone + noise |
| `0x1` | noise only |
| `0x8` | tone only (all pitched instruments on this disk) |
| `0x9` | silent |

`Hihat` and `Snare` are the two voices that ship with `0x1`.

**Octave nibble.** The PSG frequency index is `note + (2 - hi_nibble(byte 1)) * 12`, so a
stored high nibble of 2 means "as written", 1 means one octave up, 3 one octave down.
This is independent of the MIDI OUT transposition in the channel map.

### The ADSR engine (`psg_upE`, 240 Hz)

Each phase ramps `current_volume` toward the next target with a Bresenham accumulator:

```
delta  = |from - to| * rate_table[duration]
timer  = time_table[duration]
each tick:  accum += delta;  while (accum > 360) { vol += dir; accum -= 360; }
```

with these ROM tables (`mi_evrt`, `mi_evtt`, `mi_evst`, `mi_evrl`, index 0..15):

| idx | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| rate `mi_evrt` | 0 | 360 | 180 | 120 | 85 | 72 | 60 | 45 | 30 | 20 | 15 | 12 | 10 | 8 | 6 | 4 |
| time `mi_evtt` | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 8 | 12 | 18 | 24 | 30 | 36 | 45 | 60 | 90 |
| sustain time `mi_evst` | 0 | 1 | 2 | 4 | 8 | 18 | 24 | 40 | 45 | 60 | 72 | 90 | 120 | 180 | 360 | 30000 |
| sustain rate `mi_evrl` | 0 | 360 | 180 | 90 | 45 | 20 | 15 | 9 | 8 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |

Phases run attack → decay → sustain → release → fadeout; a duration of 0 makes the phase
snap straight to its target and fall through. The final volume is clamped to
`max_volume`, which the note's velocity sets. Only three notes can sound at once — the
YM2149 has three channels — and `mq_dise` steals the voice furthest along its envelope.

---

## 4. Event stream

The stream starts at `0x208` with a `0x00`, then header commands, then a `0x00` that both
terminates the header and serves as the first group separator (`mq_skip` scans to it and
hands the pointer to `mq_setp`).

### 4.1 Header commands (`mq_parh`)

Walked from `0x208` until the first `0x00`.

| byte | size | operand | meaning |
|---|---|---|---|
| `0x80` | 3 | byte 2 | **key signature** 0..15 → `mq_bust` builds the note remap table (§5) |
| `0x81` | 2 | byte 1 | **tempo** in quarter notes per minute; `ticks_per_unit = 2400 / tempo` |
| `0x83` | 2 | byte 1 | master volume — parsed and discarded by the player |
| `0x84` | 3 | byte 2 | **default velocity** 0..127; also sets the PSG volume cap (below) |
| `0xC0` | 3 | — | program change — skipped in the header |
| `0xFF` | 1 | — | end |
| `0x01`..`0x7F` | 3 | — | a stray note event; skipped |

Byte 1 of `0x80` and `0x84` is `0x01` in every file on this disk and is ignored by the
player.

Velocity → PSG volume cap: `<0x17`→5, `<0x27`→7, `<0x37`→9, `<0x57`→11, `<0x67`→13,
else 15.

`WALTZ.SNG` header: `00 | 80 01 09 | 83 05 | 81 6D | 84 01 7F | 00`
→ key 9 (F major), volume 5, tempo 109, velocity 127.

### 4.2 Groups

After the header the stream is a sequence of **groups**, each introduced by one or more
`0x00` bytes:

```
00  <event> <event> ...  00  <event> ...  00 ... FF
```

Every event inside one group happens at the same moment — that is how chords and
multi-voice writing are expressed. The time to the *next* group is taken from the
**duration index of the group's first note event** (`mq_rdur` peeks at it before parsing).

### 4.3 Note event (3 bytes, first byte `0x00`..`0x7F`)

```
byte 0:  . T O N c c c c      byte 1:  m m A d d d d d      byte 2:  . n n n n n n n
```

| field | bits | meaning |
|---|---|---|
| `cccc` | 0.0–3 | logical voice 1..15 |
| `N` | 0.4 | **set = skip**: the event contributes its duration but plays nothing |
| `O` | 0.5 | **tie continuation**: no Note-On is sent; only the Note-Off is scheduled |
| `T` | 0.6 | **tie**: suppresses the scheduled Note-Off, so the note sustains |
| `ddddd` | 1.0–4 | duration index into the table in §6 |
| `A` | 1.5 | **accent**: velocity `0x7F`, PSG volume 15 for this note |
| `mm` | 1.6–7 | accidental (below) |
| `nnnnnnn` | 2.0–6 | note number |

**Accidentals.** `mq_pars` decides what byte 2 means from `mm`:

| `mm` | meaning | note played |
|---|---|---|
| `00` | follow the key signature | `scale_table[n]` (§5) |
| `01` | natural | `n` |
| `10` | sharp | `n + 1` |
| `11` | flat | `n - 1` |

**Ties.** A held note is written as a run: first event `T=1, O=0` (sounds, no automatic
Note-Off), any number of `T=1, O=1` middles, and a final `T=0, O=1` which schedules the
Note-Off after its own duration. `mq_qnne` writes bit 7 of the queued note word from `T`,
and `mq_snof` refuses to emit a Note-Off when that bit is set.

### 4.4 Commands (first byte ≥ `0x80`)

| byte | size | meaning |
|---|---|---|
| `0x82` | 1 | bar line. If no note has been decoded in this group yet, the group's step is re-read from the following event |
| `0x85 n` | 2 | loop start, `n` total passes. Pushes `{position after the operand, n-1}` on a 20-deep stack |
| `0x86` | 1 | loop end. Pops/decrements; jumps back while the counter is non-zero |
| `0xFF` | 1 | end of song |
| any other | 1 | ignored (a one-byte no-op) |

---

## 5. Key signatures (`mq_bust`)

Header command `0x80` supplies an index into a 16-byte chord-mask table:

```
g_msmk[16] = FF FF 77 37 33 13 11 01  00 FE EE EC CC C8 88 00
```

`mq_bust` builds a 132-entry remap: identity, then within every octave it displaces the
seven natural degrees whose mask bit is **clear** — up one semitone for index ≤ 8, down
one for index > 8. Mask bit *n* selects degree `[B, A, G, F, E, D, C][n]`.

That makes the index a standard circle-of-fifths position:

| index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| key | C | C | G | D | A | E | B | F# | C# | F | Bb | Eb | Ab | Db | Gb | Cb |
| accidentals | 0 | 0 | 1♯ | 2♯ | 3♯ | 4♯ | 5♯ | 6♯ | 7♯ | 1♭ | 2♭ | 3♭ | 4♭ | 5♭ | 6♭ | 7♭ |

Index 1 returns early (pure C major); index 0 has the all-ones mask and is equivalent.
Every key signature found on this disk decodes to a musically sensible key —
`CANON.SNG` is index 3 (D major, correct for Pachelbel's Canon in D), `TANGO.SNG` is
index 10 (B♭), `FIVE4.SNG` index 14 (G♭).

Notes written with `mm = 00` therefore carry **staff position**, not pitch: the key
signature is applied at playback time. `mm = 01/10/11` are the score's explicit
naturals, sharps and flats.

---

## 6. Timing

`mq_inti` installs the sequencer on **MFP Timer A** with `Xbtimer(0, 5, 0x28, mq_tick)` —
prescaler ÷64, counter 40:

```
2 457 600 / (64 * 40) = 960 Hz
```

This routine is byte-identical in `AUDIO.PRG` (`text+0xea12`) and `LCP_STX.PRG`
(`text+0x1112`). `mq_tick` runs the ADSR processor every 4th interrupt (240 Hz) and the
sequencer whenever its prescaler expires.

The duration index selects a value from `mi_ndt`:

| idx | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| units | 0 | 2 | 2 | 3 | 4 | 5 | 6 | 8 | 9 | 12 | 16 | 18 | 24 | 32 | 36 | 48 | 64 | 72 | 96 | 128 | 144 | 0 |
| value | – | 64th | 64th | 32nd | 16thT | 32nd. | 16th | 8thT | 16th. | 8th | 4thT | 8th. | **1/4** | 2ndT | 4th. | 1/2 | 1T | 1/2. | 1/1 | 2T | 1/1. | – |

**One unit is 1/24 of a quarter note**; the ladder is (plain, triplet of the next value,
dotted) repeated per level. The unit in hardware ticks is
`spb = 2400 / tempo`, so

```
quarter note = 24 * (2400 / tempo) ticks = 57600 / tempo ticks
             = 60 / tempo seconds  at 960 Hz
```

— the tempo byte is literally BPM.

The engine splits each step across two sequencer states (`mq_advs`): the parse pass
advances `(units - 1) * spb` ticks and the wait pass adds the remaining `spb`, so the
total is exactly `units * spb`. The Note-Off, however, is scheduled at
`(units - 1) * spb`, giving every note a one-unit-of-`spb` articulation gap.

**Cross-check.** Bar markers agree with the derived timing: `STARSPAN.SNG` has 32 bar
markers and decodes to 96.0 quarter notes → 3/4, correct for the anthem. `REQUIEM.SNG`
has 51 bars over 209.2 quarters ≈ 4/4. Song lengths land on whole quarter-note counts
(`CANON` 392.0, `ECHOROCK` 396.0, `FIVE4` 204.0, `FOLK` 108.0).

---

## 7. Output paths (`mq_dise`)

One dispatcher feeds both outputs; either can be disabled (`g_moen`, `psg_out`).

**MIDI OUT.** The note is transposed by `-(3 - hi_nibble(chanmap)) * 12`, then the 2- or
3-byte message goes out via XBIOS `Midiws` (or byte-by-byte through the ACIA in direct
mode). Channel and program come from the maps in §2.

**PSG.** Untransposed note plus the envelope's octave shift indexes a 128-entry period
table; the period is written to the channel's fine/coarse registers, the mixer nibble to
register 7, and `period/60` to the noise register 6. The eight envelope bytes are copied
into the channel's ADSR state and the attack begins. Notes outside 36..96 (`g_mnlo`,
`g_mnhi`) are dropped; a frequency index of 22 or below starts in the fadeout phase
instead of attack.

---

## 8. Lyrics

Bytes after the terminating `0xFF` are a lyric block. The player never reads them —
`mq_pars` stops at `0xFF` — but Music Studio displays them beneath the staff.
On this disk only `STARSPAN.SNG` has one: 667 bytes at `0x838`, plain text with syllables
hyphenated and runs of spaces used for horizontal alignment:

```
Oh -  say can you see   by  the dawn's  ear-ly  light,      what  so  proud-ly ...
```

The five longwords at `0x1CC` relate to it: in `STARSPAN.SNG` the first is `0x00000838`
(where the lyric block starts) and the other four are `0x00000AD2` (its last byte). They
are not reliable in general — `COUNTRY2.SNG` has `0x00060D54` in slots 1–4 with no lyrics
at all, and Little Computer People's lyric-stripped `STARSPAN.ORG` carries `0x0002DA5A`,
both of which are stale absolute RAM addresses. **Locate the lyric block by scanning past
the `0xFF`, not by trusting these pointers.**

This block is also the whole difference between this disk's `STARSPAN.SNG` (2771 bytes)
and LCP's `STARSPAN.ORG` (2104 bytes): the game's copy is the same song with the 667-byte
lyric block and those pointers removed.

---

## 9. Fields not consumed by the player

These are written by the editor and ignored by the playback code, so their meaning is
inferred rather than proven:

- `0x200`: four 16-bit fields. 35 of the 37 songs across this disk and Little Computer
  People hold `FFFF 0000 0000 0000`; `BRANDEN.SNG` has `FFFF 8000 0040 0080` and LCP's
  `MAPLE.ORG` has `0005 0009 0021 0043`. The player never reads them and both outliers
  decode and play normally.
- `0x1CC`: the five longwords of §8.
- the second name table at `0x118`, and the master volume operand of `0x83`.

---

## 10. Decoder

[`../source/tools/sngdump.py`](../source/tools/sngdump.py) implements all of the above.

```bash
python3 source/tools/sngdump.py *.SNG                    # structure report
python3 source/tools/sngdump.py --events CANON.SNG       # every stream event
python3 source/tools/sngdump.py --midi canon.mid CANON.SNG
python3 source/tools/sngdump.py --json WALTZ.SNG
```

The MIDI export writes a format-1 file with a division of `24 * spb`, so one MIDI tick
equals one 960 Hz sequencer tick and the timing is exact rather than rounded. Track 0
carries the tempo, the title and (where present) the lyric block; each voice that
actually plays gets its own named track (`02 Accordian`, `13 Bass`, …), because several
Music Studio voices routinely share one MIDI channel and would otherwise be merged.
Channel, program, velocity and transposition all follow the MIDI OUT path of `mq_dise`.

Little Computer People's own songs are converted in [`midi/`](midi/); see
[`README.md`](README.md).  (This file was written against the Music Studio disk,
so "this disk" below means that disk -- the format is the same one LCP reads.)

[`../source/tools/psgrender.py`](../source/tools/psgrender.py) is the other export: a tick-accurate
re-implementation of the engine's **YM2149** path (`mq_tick`, `mq_advs`, `mq_qnne`,
`mq_expN`, `mq_dise`, `psg_upE`), producing a register-write log and an audio rendering
of it. The two paths transpose differently and the PSG has three voices rather than
sixteen, so they are not the same performance — see [`README.md`](README.md).

Verification: all 21 `.SNG` files on this disk, plus `TESTSNG` (a header-less raw event
stream) and `STANDARD.SND`, decode to the final byte with no warnings and no
resynchronisation. `CANON.SNG`'s first voice decodes to D2–A2–B2–F♯2, the Pachelbel
ground bass, with the F♯ supplied by key signature 3 rather than by an accidental in the
data — which exercises the container, the event decoder and the key-signature table at
once.
