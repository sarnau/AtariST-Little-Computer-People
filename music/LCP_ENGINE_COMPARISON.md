# The Music Studio (AUDIO.PRG) vs. Little Computer People — sound engine comparison

Comparison of the music playback code on this disk against the Atari ST version of
Activision's *Little Computer People* (LCP), reverse-engineered in a parallel session at
`/Users/sarnau/GitHub/AtariST-Little-Computer-People`.

Reference binaries:

| | file | text | data | bss |
|---|---|---|---|---|
| Music Studio | `AUDIO.PRG` (117,957 bytes, dated 1985-11-20) | `0x18536` | `0x2b30` | `0x10c1c` |
| Little Computer People | `DATA/LCP_STX.PRG` (123,352 bytes) | `0x196dc` | `0x2fe4` | `0x2dc3a` |

Both are Alcyon C 4.14 / CP/M-68K builds — both carry the same runtime banner
`CP/M-68K(tm), Version 1.2, Copyright (c) 1983, Digital Research`.

All addresses below are **segment-relative offsets** (text+N / data+N), not load addresses.
LCP's Ghidra database uses text base `0x10000`, so `LCP text+0x15ae` = Ghidra `0x115ae`.

---

## 1. Verdict

**It is not a similar engine — it is the same engine, from the same source files, in the
same object-code form.** LCP's sequencer object holds 24 routines; 21 of them are present
in `AUDIO.PRG`, and eleven are byte-for-byte identical once relocated address operands are
masked out,
including the single biggest one (the 3,052-byte PSG envelope processor). The rest are
95 % identical revisions of each other. The shared static tables are byte-identical too.

LCP's `.SNG`/`.ORG` songs are not merely "in Music Studio format" (which the LCP analysis
already established) — eleven of them are **byte-identical copies of the demo songs on
this disk**.

The sharing extends past the sound code: the two programs also share the same hand-written
floppy copy-protection loader (~7.5 KB identical) and the same VDI binding library.

---

## 2. Shared sequencer object code

LCP's sound engine lives in one object at `LCP text+0x12a … +0x219a` (the port calls it
`midi_seq.c`, compiled into `globals.o`). Music Studio has the same routines, in the same
order, at `AUDIO text+0xd6c8 … +0xf844`, with three of them relocated into other modules.

Full machine-readable table: [`shared_functions.tsv`](shared_functions.tsv).

| function (LCP name) | LCP text | size | AUDIO text | size | relation |
|---|---|---|---|---|---|
| `mq_skip`  | `0x0012a` |   64 | — | — | absent |
| `mq_inis`  | `0x0016a` |   82 | — | — | absent |
| `mq_setp`  | `0x001bc` |  104 | `0x0d6c8` | 104 | variant, 70 % |
| `mq_stap`  | `0x00224` |   70 | `0x0d864` |  76 | variant, 96 % |
| `mq_pshl`  | `0x0026a` |   76 | `0x0d8b0` |  76 | **identical** |
| `mq_popl`  | `0x002b6` |  130 | `0x0d8fc` | 130 | **identical** |
| `mq_pars`  | `0x00338` |  662 | `0x0d97e` | 716 | variant, 93 % |
| `mq_rdur`  | `0x005ce` |   90 | `0x0dc4a` | 140 | variant, 76 % |
| `mq_qnne`  | `0x00628` |  392 | `0x0dcd6` | 392 | **identical** |
| `mq_snof`  | `0x007b0` |  154 | `0x0de5e` | 154 | **identical** |
| `mq_sepc`  | `0x0084a` |  206 | `0x0def8` | 206 | **identical** |
| `mq_dise`  | `0x00918` | 1258 | `0x0dfc6` |1354 | variant, 95 % |
| `mq_expN`  | `0x00e02` |   98 | `0x0e510` |  98 | variant, 99 % |
| `mq_rmev`  | `0x00e64` |   94 | `0x0e572` |  94 | **identical** |
| `mq_advs`  | `0x00ec2` |  378 | `0x0e7a2` | 410 | variant, 93 % |
| `mq_stop`  | `0x0103c` |  214 | `0x0e93c` | 214 | variant, 98 % |
| `mq_inti`  | `0x01112` |   80 | `0x0ea12` |  80 | **identical** |
| `mq_extm`  | `0x01162` |   34 | `0x0ea62` |  34 | **identical** |
| `mq_resp`  | `0x01184` |  118 | `0x0ebd4` | 132 | variant, 93 % |
| `mq_parh`  | `0x011fa` |  354 | — | — | absent |
| `mq_pacm`  | `0x0135c` |   88 | `0x07b9e` |  92 | variant, 88 % |
| `mq_bust`  | `0x013b4` |  466 | `0x08ce8` | 466 | **identical** |
| `psg_cpE`  | `0x01586` |   40 | `0x03f14` |  40 | **identical** |
| `psg_upE`  | `0x015ae` | 3052 | `0x0ec58` |3052 | **identical** |
| `mq_tick`  | `0x0219a` |   40 | `0x119da` |  40 | **identical** (Timer-A ISR, hand asm) |

"Identical" means every byte matches after zeroing the longs listed in each program's
relocation table — i.e. same instructions, same immediates, same structure, only the
absolute addresses of globals differ.

The headline is `psg_upE` (`psg_process_envelopes`): 3,052 bytes of software-ADSR PSG
envelope processing, identical down to the byte. So is `mq_bust`, the scale/chord
quantisation table builder, and `mq_sepc`, the program-change / envelope-load path.

### 2.1 What only Music Studio has

Three routines sit inside the same code block in `AUDIO.PRG` and have no LCP counterpart:

| AUDIO text | size | what it does |
|---|---|---|
| `0x0e5d0` | 332 | interactive note-on: key-scancode handling (`0x18`/`0x2c` step the note up/down), builds a `0x90\|ch, note, vel` message at `text+0x1bafc`, calls `mq_sepc` then `mq_dise` |
| `0x0e71c` | 134 | matching note-off: same message with velocity 0, then `Giaccess(0, 0x88)` to silence the channel |
| `0x0ea84` | 336 | staff/instrument-editor screen drawing — calls `psg_cpE` and the VDI `vro_cpyfm` binding |

These are the editor's live-preview keyboard: the part a player-only build has no use for.

### 2.2 What only LCP has

`mq_skip`, `mq_inis` and `mq_parh` are the *file loader*: strip the 10-byte Music Studio
signature, position `mi_dbase` at body+`0x1FE`, parse the header commands, skip padding.
Music Studio does not need them — it holds the song in its own editor buffers and writes
the file rather than parsing it back through a stream parser.

### 2.3 How the variants differ

The divergences are consistently "editor bookkeeping the game build drops"
(see `mq_dise_diff.txt` (in the Music Studio disk folder)):

- **`mq_rdur`** (90 → 140 bytes). LCP's version only skips `0x00` padding bytes. Music
  Studio's also maintains a bar/position counter (`text+0x24498`) and deliberately does
  *not* count `0x85`/`0x86` loop markers — the score display's current-measure tracking.
- **`mq_dise`** (1258 → 1354 bytes). One 90-byte insertion at `AUDIO text+0xe158`: an extra
  per-channel instrument lookup through a table at `text+0x23c24`, selecting an 8-byte
  envelope record (`asl #3` then add the envelope base) or falling back to program 5.
  The remaining 469 and 245-byte stretches are identical.
- **`mq_pars`** (662 → 716) and **`mq_advs`** (378 → 410) differ only by small inserted
  hunks of the same kind; both retain 190+ byte identical runs.

---

## 3. Shared static data

| table | AUDIO data | LCP data | bytes | status |
|---|---|---|---|---|
| PSG frequency table (`psg_fre`) + preceding block | `0x358` | `0x244` | 258 | identical |
| note-duration / event tables (`mi_evrt`…`mi_ndt`) | `0x253d` | `0x018f` | 130 | identical |
| note transpose identity table `g_mstr` + chord-mask table `g_msmk` | `0x720` | `0x36e` | 151 | identical |
| default program map `mi_pgtab` | `0x888` | `0x404` | 16 | identical |
| Alcyon/CP-M-68K runtime strings | `0x2ade` | `0x2f92` | 82 | identical |

`g_msmk` is the 16-entry chord mask
`FF FF 77 37 33 13 11 01 00 FE EE EC CC C8 88 00` that the LCP port had to dump from the
binary because the published Music Studio 2.0 documentation disagreed with it. This disk
carries the same sixteen bytes at `AUDIO data+0x7a6` — the documentation was wrong, the
1985 code was not.

The PSG frequency table (`0258 0238 0218 01fa 01dd 01c3 01a9 0191 …`) is identical for its
full 256 bytes. Immediately after it the two diverge: LCP stores the defaults
`mi_vel=0x7F, mi_dvel=0x7F, psg_dvol=0x0F, g_mnhi=0x60, g_mnlo=0x24`, Music Studio a
descending byte ramp instead.

---

## 4. Shared song data

Eleven of this disk's twenty-one demo songs ship inside LCP unchanged
([`song_files.tsv`](song_files.tsv)):

| Music Studio | LCP `DATA/` | relation |
|---|---|---|
| `AISLEDAN.SNG`, `BALLAD.SNG`, `BOOGIE.SNG`, `CALYPSO.SNG`, `CANON.SNG`, `COUNTRY2.SNG`, `MYSTERY.SNG`, `TANGO.SNG` | same names | byte-identical |
| `FIVE4.SNG` | `FIVEFOUR.SNG` | byte-identical |
| `INTRO.SNG` ("Bebop by Ed Bogas") | `BEBOP.SNG` | byte-identical |
| `PRELUDE.SNG`, `REQUIEM.SNG` | `PRELUDE.ORG`, `REQUIEM.ORG` | byte-identical |
| `BOSSA.SNG`, `FOLK.SNG`, `STARSPAN.SNG` | `BOSSA.SNG`, `FOLKSONG.ORG`, `STARSPAN.ORG` | same piece, different revision |
| `BRANDEN`, `ECHOROCK`, `FIFTIES`, `HARMONY`, `LEADVOCL`, `WALTZ` | — | not shipped with LCP |

LCP's `.ORG` files carry the identical `\xCD` `Mstudio` `\xCD` header — the extension is
LCP's own naming for "piano/record-player music", not a different format. Sixteen of the
twenty-one titles at offset `0x1e0` read "… by Ed Bogas", the composer credited on LCP.

### 4.1 `.SNG` layout, cross-checked from both sides

The LCP loader's constants (`sgPlay` strips 10 bytes; `mi_dbase = body + 0x1FE`;
`mq_pacm` reads a 90-byte block at `mi_dbase - 90`) map onto these files exactly:

| file offset | size | contents |
|---|---|---|
| `0x000` | 10 | signature `CD 4D 73 74 75 64 69 6F CD tt` — `tt` is a **file type**: `02` = song (`.SNG`/`.ORG`), `01` = instrument set (`STANDARD.SND`) |
| `0x00a` | 150 | instrument name table A — 15 × 10-byte NUL-padded names ("Blocks", "Accordian", …; trailing bytes still hold fragments of previously longer names) |
| `0x0a0` | 120 | 15 × 8-byte PSG envelope records (this is what `mq_sepc` loads on program change) |
| `0x118` | 150 | instrument name table B — same 15 × 10 layout |
| `0x1ae` | 90 | `mq_pacm`'s window (`mi_dbase − 90`): 15-byte channel map at `0x1ae`, 15-byte program map at `0x1bd`, five longwords at `0x1cc`, the 32-byte **song title** at `0x1e0` and 8 bytes at `0x200` (`FF FF` + flags) |
| `0x208` | … | MIDI event bytecode — this is `mi_dbase` |

Verified against `WALTZ.SNG`, `CANON.SNG`, `BRANDEN.SNG`, `MYSTERY.SNG` and the
header-less `TESTSNG` (which is a bare event stream starting with the same
`00 80 01 …` prologue that appears at `0x208` in the full files).

The complete format — event bytecode, key signatures, ADSR records, timing model,
lyrics — is documented in [SNG_FORMAT.md](SNG_FORMAT.md), with a decoder in
[`../source/tools/sngdump.py`](../source/tools/sngdump.py).

Two of the three "different revision" files in the table above are explained by it:
LCP's `STARSPAN.ORG` is this disk's `STARSPAN.SNG` minus its 667-byte lyric block, and
LCP's `BOSSA.SNG` differs from this disk's in exactly one byte — the `0x84` default
velocity at `0x212`, raised from `0x3F` to `0x7F`.

---

## 5. Sharing beyond the sound engine

Masked longest-common-run analysis over the whole text segments turns up 21.4 KB of
shared code in 64 runs. The largest single run is *not* the sequencer:

| run | AUDIO text | LCP text | bytes | what |
|---|---|---|---|---|
| 1 | `0x153d2` | `0x022be` | 7502 | the copy-protection loader (`cp_main`/`cpenc`/`cpbuf` in the LCP port) — supervisor mode, `flock` at `$43e`, 96 self-decrypting bytes keyed by drive number, direct 1772 FDC access via `$ff8604`/`$ff8606` and the DMA address registers |
| 2 | `0x0ec54` | `0x015aa` | 3056 | `psg_upE`, the envelope processor |
| 3 | `0x179d4` | `0x179ce` | 1326 | VDI bindings / Alcyon runtime |
| 4 | `0x0dc9e` | `0x005f0` |  987 | `mq_rdur`/`mq_qnne` tail |

So the same in-house protection scheme guards both disks, and the identical
`v_opnvw`, `v_gtext`, `vro_cpyfm`, `vsf_color`, … bindings (LCP's `vdistx.o`) are linked
into both.

---

## 6. Interpretation

Music Studio holds the **superset**: the editor build contains everything LCP's player
contains plus live keyboard preview, bar-position bookkeeping and an extra instrument
lookup in the dispatcher. LCP holds the **subset plus a loader**: three routines that read
a Music Studio file back off disk, which the authoring tool has no need for.

Notably, this disk is not the 2.0 release the LCP analysis cites: `AUDIO.PRG` is dated
1985-11-20, the same day as LCP's own `DATA` files. The two products were being built
side by side out of the same Activision source tree, sharing the sequencer, the PSG
envelope engine, the song file format, the composer, the demo songs and the copy
protection.

One correction worth carrying back to the LCP notes: the trailing `\x02` of the
`\xCD Mstudio \xCD\x02` signature is a file-type byte, not part of a fixed magic —
`STANDARD.SND` on this disk uses `\x01` for an instrument set.

---

## 7. Reproducing this

The tools live in [`../source/tools/`](../source/tools/): `prg.py` (GEMDOS PRG reader
producing a relocation-masked image), `objsyms.py`/`locate_obj.py`/`match_fns.py`/
`scan_all.py` (the object-matching pass), `common_code.py` (longest-common-run finder)
and `m68kdis.py` (capstone front-end).

Only the first is copied here; the matching tools and their raw output
(`shared_functions.tsv`, `song_files.tsv`, `fn_similarity.txt`, `missing_fns.txt`,
`mq_dise_diff.txt`) were produced in the Music Studio disk directory,
`~/Desktop/Retro/Atari ST/music_studio_activision_(usa)/Music Studio (The) - Activision/analysis/`,
which is where they can be re-run against `AUDIO.PRG`.
