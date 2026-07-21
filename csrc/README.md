# LCP.PRG — C source reconstruction

Idiomatic K&R C reconstruction of **Little Computer People** (Activision,
1985, Atari ST) from Ghidra decompilation of `LCP.PRG`. Target compiler
is **Alcyon C 4.14** (Digital Research C for CP/M-68K 1.2, as shipped
with the Atari ST Developer Kit).

At a glance:

- ~200 functions ported across ~54 modules
- 128-byte HYBER save file loads directly into `PLAYER` struct
- 45 `do_action()` handlers implemented (no game-logic stubs remain)
- 9 original data-file formats decode byte-exactly
- 8 host-side tests + 6 Hatari-driven regression tests, all passing
- Sprite buffers sized to match ROM slots exactly (`body_buf`,
  `pex_buf`, `sp_mbuf`) with `LCP_BODY_FRAME_SIZE` /
  `LCP_BODY_SHAPE_SIZE` / `LCP_BODY_DEST_WORDS` constants for the
  three sprite pipeline dimensions

See [STATUS.md](STATUS.md) for the full per-function port ledger.

## Fidelity target

**Not** byte-exact rebuild. Goal is source that:

1. Compiles under Alcyon C 4.14 and produces a functionally-equivalent
   game — same UX, same save-file compatibility, same VDI/XBIOS/GEMDOS
   call pattern.
2. Also compiles under a modern host toolchain (`clang`/`gcc`) via the
   `HOST` shim so subsystems can be exercised without Hatari.

Byte-exact validation is a *later* pass on individual hot files.

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

```
csrc/
├── include/
│   ├── types.h         BOOL16, YES/NO, size types
│   ├── enums.h         all symbolic constants (#define)
│   ├── structs.h       PLAYER, PSG_ENVELOPE, MFDB, WORD_TO_ACTION
│   ├── globals.h       extern of every global
│   ├── osbind.h        GEMDOS/XBIOS/BIOS trap wrappers (host + target)
│   └── st_io.h         raw hardware I/O helpers
├── globals.c           storage for all extern globals + data tables
├── main.c              entry point (init_vdi + main loop)
├── sim.c               game_simulate_one_second (clock + needs)
├── ai.c, ai_random.c   9-priority AI + random-event picker
├── actions.c           do_action() dispatch + 45 handlers
│   ├── actions_bathroom.c, actions_food.c,
│   │   actions_doors.c,    actions_games.c,
│   │   actions_house.c,    actions_idle.c,
│   │   actions_leisure.c,  actions_letter.c,
│   │   action_simple.c
│   └── action_stubs.c  (empty — retained for symmetry)
├── parser.c            NLP command parser
├── vocab_data.c        160-word vocabulary + 33 action-match rules
├── save.c              HYBER load/save (128-byte format)
├── save_host.c         host GEMDOS shim
├── letter_load.c       LETTER.TXT nibble decoder + template loader
├── assets.c            OBJECTS / SPRITES / BODY.LCP / PEx.LCP loaders
├── cards.c             poker card graphics loader
├── sprites.c           45-entry sprite table + head/body updater
│   ├── sprglobs.c, sprender.c, sprhead.c, sprload.c
├── movement.c, walk.c  position table + pathfinding
├── render.c            screen_render_8hz frame driver
│   ├── renderx.c, renderf.c, gfx_prim.c, tvanim.c
├── (VDI/AES trap wrappers now come from Alcyon gemlib)
├── sound.c             song_play + soundeffect_* dispatch
│   ├── midi_seq.c      MIDI sequencer control
│   ├── psg_io.c        YM2149 register writers
│   ├── sfx_irq.c       8Hz Dosound tick
│   ├── psgfreq.c       132-entry PSG tone-period LUT
│   └── tools/dk/mq_tick.s  Timer-A MFP ISR (byte-faithful asm)
├── games.c, cards.c    mini-games (poker/blackjack/anagram/war/word-puzzle)
├── clock.c, calendar.c, keyboard.c, random.c,
│   dog.c, delivery.c, events.c, alerts.c, health.c
├── tvanim.c            LCP's on-screen computer/TV animations
├── init.c              cs_mvIn move-in cutscene + boot-time helpers
│                       (also hosts TEST_ACTIONS / TEST_KEY /
│                       TEST_STAIRS #ifdef hooks for regression tests)
└── tests/              host-side smoke tests (see below)
```

Subsystem layout mirrors the Python port under `../lcp/`. When Ghidra
output is ambiguous, the Python behaviour is authoritative — it's the
executable reference.

## Building

### Host build (for tests / development)

Requires a C89/C99 compiler (`cc`, `clang`, `gcc`) with `-DHOST`. From
the `csrc/` directory:

```
make host
```

This links the whole game against the host GEMDOS/XBIOS shims (`save_host.c`,
stubbed traps in `osbind.h`).  It **won't run interactively** on a
non-ST host but it lets every non-graphical subsystem be exercised.

### Alcyon (target) build

Under Hatari + TOS + Atari ST Developer Kit:

```
make alcyon
```

This drives Alcyon's `CP68` (preprocessor), `C068` (compiler), and
`AS68` (assembler) in sequence, linking with `LO68` to produce a
runnable `LCP.PRG`.

The `Makefile` documents both toolchains side-by-side; the primary
`CFLAGS` gate on the `HOST` define.

## Testing

Eight host-side smoke tests live in `tests/`:

```
make test
```

| Test              | Verifies                                                  |
|-------------------|-----------------------------------------------------------|
| `linktest`        | Every module compiles + links, no unresolved symbols       |
| `hyber_test`      | HYBER save-file round-trip: load → mutate → save → reload  |
| `letter_test`     | `LETTER.TXT` nibble-encoded template decoder               |
| `parser_test`     | NLP parser: `"please play a game"` → `ACTION_PLAY_GAME`    |
| `vdi_pb_test`     | VDI parameter-block layout matches GEM ABI                 |
| `assets_test`     | `OBJECTS` + `SPRITES` big-endian header decode             |
| `scn_test`        | `HOUSE.SCN` compressed screen decompression                |
| `sounds_test`     | `SOUNDS.LCP` sound-effect table load                       |

All tests read real 1985 data files from `../data/` and assert
byte-exact matches against reference dumps.

## Adding a new subsystem

1. **Find the Ghidra symbol.** Every port names its source function
   in an `addr:` line at the top of the docstring — search the
   Ghidra decompile tree by that name.
2. **Preserve the plate comment verbatim.** Anything Ghidra added
   about caller sites, calling convention, or side effects goes
   above the function unchanged. Add commentary below the plate,
   not inside it.
3. **Copy variable names from the decompile.** If the decompile
   calls it `iVar3`, keep it as `iVar3` on the first pass; only
   rename once behaviour is understood and the name reads clearly
   in context.
4. **Add extern declarations to `globals.h`** for any global you
   touch; storage goes in `globals.c` grouped by subsystem.
5. **Wire up a host test** when possible. If the function loads
   or decodes a real 1985 file format, that format has a fixture
   under `../data/` — add a `tests/foo.c` that exercises the round
   trip and asserts byte-exact.
6. **Update `STATUS.md`** with the new port and its status.

## Known gaps

See [CLAUDE.md](../CLAUDE.md)'s "Known open issues" section for the
live list.  Highlights:

- `cp_main` copy protection is intentionally stubbed (the ROM
  routine can't run under Hatari — flock + XOR-decrypt + FDC
  signature read — documented in `csrc/stubs.c`).
- Stair test harness now runs end-to-end but surfaces a
  game-behavior regression: LCP descends by falling through floors
  rather than engaging stair mode.  Manual play in the same
  scenario worked; automated harness caught what the manual test
  missed.
- Music playback in the production build (no `-DSKIP_MIDI=1`) not
  yet verified end-to-end.

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
  harness: TEST_STAIRS hook in `cs_mvIn`, `--auto` load address
  fix, TOS boot-probe (`$fc0174`) filter in `run_hatari.sh` --
  test_actions / test_keyboard / test_saveload all clean.

See [STATUS.md](STATUS.md) for the current port ledger.
