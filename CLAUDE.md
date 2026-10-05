# Working notes for Claude / AI assistants on this repo

A **faithful C port** of the 1985 Atari ST game *Little Computer People*
(Activision).  **`DATA/LCP_STX.PRG` is the one and only reference** --
the uncracked shipped build, extracted from the Pasti image in the repo
root.  The C source in `source/` compiles under Alcyon C 4.14 (K&R) to a
binary that is **byte-identical** to it: 123 352 bytes, MD5
`eae52d14023b51d7ac459a90d37eed10` (text 104 156, data 12 260, bss
187 450, relocations 6 908).  Every change must keep it that way.

`docs/history.md` is the full log: how identity was reached, the
source-shape rules recovered from the binary, and every investigation
with its evidence.  Read the relevant section there before reopening a
question this file only summarises.

## Verify

    source/tools/run_all.sh              # ~4 min, includes the emulator
    source/tools/run_all.sh --quick      # ~10 s, build + host only

It checks byte identity on a clean SHIPPED build, runs the host build and
unit tests, then the Hatari runtime tests on a GATED build, and restores
the shipped build afterwards.  Run `--quick` after every change and the
full run before calling a larger change done.  Pieces:
`alcyon_build.sh && alcyon_link.sh`, `prg_diff.py`, `reloc_audit.py`
(expects A..D and F = 0, E = 1 for altScreen), `stx_check.sh`.

`prg_diff` alone is not enough for changes touching globals: it compares
after the BSS remap, so a wrong variable reference is invisible to it --
`reloc_audit.py` pairs every relocation and catches it.

For comment- or whitespace-only edits across many files, compare each
file's comment-stripped token stream against HEAD as well: it covers the
host-only and test-gated code the shipped binary never compiles.

## The rules

**Literal-faithful.**  Before writing or changing a code path, check it
against the original (Ghidra project `LCP.rep`, program `LCP.PRG.1.1`,
loaded at base 0x10000: Ghidra address - 0x10000 = text offset).  Match
structure, order of operations, every numeric literal, comparison
operator, sentinel value, mask, shift and loop bound.  A shape-only audit
once let a wrong no-key sentinel through and caused a runaway compositor
reset (the getKey incident, history).  Long runs catch that class of bug:
`test_longrun_stable.sh` (30 000 VBLs).

**Don't invent.**  If a global is unset at run time, find where the
original sets it.  No glue functions, no extra error handling, no
"fixes" to the original's behaviour -- its bugs are kept on purpose
(`== SICKNESS_CRITICAL;`, `dispPlyrChips;` without parentheses,
patSprites read one past its end).  Diagnostic scaffolding is fine while
debugging and must be removed afterwards.

**Odd source shapes are deliberate.**  Unused locals, redundant
re-tests, `goto` loops, declaration order, a missing `return`, `i++` vs
`i = i + 1`: each reproduces the original's instructions, and the
comment next to it says so.  Do not tidy them.  The full catalogue of
shape rules is in history ("Recurring source-shape rules").

**Comments** say how and why, not what; no Ghidra names or addresses.

## Build configurations

ONE shipped configuration, byte-identical.  Test-only defines, passed
through `ALCYON_CPPFLAGS` (rebuild from clean afterwards -- a stale
gated object silently breaks identity):

- `-DSKIP_COPYPROT=1` -- skips the copy-protection call so the game is
  playable under an emulator (it always fails there, even with the
  original disk; not a port bug).
- `-DSKIP_TITLE=1` -- seeds the guestbook so an unattended run reaches
  gameplay.
- `-DSKIP_MIDI=1` -- no Timer-A install, for frame hashing.  Never with
  long runs: the song retry leaks GEMDOS folder buffers until TOS halts.

Gated builds skip the BSS remap, so their layout differs; `globals.c`
pads `sfxBuffer` to 400 in gated builds only, to contain the original's
Dosound overrun.

## Toolchain

The 1985-05-30 Alcyon distribution (`~/Hatari_C/Compiler/Alcyon/alcyon2`:
GEMSTART.O, OSBIND/AESBIND/VDIBIND/GEMLIB, no libf -- `alcyon_link.sh`
links with `UNDEFINED`).  Host tools are rebuilt by
`source/tools/build_toolchain.sh` and are codegen-equivalent to the
period compiler.  cp68 crashes on input paths over ~120 characters.
`cp_asm.s` must be assembled with `as68 -n` (explicit branch sizes).

## Layout

- **Unity units** -- `stx_u1.c`..`stx_u4.c`, `games.c`, `vdistx.c`,
  `midi_seq.c` (via `globals.c`) -- each reproduces one object of the
  original.  Their `#include` order IS the object's function order;
  never reorder.  `tools/stx_units.txt` lists their constituents.
- **`source/parts/`** -- one function body per file, compiled only
  through its unit (`parts/README.md`).
- **Data files** -- `dat_world.c`, `dat_aitables.c`, `dat_anim.c`,
  `dat_parser.c`, ... and `globals.c`, `sprglobs.c`: where a global is
  declared decides the data layout, so declarations stay in place.
- **Headers** -- every function is declared in `include/protos.h`,
  except `rnd()` (`include/rnd.h`, included only by stx_u1/stx_u2/games):
  the original's `stepHead` calls `rnd()` undeclared, and declaring a
  long- or pointer-returning function where it was not visible changes
  that unit's code.  Constants live in `include/enums.h`.
- **Assembly** -- `cp_asm.s` (copy protection), `mq_tick.s` (Timer-A
  sequencer interrupt), `psg_asm.s`, `blkcp_a.s`, `vdistx_a.s`.
- **Host build** (`cd source && make`, `make test`) -- a syntax check
  and the ten unit tests in `source/tests/`; `hostgem.h`, `hostasm.c`
  and `savehost.c` stand in for the ST headers and traps.  The host is
  little-endian: tests that drive real assets write a host-endian copy.

## Things that break byte identity

- **Names**: the linker keeps 8 characters (`_` + 7).  Every external
  name must be unique in its first 7 characters (case-insensitive to be
  safe) and must not match a library symbol.  `tools/renames.tsv` maps
  the 2026-10-05 renames; symbol files show truncated names.
- **The BSS layout** comes from `tools/stx_bss_layout.tsv`, keyed by
  linker name.  Renaming or resizing a BSS global: edit the spec rows,
  build, then `bss_remap.py --gen` (against `build/alcyon/LCP_nobss.PRG`)
  and diff -- it must reproduce the spec.
- **Declared sizes** of BSS arrays set the layout even where the code
  uses less (`bodyFrames` keeps 120 slots for 98 frames).
- **Declarations**: making a declaration visible where it was not can
  change code (see rnd.h).  `sizeof` on an `extern T a[]` array is
  silently the element size in Alcyon -- use a named count.
- **Preprocessor**: cp68 has no `defined()`, and an unclosed `#ifdef`
  silently drops the rest of the file (`ppbalance.py` checks).
- **Data order**: a unit's string literals are pooled in the order c168
  meets them, so moving a declaration can move its string.

## Running it (Hatari)

Use the `hatari` MCP server for interactive work.  TOS104US.ROM, never
EmuTOS; `--machine st --cpulevel 0 --cpuclock 8 --memsize 1`; scripted
launches also need `--confirm-quit off` and an explicit `--fast-forward`.
Launch LCP.PRG directly, not from COMMAND.PRG (the water tank turns
brown).  Gated builds run fine from a mounted (GEMDOS) folder; the
shipped build's copy protection needs the floppy and fails under Hatari
regardless.

Traps worth knowing (details in history, "Running it under Hatari" and
"All ten handleKey key commands work"):
- The load base differs: `--auto` 0x12596, the MCP's `run_program`
  0x12492.  Check it by reading `bitSet32` (1, 2, 4, 8, ...).
- Turbo hides transient state and drops keystrokes; type with turbo off
  and use value-change breakpoints for short-lived flags.
- Song actions (record, organ) hang in an invisible "cannot open" alert
  when run from a mounted host folder -- likely Hatari's GEMDOS
  emulation; test song playback from a floppy image.
- `PLAY GAME` reaches the minigame menu after a long walk; press the
  digit once (keys buffer and replay).

## Ghidra

`tools/sync_ghidra_names.sh` pushes names (Ghidra must be CLOSED: it
holds an exclusive lock); `sync_ghidra_names.sh verify` is read-only.
For one symbol with Ghidra open, the GhidraMCP plugin on :8089 takes
`POST /rename_data {"address": ..., "newName": ...}`.  BSS addresses come
from the spec, not lcp_sym.68k; expand truncated names before pushing.
Ghidra was re-synced to the 2026-10-05 names on 2026-10-05, through the
plugin with Ghidra open: every port symbol whose Ghidra name was an old
port name now carries the new one (altScreen stays unlabelled).  Names
Ghidra's own analysis gave were left alone.  For a cell with no defined
data, `analyze_data_region` reports a `DAT_` name even when a label
exists; `rename_or_label` answering "already exists" is the check.

## Open questions and decisions

- **altScreen's base** (+0x1FF vs +0x200): undecidable from the binary;
  the maintainer decided to keep +0x1FF (2026-10-04).  Do not re-propose
  without new external evidence.
- **sfxBuffer's size** (56 with an overrun, or a 400-byte struct):
  undecided, behaviourally identical; Music Studio cannot settle it.
- **loopStack's 488-byte gap**: harmless, open.
- Two typed-command rows can never fire (HELLO, `MESSY IS HOME`): the
  1985 data's quirks, documented in history.
