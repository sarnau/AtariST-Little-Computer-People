# LCP.PRG — C source reconstruction

Idiomatic K&R C reconstruction of **Little Computer People** (Activision,
1985, Atari ST) from Ghidra decompilation of `LCP.PRG`. Target compiler
is **Alcyon C 4.14** (Digital Research C for CP/M-68K 1.2, as shipped
with the Atari ST Developer Kit).

At a glance:

- **The build is BYTE-IDENTICAL to `DATA/LCP_STX.PRG`** — 123 352
  bytes, MD5 `eae52d14023b51d7ac459a90d37eed10`, text 104 156 / data
  12 260 / bss 187 450
- 268 game functions below the DRI library boundary, 185 of them as
  single-function bodies under `parts/`
- 64 top-level `.c`, 13 `dat_*.c` data units, 4 `stx_u*.c` unity
  translation units, 5 hand-written `.s` files
- 128-byte HYBER save file loads directly into `PLAYER` struct
- 8 test programs on the host, 4 Hatari-driven runtime tests, one
  command to run everything (`tools/run_all.sh`)

See [STATUS.md](STATUS.md) for the full per-function port ledger and
[../CLAUDE.md](../CLAUDE.md) for the working notes.

## Fidelity target

**Byte-exact rebuild, and it is achieved.**  `source/tools/prg_diff.py`
reports BYTE-IDENTICAL against `DATA/LCP_STX.PRG`, the uncracked
shipped 1985 binary extracted from the protected Pasti image.

(This section said "**Not** byte-exact rebuild ... byte-exact
validation is a *later* pass" until 2026-09-06.  That was true when it
was written and had been wrong since 2026-09-03, which is exactly the
kind of stale claim worth correcting rather than leaving to mislead.)

Getting there needs both toolchains:

1. **Alcyon C 4.14** produces the shipped binary.  Byte identity is
   checked on every change — the source is not merely "equivalent",
   it emits the same instructions in the same order.
2. **A modern host compiler** (`clang`/`gcc`) with `-DHOST` builds the
   same sources so pure-logic subsystems can be tested without an
   emulator.  Keeping clang and Alcyon reading one source has its own
   set of constraints; CLAUDE.md's "host build" section lists them.

Two things are NOT byte-identical by construction: the test
configurations gated behind `-DSKIP_TITLE`, `-DSKIP_COPYPROT` and
`-DSKIP_MIDI`.  Those exist so an unattended emulator run can reach
gameplay at all, and `alcyon_link.sh` refuses to remap BSS when they
are set.  Rebuild clean before checking `prg_diff` again.

## Style bible

- **K&R** function definitions. No ANSI prototypes at the point of
  definition; extern declarations use empty parens.
- **`short`** = 16-bit int (Alcyon default), **`long`** = 32-bit,
  **`char`** = 8-bit signed. `BOOL16` typedef → `short` with
  `YES`/`NO` (= 1 / 0).
- **No `enum`** — Alcyon 4.x doesn't recognise it. All symbolic
  constants are `#define`s in `include/enums.h`.
- **Variable declarations at block top only.**
- **Globals** declared once in `globals.c`, extern'd via
  `include/globals.h`. Names preserved from Ghidra one-to-one so
  cross-referencing the decompile stays trivial.
- **Structs** in `include/structs.h` mirror the Ghidra layouts
  verbatim; `PLAYER` (128 bytes) loads directly from `HYBER`.
- Every ported function carries the Ghidra plate comment plus an
  `addr:` line naming the original symbol.
- Line width ~78 col, tabs = 8, K&R braces.

## Layout

The file structure is NOT free: it reproduces LCP_STX's own object
partition, recovered from the binary (see CLAUDE.md, "Recovering
LCP_STX's C sources").  A `bsr` from A to B proves everything between
them is one object, which bounds the original's ~7 huge game objects.
The port therefore builds **unity translation units** that `#include`
their constituents in the original's order.

```
source/
├── include/            63 headers -- types, enums, structs, globals,
│                       trap wrappers.  obdefs1.h wraps the DK header,
│                       which has no include guard of its own.
├── parts/              185 single-function bodies.  One function per
│                       file so a unity unit can place it at its exact
│                       address; several port .c files straddled two
│                       LCP_STX objects and had to be split this way.
├── stx_u1.c ... u4.c   the unity units.  Their #include ORDER IS the
│                       object's function order -- LCP_STX did not
│                       group by source file, so aleisure's functions
│                       alone run from 0xe338 to 0x12ca0.
├── dat_u*.c            per-object data files.  A unit's .data comes
│   dat_games*.c        out in source order and its string literals in
│                       the order c168 met them, so where a global is
│                       declared is itself evidence.
├── globals.c           storage for every extern, plus data tables
├── games.c             the whole minigame suite -- one LCP_STX object
├── midi_seq.c          MIDI sequencer (descends from Music Studio's
│                       player: mq_bust is 73.8% byte-identical to it)
├── cp_asm.s            the copy protection, hand assembly.  97.2%
│                       byte-identical to The Music Studio -- it is
│                       Activision's routine, not LCP's.
├── mq_tick.s           Timer-A MFP ISR, psg_asm.s, blkcp_a.s,
│                       vdistx_a.s -- the other hand-written asm
├── vdistx.c            the VDI binding module as LCP_STX links it:
│                       ONE trap dispatcher, ONE parameter block
├── hostasm.c           host stand-ins for the .s files; savehost.c
│   savehost.c          for the GEMDOS/BIOS traps.  alcyon_build.sh
│                       skips both BY NAME so they cannot ship.
├── tools/              build, verification and test scripts
└── tests/              host-side test programs
```

## Building

### Host build (for tests / development)

Requires a C89/C99 compiler (`cc`, `clang`, `gcc`) with `-DHOST`. From
the `source/` directory:

```
make host
```

This links the whole game against the host GEMDOS/XBIOS shims
(`savehost.c`, `hostasm.c`, stubbed traps in `osbind.h`).  It **won't
run interactively** on a non-ST host, but it type-checks every source
file and lets the pure-logic subsystems be tested.  `main()` becomes
`lcp_main()` under `-DHOST` so the tests can supply their own.

### Alcyon (target) build

```
make alcyon                 # = tools/alcyon_build.sh + alcyon_link.sh
```

Alcyon runs **natively**, not under emulation: `tools/build_toolchain.sh`
rebuilds `cp68`/`c068`/`c168`/`as68`/`ar68`/`link68`/`relmod` from
Thorsten Otto's cleaned-up sources into `~/Hatari_C/hatari-c/bin`.  That
rebuilt toolchain is **codegen-equivalent** to the one that built
LCP_STX — running alcyon2's own 1985 `C168.PRG` under Hatari on the
same input emits the same instructions — so a difference in output is
a difference in SOURCE, never in the compiler.

The link ends with `tools/bss_remap.py`, because lo68 and the 1985
linker pack `.comm` blocks at different offsets.  Text, data and the
relocation stream come out identical without it; BSS addresses do not.
The original allocation is checked in as `tools/stx_bss_layout.tsv`
and resolved against a second symbols link — **the reference binary is
not read at link time.**

## Testing

Everything, in the order that works:

```
tools/run_all.sh            # ~3.5 min, includes the emulator tests
tools/run_all.sh --quick    # ~10 s, build + host only
```

It runs the byte-identity checks on a clean SHIPPED build, then the
host build and unit tests, then the runtime tests on a GATED one
(`-DSKIP_TITLE -DSKIP_COPYPROT`), and restores the shipped build
afterwards even on failure.  The two build configurations are not
interchangeable and leaving the wrong one behind makes the next run
report a failure that is not real, which is why this exists.

Ten host-side test programs live in `tests/`, and run on their own
with:

```
make test
```

| Test                | Verifies                                                |
|---------------------|---------------------------------------------------------|
| `linktest`          | Every module compiles + links, no unresolved symbols     |
| `hyber_test`        | HYBER save-file round-trip: load → mutate → save → reload |
| `letter_test`       | `LETTER.TXT` nibble-encoded template decoder             |
| `parser_test`       | NLP parser smoke test: `"please play a game"` reaches `ACTION_PLAY_A_GAME`.  Note it gets there on `play`+`game` alone — `please` is `vwd_tab[0]`, and chk_encm reads chk_vwd's 0 as "unrecognised", so the politeness word contributes nothing and costs +4 priority |
| `vdi_pb_test`     | VDI parameter-block layout matches GEM ABI                 |
| `assets_test`     | `OBJECTS` + `SPRITES` big-endian header decode             |
| `scn_test`        | `HOUSE.SCN` compressed screen decompression                |
| `sounds_test`     | `SOUNDS.LCP` sound-effect table load                       |

All tests read real 1985 data files from `../DATA/` and assert
byte-exact matches against reference dumps.

Two things to know before touching them.  **The host is little-endian
and the loaders are not**: sf_sl and al_loal read their length fields
with raw two-byte `fr_read`s, so 34 arrives as 8704 and the file walk
is lost after the first block.  That is faithful ST code, so
`t_sounds.c` and `t_assets.c` write a HOST-ENDIAN copy of the asset --
same payloads, only the length fields swapped -- and put the loader's
LOGIC under test.  And **not everything is checkable here**: chk_encm
walks `g_ew2a` until `table[0] == 0xff`, which Alcyon narrows to a
signed char so the sentinel matches and clang does not, so on the host
that walk runs off the table.  `t_parser.c` reports that case instead
of asserting it, and it is why the FULL parser can only be checked
under the emulator, by `tools/test_actions.sh`.

## Adding a new subsystem

1. **Find the Ghidra symbol.** Every port names its source function
   in an `addr:` line at the top of the docstring — search the
   Ghidra decompile tree by that name.
2. **Preserve the plate comment verbatim.** Anything Ghidra added
   about caller sites, calling convention, or side effects goes
   above the function unchanged. Add commentary below the plate,
   not inside it.
3. **Match the frame, not just the behaviour.**  Compare the
   `link #-N` prologue FIRST: it says exactly how many locals the
   function really has, and local offsets are assigned in
   DECLARATION order, so a fn_diff pins the declaration list and its
   ORDER exactly.  a_wandi needed an unused local the port lacked;
   a_driwa's order is rnd, counter, last_pick, pick and it never
   initialises last_pick.  Preserve such things as written.
4. **Add extern declarations to `globals.h`** for any global you
   touch; storage goes in `globals.c` grouped by subsystem.
5. **Wire up a host test** when possible. If the function loads
   or decodes a real 1985 file format, that format has a fixture
   under `../DATA/` — add a `tests/t_foo.c` that exercises the round
   trip and asserts byte-exact.
6. **Update `STATUS.md`** with the new port and its status.

## Known gaps

**One**, and the binary cannot close it.  `g_sfDoB`'s declared size is
an inference: a declared array size never reaches the codegen, so 56
came only from the distance to the next referenced cell.
`g_sfDoB..g_srlgb` is exactly 400 bytes, so the original may instead
have had one 400-byte object with `g_sfdos`/`g_sfdoc` as fields inside
it — in which case there is no overrun at all.  Both readings are
behaviourally identical and produce the same bytes.  The Music Studio
disk was checked and rules itself out: `sf_irqp` shares ZERO of its
456 bytes with it.  See CLAUDE.md, "Is the 56 real?".

Everything the older version of this section listed is CLOSED, and
each was wrong in an instructive way:

- **`cp_main` is not "intentionally stubbed"** — it is fully recovered
  as hand assembly in `cp_asm.s`, byte-identical, and 97.2%
  byte-identical to THE MUSIC STUDIO, so it is Activision's shared
  protection routine rather than LCP's own code.  It genuinely does
  not pass under an emulator, but the control is decisive: the
  ORIGINAL 1985 binary off the Pasti image fails identically.  Build
  with `-DSKIP_COPYPROT=1` to play.
- **The stairs "regression" was a false negative** in a harness that
  warped the resident past the walk establishing stair state.  Real
  play walks stairs correctly; the harness is gone.
- **.SNG playback works.**  The Timer-A ISR installs and ticks through
  a 36 000-VBL run; the resident simply had not chosen the record
  player autonomously.  Ask it to play a record.

CLAUDE.md's corresponding section is titled "Issue log — ALL CLOSED".

## History

- **v0**: 5 initial ports (main, sim, save, HYBER load, parser stub).
- **v1**: All 45 action handlers ported; save round-trip verified.
- **v2**: File-format loaders complete; 8 host tests all passing.
- **v3**: Sound subsystem (MIDI + PSG + SFX IRQ) fully ported.
- **v4**: Music Studio 2.0 provenance for `.SNG` / `.ORG` files
  documented (byte-diff of 9/11 songs identical to the Music
  Studio distribution disk).
- **v5**: Sprite buffer sizes matched to ROM slots exactly
  (`body_buf[120][168]`, `pex_buf[66][168]`,
  `sp_mbuf[14000]`); introduced `LCP_BODY_FRAME_SIZE` /
  `LCP_BODY_SHAPE_SIZE` / `LCP_BODY_DEST_WORDS` constants
  replacing scattered `168` / `84` literals.  4 real OOB bugs
  fixed (`pst_arr`, `usr_buf`, `g_pcdrp`, `g_ppdrp`) via targeted
  audit of every port array vs. its Ghidra ROM slot.
- **v6**: `tv_boul` / `tv_patl` v_pline point-buffer size fix (was
  reading 1 uninitialised endpoint per call, corrupting the
  compositor over minutes and crashing the game during the
  computer-typing session).  Long-run stability test now passes
  for 36 000 VBLs / 10 real minutes with 0 bus errors.  Test
  harness: `--auto` load address fix, TOS boot-probe (`$fc0174`)
  filter in `run_hatari.sh`.

  **Correction (2026-09-06):** this entry also claimed
  "test_actions / test_keyboard / test_saveload all clean".  They
  were not clean; they did not RUN.  test_actions and test_keyboard
  built with `-DTEST_ACTIONS` / `-DTEST_KEY` to switch on in-game
  harnesses that the LCP_STX restructuring had already removed, so
  the flags compiled to nothing and both scripts exercised no hook
  while still exiting 0.  test_saveload looked for HYBER in a
  `data/` subdirectory that does not exist and bailed out at its
  setup check.  All three were rewritten on `hatari_probe.sh`
  (2026-09-06) and now assert against the game's own globals:
  11/11, 33/33 and 7/7 respectively.


- **v7 (2026-09-03)**: **BYTE IDENTITY REACHED.**  The build reproduces
  `DATA/LCP_STX.PRG` exactly — MD5 `eae52d14023b51d7ac459a90d37eed10`.
  Getting there was not more porting but recovering the original's
  STRUCTURE: its object partition (from which calls are `bsr` and which
  `jsr`), the function order inside each object, its data declaration
  order, and its `.comm` allocation.  cp_main turned out to be hand
  assembly; the VDI layer to be one dispatcher where the port had
  three; and ~30 recurring source-shape rules had to be recovered
  one site at a time.  See CLAUDE.md.
- **v8 (2026-09-06)**: verification caught up with the code.  A crash
  that killed every long run at VBL 16983 was diagnosed as `sf_irqp`
  overrunning `g_sfDoB` — and it also explained the missing dog and the
  `introSeq` flag that never cleared, both of which had been chased as
  separate bugs.  Two dead test scripts were rewritten to assert
  against the game's own globals, four stale diagnostics deleted, and
  `tools/run_all.sh` added so the whole suite is one command.  All five
  minigames, all ten key commands and all 31 reachable typed commands
  are now verified end to end.

See [STATUS.md](STATUS.md) for the current port ledger.
