# Activision *The Music Studio* — the demo songs from its own disk

Little Computer People's sequencer is object-for-object the same engine that ships in
Activision's *The Music Studio* (Atari ST, `AUDIO.PRG`, 1985-11-20); see
[`../LCP_ENGINE_COMPARISON.md`](../LCP_ENGINE_COMPARISON.md). Eleven of that disk's
twenty-one demo songs are byte-identical to files in this repo's `DATA/`.

These are all twenty-one of them, decoded and exported exactly like LCP's own songs in
[`../midi/`](../midi/) and [`../psg/`](../psg/):

| | |
|---|---|
| [`midi/`](midi/) | format-1 MIDI, one named track per voice |
| [`psg/`](psg/) | YM2149 register-write logs (the WAV renderings are gitignored) |
| [`sng_report.txt`](sng_report.txt) | structure dump of all 21 files, plus `TESTSNG` and `STANDARD.SND` |

The format and the two export paths are documented in [`../SNG_FORMAT.md`](../SNG_FORMAT.md)
and [`../README.md`](../README.md); the tools are in
[`../../source/tools/`](../../source/tools/).

## Regenerating

The source `.SNG` files are **not** in this repo — they live on the Music Studio disk
image, at
`~/Desktop/Retro/Atari ST/music_studio_activision_(usa)/Music Studio (The) - Activision/`.
With that directory available:

```bash
MS="$HOME/Desktop/Retro/Atari ST/music_studio_activision_(usa)/Music Studio (The) - Activision"
for f in "$MS"/*.SNG; do b=$(basename "$f")
  python3 source/tools/sngdump.py --midi "music/music-studio/midi/${b%.SNG}.mid" "$f"; done
cd source/tools && python3 render_psg_all.py "$MS" ../../music/music-studio/psg
```

`AUDIO.PRG` is not needed for the audio: `psg_freq` is byte-identical in both binaries, so
`DATA/LCP_STX.PRG` produces the same renderings. `render_psg_all.py` finds whichever
binary is present.

## Overlap with `DATA/`

Twelve exports here are byte-identical to their counterpart one directory up, which is
what you would expect from identical inputs through the same tools:

`AISLEDAN`, `BALLAD`, `BOOGIE`, `CALYPSO`, `CANON`, `COUNTRY2`, `MYSTERY`, `TANGO`,
`FIVE4`↔`FIVEFOUR`, `INTRO`↔`BEBOP`, `PRELUDE`, `REQUIEM`.

The three near-misses are documented in [`../README.md`](../README.md): `BOSSA` differs by
one velocity byte, `FOLK`/`FOLKSONG` is a different arrangement, and `STARSPAN` differs
only in the MIDI file because this disk's copy still carries its 667-byte lyric block.

## Songs only on this disk

Six of the twenty-one never made it into the game: `BRANDEN` (Brandenburg Concerto 2,
19,301 bytes and 4½ minutes — the longest song in either set), `ECHOROCK`, `FIFTIES`,
`HARMONY`, `LEADVOCL` and `WALTZ`.

## Songs

| file | title | tempo | voices | notes | length | PSG steals |
|---|---|---|---|---|---|---|
| `AISLEDAN.SNG` | Aisle Dance by Ed Bogas | 150 | 5 | 751 | 78 s | 7 |
| `BALLAD.SNG` | Ballad by Ed Bogas | 141 | 6 | 586 | 62 s | 0 |
| `BOOGIE.SNG` | Boogie by Ed Bogas | 160 | 3 | 546 | 54 s | 8 |
| `BOSSA.SNG` | Bossa Nova by Ed Bogas | 78 | 4 | 602 | 65 s | 2 |
| `BRANDEN.SNG` | Brandenburg Concerto 2 by Bach | 104 | 3 | 3689 | 272 s | 23 |
| `CALYPSO.SNG` | Calypso by Ed Bogas | 133 | 4 | 420 | 43 s | 0 |
| `CANON.SNG` | Pachelbel's Canon in D | 160 | 2 | 667 | 147 s | 3 |
| `COUNTRY2.SNG` | Country Too by Ed Bogas | 130 | 4 | 725 | 54 s | 3 |
| `ECHOROCK.SNG` | Echo Rock by Ed Bogas | 160 | 7 | 971 | 148 s | 0 |
| `FIFTIES.SNG` | Fifties by Ed Bogas | 109 | 4 | 854 | 100 s | 5 |
| `FIVE4.SNG` | Five Four by Ed Bogas | 171 | 4 | 664 | 71 s | 0 |
| `FOLK.SNG` | Folk Song by Ed Bogas | 100 | 2 | 571 | 65 s | 0 |
| `HARMONY.SNG` | Harmony by Ed Bogas | 100 | 5 | 267 | 45 s | 1 |
| `INTRO.SNG` | Bebop by Ed Bogas | 138 | 5 | 513 | 47 s | 5 |
| `LEADVOCL.SNG` | Lead Vocal by Ed Bogas | 141 | 4 | 715 | 82 s | 7 |
| `MYSTERY.SNG` | Mystery by Ed Bogas | 104 | 3 | 571 | 74 s | 22 |
| `PRELUDE.SNG` | Prelude | 120 | 1 | 578 | 56 s | 9 |
| `REQUIEM.SNG` | Kyrie eleison-Mozart's REQUIEM | 128 | 1 | 1200 | 94 s | 48 |
| `STARSPAN.SNG` | Star-Spangled Banner / F.S.Key | 133 | 8 | 389 | 43 s | 4 |
| `TANGO.SNG` | Tango by Ed Bogas | 133 | 6 | 618 | 57 s | 2 |
| `WALTZ.SNG` | Waltz by Ed Bogas | 109 | 4 | 447 | 64 s | 6 |
