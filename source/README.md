# LCP.PRG -- C source reconstruction

K&R C source for **Little Computer People** (Activision, 1985, Atari ST),
recovered from the shipped binary.  It compiles under **Alcyon C 4.14**
to a program that is **byte-identical** to `DATA/LCP_STX.PRG`, the
uncracked 1985 build extracted from the Pasti image in the repo root:
123 352 bytes, MD5 `eae52d14023b51d7ac459a90d37eed10` (text 104 156,
data 12 260, bss 187 450).

The same sources also build with a modern host compiler (`-DHOST`) as a
syntax check and for the unit tests.

[../CLAUDE.md](../CLAUDE.md) is the short working guide;
[../docs/history.md](../docs/history.md) is the full record of how
identity was reached and why the source looks the way it does.

## Style

- **K&R** definitions; every function is declared, without
  parameters, in `include/protos.h`.
- **`short`** is the 16-bit int, **`long`** 32-bit, **`char`** signed
  8-bit.  `BOOL16` is a `short` holding `YES`/`NO`.
- **No `enum`** -- Alcyon has none.  Constants are `#define`s in
  `include/enums.h`; timers and counts in decimal, bit masks in hex.
- **Names are readable lowerCamelCase**, unique in their first seven
  characters because the linker keeps only eight (`_` + 7).
  `tools/renames.tsv` maps them to the names older commits used.
- **Comments say how and why**, not what.  Odd shapes -- unused locals,
  redundant tests, `goto` loops, a missing `return` -- reproduce the
  original's instructions and carry a comment saying so; do not tidy
  them.

## Layout

The file structure reproduces the original's object partition, which
was recovered from the binary: a `bsr` from A to B proves everything
between them is one object.  Each object is built as a **unity
translation unit** that `#include`s its functions in the original's
order.

```
source/
├── stx_u1.c .. stx_u4.c  unity units for four of the game's objects;
├── games.c               the minigame suite (one object of its own);
├── vdistx.c              the VDI binding module, one trap dispatcher;
├── midi_seq.c            the MIDI sequencer, compiled through globals.c.
│                         The #include order of a unit IS its function
│                         order.  tools/stx_units.txt lists constituents.
├── parts/                one function body per file, placed by a unit
├── *.c                   the remaining constituents (ai.c, tick.c,
│                         sprites.c, ...), each wholly inside one unit
├── dat_*.c, globals.c,   initialized and zeroed data.  A unit's .data
│   sprglobs.c            comes out in declaration order and its strings
│                         in the order c168 meets them, so declarations
│                         stay where they are.
├── cp_asm.s              copy protection (Activision's shared routine)
├── mq_tick.s             Timer-A interrupt driving the sequencer
├── psg_asm.s, blkcp_a.s, the other hand-written assembly
│   vdistx_a.s
├── include/              types, structs, constants, globals, protos
├── hostasm.c, savehost.c host stand-ins for the assembly and the
│                         GEMDOS/BIOS traps; never part of the ST build
├── tools/                build, verification and test scripts
│                         (tools/README.md)
└── tests/                host-side unit tests
```

## Building and testing

```
tools/run_all.sh            # ~4 min, includes the emulator tests
tools/run_all.sh --quick    # ~10 s, build + host only
```

`run_all.sh` checks byte identity on a clean shipped build, runs the
host build and unit tests, then the Hatari runtime tests on a gated
build, and restores the shipped build afterwards.

The pieces:

```
tools/alcyon_build.sh && tools/alcyon_link.sh   # the ST binary
python3 tools/prg_diff.py                       # byte identity
python3 tools/reloc_audit.py                    # variable references
make && make test                               # host build, unit tests
```

The Alcyon toolchain runs natively: `tools/build_toolchain.sh` rebuilds
cp68/c068/c168/as68/link68 from Thorsten Otto's cleaned-up sources, and
it is codegen-equivalent to the 1985 compiler, so a difference in output
is a difference in source.  The link ends with `tools/bss_remap.py`,
which lays BSS out from the checked-in `tools/stx_bss_layout.tsv`; the
reference binary is not read at link time.

The test-only defines `SKIP_COPYPROT`, `SKIP_TITLE` and `SKIP_MIDI` make
an unattended emulator run reach gameplay.  Those builds are not
byte-identical by construction; rebuild from clean before checking
identity again.

### Unit tests (`make test`)

| Test                 | Verifies                                              |
|----------------------|-------------------------------------------------------|
| `linktest`           | every object links, no unresolved symbols             |
| `hyber_test`         | HYBER save file round-trips byte for byte             |
| `letter_test`        | LETTER.TXT nibble decoder and line index              |
| `parser_test`        | typed sentences reach the right action                |
| `vdi_pb_test`        | VDI parameter blocks match the GEM ABI                |
| `assets_test`        | OBJECTS, SPRITES, BODY.LCP and PEx.LCP headers        |
| `scn_test`           | HOUSE.SCN decompression                               |
| `sounds_test`        | SOUNDS.LCP block loader                               |
| `sim_test`           | the one-second needs/clock/mood tick                  |
| `sprite_test`        | body sprite composition                               |
| `sprite_golden_test` | a composed frame against `tests/reference/`           |

All of them read the real 1985 files in `../DATA/`.  The host is
little-endian and the loaders read big-endian length fields raw, so
tests that drive a real asset write a host-endian copy first.  The
parser's table walk depends on Alcyon narrowing `0xff` to a signed
char, which clang does not do; the full parser is therefore checked
under the emulator by `tools/test_actions.sh`.

## Open questions

The binary cannot settle these; both readings produce the same bytes.

- **`sfxBuffer`'s size**: 56 bytes, overrun by the longer sound effects,
  or a 400-byte object with two status words inside it.
- **`loopStack`'s 488-byte gap**: a larger array, or an unused global.
- **`altScreen`'s base**: `+0x1FF` or `+0x200`; kept at `+0x1FF`.

See docs/history.md for the evidence on each.
