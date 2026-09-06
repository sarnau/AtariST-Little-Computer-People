# Songs: decoded, converted and rendered

Everything in `DATA/*.SNG` and `DATA/*.ORG`, decoded from the format documented in
[`SNG_FORMAT.md`](SNG_FORMAT.md) and exported two ways.

| | |
|---|---|
| [`midi/`](midi/) | format-1 MIDI, one named track per voice |
| [`psg/`](psg/) | YM2149 renderings (44.1 kHz mono WAV) + register-write logs |
| [`sng_report.txt`](sng_report.txt) | full structure dump of all 16 files |
| [`SNG_FORMAT.md`](SNG_FORMAT.md) | the file format, derived from the playback code |
| [`LCP_ENGINE_COMPARISON.md`](LCP_ENGINE_COMPARISON.md) | why `LCP_STX.PRG` and Activision's *Music Studio* share this engine |

Tools are in [`../source/tools/`](../source/tools/): `sngdump.py` (decoder + MIDI export),
`psgrender.py` (YM2149 engine simulation + audio), `render_psg_all.py` (batch driver),
`midicheck.py` (independent MIDI re-parse), `prg.py` (GEMDOS PRG reader).

```bash
for f in DATA/*.SNG DATA/*.ORG; do b=$(basename "$f")
  python3 source/tools/sngdump.py --midi "music/midi/${b%.*}.mid" "$f"; done
cd source/tools && python3 render_psg_all.py ../../DATA ../../music/psg ../../DATA/LCP_STX.PRG
python3 source/tools/midicheck.py music/midi/*.mid
```

The batch driver applies one common gain (1.102x) across all songs so their relative
loudness is preserved; rendering a single file with `psgrender.py` normalises that file
alone, so it will not be byte-identical to the copy here.

## The two exports are different music

`mq_dise` sends every note down both output paths with a *different* transposition:

- **MIDI OUT** transposes by `-(3 - hi_nibble(channel map)) * 12`.
- **PSG** transposes by `(2 - hi_nibble(envelope byte 1)) * 12`, then looks the result up
  in `psg_freq` (the literal `0x19922` in `LCP_STX.PRG`'s `mq_dise`, i.e. `data+0x246`).

In `CANON.SNG` the ground bass is written D3-A2-B2-F#2; MIDI OUT sends it as written,
while the PSG plays it an octave higher because the Clarinet voice carries a +1 octave
nibble. The register log confirms it: periods 425, 568, 506, 675 -> 294.1, 220.1, 247.0,
185.2 Hz.

The PSG also has three channels where the MIDI side has sixteen, so `mq_dise` steals the
channel furthest along its envelope. The counts are in the table below.

### What is exact in the PSG render

The register log is a faithful trace of the engine: the 960 Hz Timer-A tick, `psg_upE` on
every 4th tick, the two-phase `mq_advs`, `mq_pars`, the note queue and `mq_expN`/`mq_snof`
expiry, channel allocation and voice stealing, the 36..96 note guard, the `index <= 22`
fadeout case, the eight-byte ADSR records with their four ROM rate/time tables and the
`> 0x168` Bresenham accumulator, and `psg_freq` read straight out of `LCP_STX.PRG`.

Only turning that log into audio is approximate: tone channels are box-filtered per output
sample rather than simulated at the 125 kHz counter rate, the per-channel `tone OR noise`
stage is computed as a product of the two duty signals, channels are summed linearly, and
the unipolar output is high-passed at 20 Hz to stand in for the ST's output coupling.

## Cross-check against Activision's Music Studio disk

Twelve of these files are byte-identical to a demo song shipped on the *Music Studio*
disk, and they convert to byte-identical MIDI and byte-identical audio -- a regression
check on the whole toolchain.

| Music Studio | here | MIDI | PSG WAV |
|---|---|---|---|
| `AISLEDAN`, `BALLAD`, `BOOGIE`, `CALYPSO`, `CANON`, `COUNTRY2`, `MYSTERY`, `TANGO` | same names | identical | identical |
| `FIVE4` | `FIVEFOUR.SNG` | identical | identical |
| `INTRO` | `BEBOP.SNG` | identical | identical |
| `PRELUDE`, `REQUIEM` | `PRELUDE.ORG`, `REQUIEM.ORG` | identical | identical |
| `BOSSA` | `BOSSA.SNG` | differs | differs |
| `FOLK` | `FOLKSONG.ORG` | differs | differs |
| `STARSPAN` | `STARSPAN.ORG` | differs | **identical** |

The three differences are the ones the format analysis predicts:

- **`BOSSA.SNG`** differs by one byte -- the `0x84` default-velocity operand, `0x3F` on
  the Music Studio disk and `0x7F` here. It reaches both outputs: MIDI velocities and the
  PSG volume cap. Its render peaks at 0.71 versus 0.42 for the other copy.
- **`FOLKSONG.ORG`** is a different arrangement, 75 differing bytes through the stream.
- **`STARSPAN.ORG`** differs *only* in the MIDI file: the Music Studio copy carries a
  667-byte lyric block that this one had stripped, and the exporter writes it as a text
  meta event. The audio is bit-for-bit identical, confirming the lyric block is inert.

## `MAPLE.ORG`

The one song here that is not on the Music Studio disk: **"MAPLE LEAF RAG by Scott
Joplin"**, A-flat major, 109 bpm, 144 bar markers over 287.8 quarter notes (2/4, correct
for a rag), 3,717 note events.

It is written on a **single logical voice** (voice 10, "Sax") carrying full piano chords --
up to 7 notes at once. On MIDI OUT that is fine. On three PSG channels it forces
`mq_dise` to steal a voice **880 times**, against 48 for the next worst song.

Its 8-byte field at `0x200` is also unusual -- `0005 0009 0021 0043` where 35 of the 37
known songs hold `FFFF 0000 0000 0000`. The player never reads it and the song decodes and
plays normally.

## Songs

| file | title | tempo | voices | notes | length | PSG steals |
|---|---|---|---|---|---|---|
| `AISLEDAN.SNG` | Aisle Dance by Ed Bogas | 150 | 5 | 751 | 78 s | 7 |
| `BALLAD.SNG` | Ballad by Ed Bogas | 141 | 6 | 586 | 62 s | 0 |
| `BEBOP.SNG` | Bebop by Ed Bogas | 138 | 5 | 513 | 47 s | 5 |
| `BOOGIE.SNG` | Boogie by Ed Bogas | 160 | 3 | 546 | 54 s | 8 |
| `BOSSA.SNG` | Bossa Nova by Ed Bogas | 78 | 4 | 602 | 65 s | 2 |
| `CALYPSO.SNG` | Calypso by Ed Bogas | 133 | 4 | 420 | 43 s | 0 |
| `CANON.SNG` | Pachelbel's Canon in D | 160 | 2 | 667 | 147 s | 3 |
| `COUNTRY2.SNG` | Country Too by Ed Bogas | 130 | 4 | 725 | 54 s | 3 |
| `FIVEFOUR.SNG` | Five Four by Ed Bogas | 171 | 4 | 664 | 71 s | 0 |
| `FOLKSONG.ORG` | Folk Song by Ed Bogas | 100 | 1 | 571 | 65 s | 0 |
| `MAPLE.ORG` | MAPLE LEAF RAG by Scott Joplin | 109 | 1 | 2568 | 158 s | 880 |
| `MYSTERY.SNG` | Mystery by Ed Bogas | 104 | 3 | 571 | 74 s | 22 |
| `PRELUDE.ORG` | Prelude | 120 | 1 | 578 | 56 s | 9 |
| `REQUIEM.ORG` | Kyrie eleison-Mozart's REQUIEM | 128 | 1 | 1200 | 94 s | 48 |
| `STARSPAN.ORG` | Star-Spangled Banner / F.S.Key | 133 | 8 | 389 | 43 s | 4 |
| `TANGO.SNG` | Tango by Ed Bogas | 133 | 6 | 618 | 57 s | 2 |
