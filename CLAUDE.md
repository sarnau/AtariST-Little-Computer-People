# Working notes for Claude / AI assistants on this repo

This is a **faithful C port** of the 1985 Atari ST game *Little Computer
People* (Activision).  The original LCP.PRG is disassembled in Ghidra
on the maintainer's machine and is the ground truth for every function,
initialization step, and control-flow decision.  The C source in
`source/` compiles under Alcyon C 4.14 (K&R) and runs on Hatari.

## Original toolchain (identified 2026-08-31)

The original LCP.PRG was built with **Alcyon C (DRI CP/M-68K C
compiler) as shipped in the official Atari ST Developer's Kit** — the
same Alcyon C 4.14 toolchain the port uses.  Fingerprinted from
`DATA/LCP_ORG.PRG` against the compiler collection in
`~/Hatari_C/Compiler/`:

- **Startup**: instruction-for-instruction the Atari DK
  `DISK_2/LINKER/GEMSTART.S` — including the "constant STACK value"
  fragment with **STACK=$8000 (32 KB stack)**, `MINSTACK=$1000`,
  `FUDGE=$200`, the `ext.w` command-length sequence, and the
  `tst.l (a7)+ / Pterm($4C)` exit.  The older `alcyon2` GEMSTART
  variant (`andi.l #$ff`, no stack models) does NOT match — it is
  specifically the DK revision.
- **Runtime library**: the DK's GEMLIB, wildcard `_main` (`xwmain.c`)
  linked in (strings `: unmatched quote`, `Cannot open/append/create`,
  `: No match`, `Stack Overflow`).  No LIBF traces — no floating point.
- **Control experiment**: the port's `source/build/alcyon/LCP.PRG`
  startup is byte-identical to the original modulo relocated absolute
  addresses.
- Ruled out: Pure C and Lattice 5.60 (postdate the game), Laser C /
  Megamax and Mark Williams C (different crt0 + runtime strings),
  early Lattice/Metacomco (no DRI-style wildcard `_main`).

## The rule: Ghidra-faithful means literal-faithful

Before writing or modifying any code path, verify it against the
original in Ghidra.  Match structure, order of operations, identifier
shape — **and every numeric literal, comparison operator, and sentinel
value**.

A shape-match audit ("both check `if (key)`, both set `tx_sctm=160`")
is NOT sufficient.  A single-token divergence can produce a silent
runaway bug:

**The getKey sentinel incident (2026-07-19)** — tick.c compared
getKey()'s result against the wrong no-key sentinel, so the "key
received" branch fired every tick, resetting `tx_sctm = 160`
continuously and locking the split-copy compositor.  Visible screen
corruption at ~11 500 VBLs, TOS bus error shortly after.  A
shape-audit had passed; literal-audit had not.  CAUTION (2026-09-01):
the sentinel POLARITY differs between the two game revisions -- in
DATA/LCP_ORG.PRG the tick-side test is `key != 0` (byte-verified),
while the other Ghidra image used `!= -1`.  Always verify against the
chosen reference binary, not against this anecdote.

**Every audit must diff:**
- Numeric literals (`0`, `-1`, `0xFF`, `160`, `27`, hex constants).
- Comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`) — check
  operand values, not just the operator.
- Sentinel return values.  When Ghidra tests `foo() != foo_sentinel`,
  the port MUST test against the same sentinel, not a substitute like
  `0` or `NULL`.
- Bitmasks and shift counts.
- Loop bounds (`< N` vs `<= N` vs `< N+1`).

**Testing must be long enough to catch runaways.**  A corruption
caused by a per-tick reset takes ~3 min of gameplay to become
visible; it will not surface in a 25-second smoke test.  Regression
checks that exercise long-running gameplay (≥ 15 000 VBLs under
`--auto`) and diff a mid-run frame against an early-run frame catch
this class of bug in seconds — see
`source/tools/test_longrun_stable.sh`.

## Don't invent

- If a global reads NULL at runtime, find where the **original** sets
  it (init sequence, IRQ handler, asset loader) and port that.
  Don't invent a wrapper that sets it from `main()`.
- Don't invent glue functions like `init_compositing_screens()` or
  side pointers like `alt_screen_ptr` just because a global was
  uninitialized.  The 1985 code populated it somewhere; find that
  call site.
- Don't add error handling, fallbacks, or validation for scenarios
  that can't happen.  Trust internal code and framework guarantees.
- If a Ghidra decompile shows `if (x < 0)`, the port has `if (x < 0)`.
  Not `if (x <= 0)`, not `if (x == -1)`.

## Diagnostics are the exception

Sprinkling `gemdos(9, "…")` markers, dump-to-low-RAM writes, or
Hatari conditional breakpoints to localize a crash is fine.  Once
the diagnosis is done, **remove the test code**.  Ghidra-faithful
source ships; ad-hoc debug scaffolding does not.

## Launching / running the port

- **Always use the Hatari MCP server for interactive testing** (AI
  assistants: registered as `hatari` in local MCP config; source in
  `~/Downloads/Hatari/`).  It drives a patched Hatari over a socket:
  keyboard/mouse injection, inline screenshots, resolution & CPU
  clock control, floppy/GEMDOS-drive mounting, and debugger
  memory/register reads.  Batch regression scripts in `source/tools/`
  keep their own Hatari invocations.

- LCP.PRG must be launched **directly** — from Hatari's `--auto`
  option or double-clicked from the GEM desktop.  Launching via
  COMMAND.PRG (Atari's shell) leaves the workstation in a state
  where subsequent `Setscreen` calls invalidate VDI line-attribute
  state, and `vsl_color` silently falls back to pen 15 (dark brown).
  This causes the water tank to render brown instead of blue — see
  the comment above `sc_sdtb` in `source/gfx_prim.c`.
- `SKIP_TITLE` no longer exists: the interactive title screen was an
  other-revision feature; st_titl is the ROM's defaults stub.  Test
  builds may still use `-DSKIP_MIDI=1` (skips the kept Timer-A
  install for frame-hash determinism).  `-DFAITHFUL=1` selects the
  byte-identical configuration (ROM minigame stubs, empty mq_intim).

## Key project layout

- `source/*.c` — the port itself.
- `source/include/*.h` — types, enums, struct layouts.
- `source/tools/` — build & test scripts (Alcyon build, Hatari-driven
  regression tests, symbol lookup helpers).
- `source/tests/` — host-side unit tests (compile under host cc, not
  Alcyon; not part of the shipped binary).

## Ghidra ↔ port cross-reference

`source/tools/ghidra_globals_map.md` documents every port global that
has a Ghidra counterpart with a different (usually longer) name.  To
push the map to the Ghidra project, run:

    source/tools/apply_ghidra_renames.sh

This regenerates a TSV via `gen_ghidra_rename_tsv.py`, then POSTs to
Ghidra's HTTP server at `localhost:8089/run_script` to invoke
`~/ghidra_scripts/RenameLcpGlobals.java`.  Prereqs: Ghidra open with
LCP.PRG loaded; `list_data_symbols.java` has been run at least once.

Struct name/field syncing uses the same HTTP mechanism via
`~/ghidra_scripts/RenameGhidraStructs.java` and a
`lcp_struct_rename_map.tsv`.

## Byte-fidelity campaign (2026-09-01)

`source/tools/verify_bytes.py` proves each port function byte-identical
to `DATA/LCP_ORG.PRG` (relocations and PC-relative displacements
wildcarded); `fn_diff.py NAME [orig_hex]` prints side-by-side
disassembly for recovering exact C.  Workflow: build, link, relink
without `-s` to `lcp_sym.68k`, run verify, fix, repeat.

Status: **303 matched / 57 divergent, 91.6% of the original text
proven byte-identical.**  Recovered so far: the ROM's own osbind.h
shapes (GEMDOS padded to opcode+3 args, NO argument casts; XBIOS
per-site), the od_* frame-id global tables (data 0x11758-0x1177e and
0x1200a-0x12026), WORD_TO_ACTION 14-byte rows, screen alignment
`(scrbufA+0xFF)&~0xFF` with alt screen at +0x8000, the shipped
(unsigned char)->ext.w Alcyon miscompile in sp_lcpf, several real
logic recoveries (chk_actT hunger gate, a_kitcc chew loop + g_actif,
dg_mvAn eating-sprite gate, ev_ansP od_med1, sf_sl is open+close
only in the ROM -- see the SOUNDS.LCP crash note under Known open
issues), and lcp_save/lc_load/sgPlay call shapes.

Second wave (after the maintainer confirmed LCP_ORG.PRG as the
reference): stair states hold the planted F3/F3S frames (walk core
lcp_pat/lcp_fst byte-MATCH); tv_boul/tv_patl draw a deliberate
pseudo-random second polyline point via frame-layout overlap (reverts
12e572f semantics); the game's OWN VDI binding layer exists at ROM
0xd664-0xd976 (vdiown.c + injected trap-#2 stub -- game code never
draws through VDIBIND); od_draw/sp_draw use the discrete-arg vro_cpy;
stpScrB/fillTopR share ONE buffer (dsb_stor was invented); rev_tab is
initialized DATA; and this binary has NO interactive title screen, NO
Timer-A install (st_titl defaults PLAYER/noon, mq_intim is empty), and
NO move-in cutscene (cs_mvIn is a state initializer).

Third wave: vdi_init matches ROM 0x7b72 (local work arrays, no
resolution check, inline mouse-off + screen clear; sc_ers deleted),
initVdi matches 0x78c8, vdi_cprt folded into vro_cpy.

**Policy decision (2026-09-01): the minigames are KEPT.**  The poker/
blackjack/word-puzzle/anagram suite (~26 KB), the Timer-A MIDI
sequencer (mq_*/psg_upE + mq_tick.s + the Xbtimer install in
mq_intim), and their support helpers (mg_wkev, lcp_lgt/lcp_rgt,
vst_h20, rst_vsth, moff, + linked vqt_att/vst_hei) come from the
OTHER, larger game revision and do not exist in LCP_ORG.PRG (whose
entire games area is ~1 KB at 0x72ac and whose music engine is ~1.5 KB
at 0x8cce, polled -- no ISR).  They are retained as intentional
non-fidelity and reported as KEPT by verify_bytes.py.

**GOAL ACHIEVED (2026-09-01): the FAITHFUL build is BYTE-IDENTICAL
to LCP_ORG.PRG** (MD5 02900cfd883ed80b9187013c161536f2, 87832 bytes).
Reproduce with:

    ALCYON_CPPFLAGS="-DFAITHFUL=1" source/tools/alcyon_build.sh
    FAITHFUL=1 source/tools/alcyon_link.sh
    python3 source/tools/prg_diff.py     # prints *** BYTE-IDENTICAL ***

How it was reached (all phases DONE):
 1. Code completeness (-DFAITHFUL): ROM minigame banner stubs, dead
    stub 0x8030, empty mq_intim, the vestigial music engine (mq_dise
    byte-identical; 10-byte PSG_ENVELOPE; Timer-A tail gated out and
    mq_tick.o dropped via FAITHFUL=1), the ROM's own workstation
    module (vdilib.c + vdilib_a.s, second parameter block vdipb2).
    Text segment: every function at its exact ROM address.
 2. Data layout: globals.c reordered to the ROM's declaration order
    (od_* blocks, mi_evi/mi_evcn/mi_nlp0 initialized in data, g_msmk
    position, vdipb moved from vdiown.c, minigame window at 0x12484,
    poker/wordpuzzle kept-only data gated with #ifndef FAITHFUL,
    g_ptdsi[12]/g_obdea[3] boundary, short[19] g_cotbl, static
    g_dsb = dsb_stor initializer).  Three real code-reference fixes
    fell out: pk_main clears g_pcbet/g_ppbet at ROM 0x124a0, tick's
    petting table starts at 0x1358a, and cs_mvIn sets lcp_face (not
    g_lcyof).
 3. BSS layout: kept-only commons gated; ROM sizes restored under
    FAITHFUL (SPRITE_HW_SLOTS_ALLOC=8, pst_arr[4], dsb_stor[16256],
    pk_ch/pk_ph[26], assets body_buf[20000]/pex_buf[12000]); the ROM
    stores the pex frame pointer over pex_name's first 4 bytes --
    FAITHFUL aliases pex_ptr to that slot via the 8-char-truncation
    trick (pex_namP -> _pex_nam).  The 1985 linker's .comm
    allocation order matches NO surviving linker (native lo68/link68
    hash-group, ALN.PRG sorts alphabetically, and it is neither
    first-mention nor last-mention order), so the allocation is
    carried as a checked-in layout spec: tools/rom_bss_layout.tsv
    (222 rows, port symbol+offset -> ROM address, bss size header).
    alcyon_link.sh's FAITHFUL path finishes with tools/bss_remap.py,
    which resolves the spec against the lcp_sym.68k side link and
    rewrites the 873 relocated BSS longwords -- the original binary
    is NOT read at link time.  `bss_remap.py --gen` regenerates the
    spec from DATA/LCP_ORG.PRG after a layout-affecting change; its
    site-by-site pairing verifies the port->ROM translation is a
    consistent one-to-one mapping, which is the proof that the
    port's reference structure matches the ROM's exactly.

The default (kept) build is unaffected: minigames, Timer-A MIDI, and
the hardened array sizes all remain (verified with --auto long-run,
see the SOUNDS.LCP note below).

Current: **verify_bytes on the FAITHFUL build reports 316 matched /
0 divergent (99.2% of text; the rest is sub-48-byte stubs it skips)
-- and whole-file byte-identity makes the function-level stats a
formality.**  `main` matches ROM 0x1ba (no Dsetpath, no
initBM call -- bm32or/bm32and stay zero at runtime, the ROM's own
dead code; ct_clrB lands at ROM-identical 0x42e via same-object bsr).
`gameTick` matches ROM 0xce28 (carrying mode returns via the restored
cy_yoff switch helper; getKey's no-key sentinel is 0 in this binary,
NOT -1 -- the 2026-07-19 incident's polarity belongs to the other
image).  The ROM's polled music engine (0x8cce) and games
shell (0x72ac) are byte-recovered in the FAITHFUL build; the kept
build carries the other-revision versions.  The kept build's
behavior changes (SOUNDS.LCP, tv polylines, stairs, single-buffer
compositing, vdi_init clear, gameTick Path-B return, key sentinel)
are re-verified with test_longrun_stable.sh after risky changes
(NOTE: run_hatari.sh lacked --auto until 2026-09-01 -- smoke runs
before that date booted to the desktop and tested nothing).

**Two Ghidra programs (RESOLVED 2026-09-01).**  `LCP.rep` contains
TWO programs (`LCP.PRG.1` and `LCP.PRG.1.1`); port comments citing
addresses like main@0x15546 / gameTick@0x256a6 / mq_tick@0x1219a come
from the OTHER, larger revision, not from LCP_ORG.PRG.  The
maintainer confirmed `DATA/LCP_ORG.PRG` as the porting reference; the
other image's exclusive features (playable minigames, Timer-A MIDI,
interactive title) live on as the intentionally-kept default build.
When reading old Ghidra-address comments, check which image they
refer to before trusting offsets.

**Physical second reference recovered (2026-09-01):** the protected
Pasti image `Little Computer People.stx` (repo root) was extracted
with `source/tools/stx_extract.py` -- an 80-track single-sided FAT12
volume whose 32 DATA/ files are all byte-identical to the repo's,
plus the UNCRACKED `LCP.PRG` of the larger revision (123352 bytes:
text 104156 / data 12260 / bss 187450), now checked in as
`DATA/LCP_STX.PRG`.  It carries the playable minigames, an Xbtimer
(Timer-A) install, and a real (non-stubbed) protection region where
LCP_ORG.PRG has the crack's 10-byte cp_main stub.  A verify_bytes
sweep against it matches 84 port functions byte-for-byte but shows
the two revisions genuinely differ (immediate-vs-global operand
shapes, bsr-vs-jsr call shapes) -- it is a different build, not a
relink.  Open lead: LCP_STX.PRG may be the pre-crack parent of the
other Ghidra image, and may reveal the intact SOUNDS.LCP loading
that the crack destroyed (see the SFX crash note below).

## Toolchain reconstruction (2026-09-01)

The original native toolchain directory (~/hatari-c) was lost in a
disk reorganization.  The entire environment is now REGENERABLE with
`source/tools/build_toolchain.sh`, which rebuilds host tools from
`~/Hatari_C/Compiler/Alcyon/alcyon` (Thorsten Otto's cleaned-up
Alcyon C sources -- "no changes that would lead to different code
generation") into `~/Hatari_C/hatari-c/{bin,src,TOOLS/INCLUDE,GAME}`:
cp68 c068 c168 as68 ar68 link68 relmod optimize.  Host patches (all
scripted, see the script header): SSIZE 8->32 (identifier/macro
significance), parser/init.c unsigned-array initializer fast paths,
macOS shims.  The linker is now link68 (DRI CLI, response files,
`PRGFLAGS[0]` -- the modern default of 7 was the single differing
byte on first try); relmod converts to PRG; bss_remap.py is
unchanged.  TOOLS/INCLUDE comes from the DK DISK_1/COMPILER headers
with CP/M ^Z markers stripped plus reconstructed ostruct.h (_DTA)
and an MFDB typedef appended to vdibind.h.  VALIDATED: rm -rf of
both the toolchain and build tree, full regeneration, FAITHFUL
rebuild -> byte-identical LCP_ORG.PRG (MD5 02900cfd).  Quirk: this
cp68 crashes on ~120+ char input paths; keep build paths short.

## Campaign #2 (2026-09-01): LCP_STX.PRG is the truth for the C sources

Maintainer directive: recover the C sources for `DATA/LCP_STX.PRG`
-- the uncracked, larger revision extracted from the protected
Pasti image -- as the porting truth going forward.  The FAITHFUL
(-DFAITHFUL) configuration and its byte-identity to LCP_ORG.PRG
remain frozen as-is.

Reconnaissance (all verified by direct binary analysis):
- **Ghidra correspondence solved:** the "other Ghidra image"
  (`LCP.PRG.1.1`) is LCP_STX loaded at BASE 0x10000.  Every old
  port comment citing other-image addresses resolves as
  `addr - 0x10000 = LCP_STX text offset` (mq_tick 0x1219a ->
  0x219a, gameTick 0x256a6 -> 0x156a6 [confirmed: STX code calls
  0x156a6 where the port calls gameTic], main 0x15546 -> 0x5546,
  st_titl 0x16de6 -> 0x6de6).
- **Toolchain differs from LCP_ORG's:** the STX startup is the
  OLDER Alcyon distribution's GEMSTART.O verbatim
  (~/Hatari_C/Compiler/Alcyon/alcyon2/, dated 1985-05-30; 250
  bytes, only relocation tails differ) -- NOT the Atari DK
  revision used by LCP_ORG.  The alcyon2 distribution has the
  full toolchain (CP68/C068/C168/AS68, AESBIND, and
  alcyon/orig/lib's gemlib) for an eventual byte-identical link.
- **Function inventory** (verify_bytes with LCP_REF=DATA/LCP_STX.PRG
  against the KEPT build, kept-classification disabled): 85
  functions already byte-match, including mq_tick at 0x219a (the
  Timer-A ISR is byte-faithful to this truth), mq_inti/mq_extm,
  mg_stp, ag_intr, pk_pmsg, wp_shwm, and Activision's workstation
  module (v_opnvwk at 0x17426, vro_cpyfm, vdi_go2).  ~213 port
  functions diverge: the two revisions are genuinely different
  builds (immediate-vs-global operand shapes, bsr-vs-jsr call
  shapes), and the kept minigame/MIDI code is shape-faithful but
  not yet literal-faithful to this binary.

Tooling: all comparison tools (verify_bytes.py, prg_diff.py,
fn_diff.py, rom_map.py) now honor `LCP_REF=<path>` to select the
reference binary; default stays DATA/LCP_ORG.PRG.

**The library boundaries (maintainer, 2026-09-02).**  In LCP_STX
everything BEFORE mq_skip (0x12a) is runtime library -- alcyon2's
GEMSTART.O plus the trap bindings at 0x11a -- and everything from
vswr_mode (0x1733a) on is library too: the VDI bindings, AES, stdio,
string and math.  So the game's own code is 0x12a-0x1733a, 94 736 of
the 104 156 text bytes; the other 9 420 come from the link.  Both
library regions already match, with ONE exception, now resolved:

  **LCP_STX has no LIBF.**  Its __pftoa/__petoa call _ftoa and _etoa
  at address ZERO -- the 1985 link simply left the two externals
  unresolved (the %f/%e conversions are unreachable in this program).
  alcyon_link.sh reproduces that for the default build: gemlib without
  libf, linked with link68's `UNDEFINED` option so the danglers
  resolve to 0 instead of failing the link.  That removed 2 102 bytes
  of float formatter the original never had, and with it 17 divergent
  functions.  FAITHFUL keeps libf -- LCP_ORG does contain it.

Locating a divergent function is its own problem: verify_bytes hunts
with the first 24 bytes and gives up when the prologue differs.  Four
tools solve it:
  * `stx_addrs.py` reads callee addresses out of the relocated call
    sites inside functions that already match (and inside the matching
    PREFIX of divergent ones).  Authoritative.
  * `stx_locate.py` slides a 40-byte window over the port's bytes and
    looks for a stretch that occurs exactly once in LCP_STX.  It
    located 36 further functions, but its start address assumes the
    code before the window is the same length in both revisions --
    treat it as a lead and expect to nudge the address.
  * `stx_neighbor.py` infers a function's address from the matched
    function that precedes it: where a unity unit already reproduces
    LCP_STX's order, the next function starts where that match ends.
    It reports the matching prefix so a wrong guess is obvious.
  * `stx_whatis.py` goes the other way -- given an STX address it
    ranks the divergent port functions by length and similarity.

  A `bsr`'s TARGET inside a matched function is the most reliable
  evidence of all: mq_advs's call pinned mq_pars at 0x338, sp_imfs's
  pinned sp_iniM at 0x6612, and er_food's pinned a_opecc/a_opcfd.
  Always prefer a harvested address over a window or neighbour guess.

First fn_diff findings (sf_sl vs STX 0xdcc4) -- structural rules of
this build, ALL different from LCP_ORG's:
- **sf_sl is a REAL SOUNDS.LCP block loader** in the STX revision
  (fr_read sizes, Malloc per block, store into mi_ntLp at 0x43f7a =
  Ghidra 0x53f7a - 0x10000, er_nomem on failure) -- final proof the
  crack destroyed the loading and the kept build's restored loader
  matches the real original's intent.  The port's loop shape is
  already close; literals/frame layout differ.
- **GEMDOS binding lives at text 0x11a** (right after the alcyon2
  gemstart) and calls use MINIMAL argument shapes -- `Fclose(h)`
  pushes just the opcode+handle, no 3-arg padding.  LCP_ORG's
  padded opcode+3-args osbind convention does NOT apply here; the
  include/osbind.h shapes must become configuration-dependent.
- **bsr-vs-jsr call patterns differ throughout** because the STX
  build's SOURCE-FILE PARTITION is completely different from the
  port's.  `source/tools/stx_objmap.py` recovers it wholesale: a
  bsr from A to B proves everything in [A,B] is one object (as68
  only shortens same-object calls, and the linker lays each object
  down contiguously), so merging all bsr intervals bounds the
  objects.  Result: **20 clusters, and ZERO of the 1426 jsr edges
  falls inside one** -- the model is airtight.  Independent proof
  it is real: the library clusters reproduce actual DRI libc source
  files (`_access,_chmod,_chown` / `_free,_realloc` / `_lseek,_tell`
  / `__creat,_opena,_openb`).

  The game code is only ~7 objects, all huge:

      0x0012a-0x01586   5.2 KB  mq_inti/mq_extm (MIDI init)
      0x0230e-0x04004   7.4 KB  (no matches yet)
      0x0400c-0x073ce  13.3 KB  dog, actions, movement, calendar,
                                renderx, alerts functions
      0x073e8-0x0d9ce  26.1 KB  the ENTIRE minigame suite (maps 1:1
                                onto the port's games.c -- already
                                a correct STX unit)
      0x0dece-0x1481c  27.0 KB  render, sprites, delivery, aidle,
                                asimple, ahouse, init, health,
                                gfx_prim functions
      0x14824-0x148e6   194 B   er_write (alerts)
      0x148fe-0x172e8  10.7 KB  the sprite engine

  **Strategy consequence:** per-function regrouping between .c files
  is the wrong tool -- each such move was hand-replicating one edge
  of this partition.  The port should instead build the DEFAULT
  configuration as ~7 "unity" translation units that #include the
  constituent .c files in STX order, leaving the individual files
  (and the FAITHFUL build) untouched.  The moves already committed
  (sp_spud/sp_flih -> alerts.c, initVdi/exitVdi -> games.c,
  cl_redrH/cl_drwH/drwLine -> init.c) are all consistent with the
  recovered partition -- e.g. er_write ends at 0x148e6 and sp_spud
  starts at 0x148fe, 24 bytes apart.

  **The unity-unit mechanism is VALIDATED** (probe, 2026-09-01): a
  TU that `#include`s movement.c + calendar.c compiles cleanly
  through cp68/c068/c168/as68 and turns the cross-file call into
  `bsrs` in the object bytes, where separate compilation emits
  `jsr abs.l` + relocation.  c168 always writes `jsr` in the .s;
  as68 alone decides the encoding, so ONLY the object bytes prove
  it (grepping the .s is misleading).

  **Membership must be per-FUNCTION, not per-file:** the port's
  alerts.c is split across two STX objects -- er_nomem lands in the
  0x400c-0x73ce cluster while er_write is in the 0x14824 one.  So
  the unity units cannot simply include whole port .c files
  wherever the port's own grouping disagrees; those files need
  splitting first (or per-function #ifdef gating as already done
  for sp_spud/sp_flih).  Extending cluster membership beyond the 96
  byte-matched functions -- e.g. by collecting the `candidate
  orig=` addresses verify_bytes reports for divergent ones -- is
  the natural next evidence step before any restructuring.

  **Merging a straddler wholesale is a REGRESSION (measured
  2026-09-01).**  Adding sprites.c to unit 2 -- on the evidence that
  sp_sprs and sp_ssco are bsr targets from inside that object --
  dropped the count 130 -> 120, because most of sprites.c actually
  lives in the 0x148fe sprite object and the merge turned its
  cross-object jsr calls into bsr.  Reverted.  The right move is
  per-FUNCTION extraction through parts/: sp_sprs, sp_ssco, hideLcp
  and showLcp now have shared bodies included by sprites.c under
  FAITHFUL and by stx_u2.c otherwise, at their STX addresses.  A
  unity unit also needs the constituent headers at its top (they
  emit no code, so the layout is unaffected) or the parts/ bodies
  compile with nothing in scope.

  **DONE: the LCP body/shape buffers are ARRAYS in STX, not
  pointers** (sp_updb now matches).  The divergence was
      port:  muls.w #84,d0 ; ext.l d0 ; add.l body_shp,d0
      STX:   muls.w #84,d0 ;            add.l #184892,d0
  Probing settled it: `(char *)ptr + i * 84` and every cast variant
  emit the ext.l, while `a2[i]` on `char a2[][84]` (or `&sarr[i]` on
  an 84-byte struct array) emit exactly the STX shape -- an
  immediate base and no ext.l.  Checked against alcyon2's own
  C168.PRG under Hatari: it emits the ext.l too, so this is NOT a
  codegen difference; STX's source indexes real global arrays where
  the port carries `body_ptr`/`body_shp` pointer variables assigned
  by al_locs from statics in assets.c.  Converted for the STX configuration: sprglobs.c defines
  body_ptr[120][168] and body_shp[98][84] as globals, sprglobs.h
  declares them with literal strides (sprglobs.h is included before
  sprites.h, so LCP_BODY_FRAME_SIZE is not yet in scope there),
  al_locs loads straight into body_ptr, and sp_updb indexes
  body_ptr[frame] / body_shp[frame].  FAITHFUL keeps the pointer
  variables plus bshdbuf.  Same class as the g_obtmp->g_obtmt and
  od_* fixes -- expect more of these wherever the port carries a
  pointer variable that STX addresses as an array.

  **The VDI binding layer differs in three ways** (all six simple
  bindings recovered together): STX assigns intin[0] BEFORE filling
  contrl, RETURNS intout[0], and reaches vdi_go with a jsr -- i.e.
  vdi_go lived in another object there.  LCP_ORG keeps vdi_go inside
  vdiown.o, where as68 shortens the calls to bsr on its own (the
  build script's jsr->bsr rewrite was belt-and-braces, not the
  cause).  The STX configuration therefore assembles vdiown_a.s as a
  separate object and skips the injection; alcyon_link.sh adds
  vdiown_a.o only when not FAITHFUL.

  v_bar and v_pline go further: instead of copying points into ptsin
  they AIM the parameter block at the caller's array for the call and
  restore it afterwards (vdipb[2] = pxy; ... vdipb[2] = ptsin), the
  same trick vdilib.c's vro_cpyfm already used.  vroCpyD is likewise
  only a wrapper in STX -- it builds a pxy[8] on the stack and defers
  to the array-form vro_cpyfm rather than writing contrl itself.
  Also: stx_addrs resolves _vdi_go and _vdi_go2 to the SAME address
  (0x1772e), so STX has one trap dispatcher where the port carries
  two.

  **The port's file split does not match STX's.**  a_gesff (afood.c)
  sits at 0xebf8, BETWEEN two adoors.c functions -- a_clocd 0xeb54
  and a_opecf 0xec22 -- which is why its call to a_opecf is a short
  bsr.  So the STX sources grouped these differently again; per-
  function parts/ placement, not file ordering, is what reproduces
  it.  `stx_addrs.py` plus a short-vs-word bsr is usually enough to
  pin where a function belongs.

  **Clusters are LOWER bounds -- objects start earlier.**  lcp_lgt
  (0xde80) and lcp_rgt (0xdf66) sit in the gap BEFORE the 0xdece
  cluster, yet both reach lcp_wkD (0x147a0) and sp_ssco (0x1203a)
  with bsr, so they are part of that object: it begins before its
  cluster does.  Both moved from games.c into unit 2 via parts/ --
  games.c straddles the minigame object and this one.  Note that
  editing a parts/ file does NOT trigger a rebuild of the unit that
  includes it; rebuild the unit (or everything) explicitly.

  **Membership map (done -- `stx_objmap.py --members`).**  Folding
  in the candidate addresses, only FIVE port files straddle STX
  cluster boundaries and therefore need splitting; every other file
  already sits wholly inside one cluster:

      alerts.c    3 clusters  er_nomem @0x400c | er_write @0x14824
                              | sp_spud,sp_flih @0x148fe
      games.c     2 clusters  minigame suite @0x73e8 | lcp_rgt @0xdece
      gfx_prim.c  2 clusters  vst_h20 @0x73e8 | drwPixel @0xdece
      init.c      2 clusters  mq_inti @0x12a | cl_drini,cl_redrH @0xdece
      sprites.c   2 clusters  hideLcp,showLcp @0xdece | sp_updb @0x148fe

  Concrete restructuring plan, in order:
   1. Split those five files along the listed boundaries (or gate
      the minority functions per-configuration, as sp_spud already
      is), so every port .c belongs to exactly one STX cluster.
   2. Add default-build-only unity units (stx_u*.c) that #include
      their cluster's .c files; teach alcyon_build.sh to compile the
      units instead of the constituents in that configuration.
      FAITHFUL keeps compiling the files individually -- its own
      partition (LCP_ORG's) is already proven correct and MUST stay
      byte-identical.
   3. Recover intra-unit ORDER from the matched functions' STX
      addresses (e.g. cluster 0x400c runs dog < actions < movement
      < calendar < renderx < alerts) and order the #includes to
      match; ordering only affects addresses, not verify_bytes
      matching, so it can be tuned after the bsr/jsr shapes land.
   4. Re-sweep.

  **Step 1-2 DONE for the 0x400c object (stx_u1.c), and the measured
  result corrects the prediction above:** the unit compiles (17 bsr
  emitted where separate compilation had jsr), all seven of its
  byte-matched members still match and are now laid out contiguously
  in STX order, FAITHFUL stays byte-identical -- but the match count
  did NOT move (112 before and after).  Unity units are necessary
  infrastructure, not a source of matches by themselves: a partition
  fix only flips a function to MATCH when the call shape was its
  LAST divergence.  The earlier gains that looked like partition
  wins (the ag_c* quartet, cl_drini) were exactly those cases.  The
  remaining divergences inside stx_u1's files are ordinary logic and
  literal differences -- e.g. execEv (0x5fae) is missing a call and
  has different branch structure -- so per-function recovery remains
  the way the count moves.  Build both configurations after any unit
  change: `tools/stx_units.txt` drives which files are skipped, and
  alcyon_build.sh deletes stale objects for skipped files.

  **Codegen is NOT the explanation for source-shape divergence
  (tested 2026-09-01).**  The obvious hypothesis -- that `i++` vs
  `i = i + 1` shapes come from LCP_STX's older compiler rather than
  from its source -- is FALSE.  Running alcyon2's own C168.PRG (the
  1985-05-30 binary that built LCP_STX) under Hatari on the same
  input emits, instruction for instruction, what our rebuilt c168
  emits: `i = i + 1` -> move/add/move through a register, `i++` ->
  `add #1,-2(R14)` straight to the frame slot.  So these really are
  source differences and must be recovered as such (gated
  `#ifdef FAITHFUL`), and -- valuable side result -- the rebuilt
  toolchain is codegen-equivalent to the one that built LCP_STX, so
  the campaign never needs to run the period compiler under Hatari.

  **sf_sl is byte-recovered (2026-09-01).**  The kept build's
  SOUNDS.LCP block loader -- restored earlier from the port's own
  history because LCP_ORG's cracked stub crashes on the first sound
  effect -- now matches LCP_STX byte for byte at 0xdcc4.  The real
  1985 loader declares `fhandle, size, block, index` in that order,
  assigns the Malloc result straight into `mi_ntLp[index]` and reads
  it back into `block`, widens with `(long) size + 4` (not
  `(long)(size + 4)`), and walks the buffer with `block++` before
  the payload read.  So the kept build's sound loading is no longer
  a reconstruction: it is the original's code.

  **Embedded-assignment rule (2026-09-01).**  Where the port writes
  two statements, LCP_STX often nests the assignment in the
  expression, which Alcyon compiles WITHOUT the reload:
      x = a / b;  y = x + '0';      ->  move.b d0,x / move.b x,d0 /
                                        ext.w / add / move.b d0,y
      y = (x = a / b) + '0';        ->  move.b d0,x / add / move.b d0,y
  The same holds for `s[6] = (rem = v % 100) / 10;`.  Confirmed by
  compiling both spellings (t1/t2/t4 probes) and comparing to the
  binary.  This recovered pk_awp / pk_dppm / pk_dpot, whose STX
  versions also use `char str[10]` plus a `short rem` temp and carry
  no (char)/(int) casts at all.

  **Function ORDER inside an object matters** (and is measurable):
  a call is bsr.s only within +-128 bytes, so ordering shows up as
  bsr.s-vs-bsr.w.  plEr's sole divergence was exactly that -- STX
  puts it at 0x86e0, past the anagram helpers, ~4 KB from initVdi,
  where the port had it 40 bytes away.  Moving it (via parts/,
  because plEr is shared and FAITHFUL needs it in LCP_ORG's spot)
  fixed it.  Known games-object order from matched addresses:
  mg_stp 0x759c < vst_h20 0x75dc < initVdi 0x764e < exitVdi 0x76d0
  < wp_shwm 0x7c78 < ag_cwda 0x7e9c < ag_cswa < ag_cgpa < ag_csb
  0x7f4a < ag_intr 0x7f84 < plEr 0x86e0 < pk_awp 0xad26 < pk_dppm
  < pk_dpot < pk_pmsg 0xb0aa < pk_actd 0xb138.

  **verify_bytes extent fix (2026-09-01).**  A `static` function
  emits no symbol, so the old symbol-to-symbol extent swallowed it
  and reported the PRECEDING function divergent on bytes that were
  not its own.  Extents are now additionally bounded by the next
  `link a6` prologue that follows an rts/nop, which is the real
  function end.  That alone converted two standing false negatives
  (exitVdi, wp_shwm) into matches, so counts before and after this
  change are not directly comparable.

  Recurring source-shape rules recovered so far, all gated:
      i = i + 1            (ORG)  vs  i++            (STX)
      x = x - 5            (ORG)  vs  x -= 5         (STX)
      *idx = *idx + 1      (ORG)  vs  (*idx)++       (STX)
      do{d--;if(!d)break;  (ORG)  vs  while(--d){body;
        body}while(cond)              if(!cond)break;}
      r = f(); if (r)      (ORG)  vs  if (f())       (STX)
      while (A && B){}     (ORG)  vs  while(A){if(!B)break;}
      unsigned short i     (ORG)  vs  short i        (STX)
      BOOL16 flag          (ORG)  vs  char flag      (STX)
                                      (tst.b at the use sites --
                                       mi_play, mi_dvel, psg_dvol)
                                      (no clr.w zero-extension
                                       around index arithmetic)
      mask in the loop     (ORG)  vs  folded into the assignment,
        condition                     computed once
      gameTick(3)          (ORG)  vs  t = 3; ... gameTick(t)
      if (x == 3) A else B (ORG)  vs  if (x != 3) B else A
      lcp_face = c ? L : R (ORG)  vs  the assignment duplicated
                                      inside both branches
      unsigned bound       (ORG)  vs  signed (bcs vs blt on the
                                      loop comparison)
      p = f(); q = p & 3   (ORG)  vs  p = f() & 3
      if/else-if ladder    (ORG)  vs  switch (Alcyon puts the
                                      compare chain at the bottom)
      w = f(); g(w)        (ORG)  vs  g(w = f())
      while (n != 0){...   (ORG)  vs  while (n--) { ... }
        n = n - 1;}                   (load into d0, subq to memory,
                                       test the OLD value in d0)
      if (c) f(); break;   (ORG)  vs  if (!c) break; f(); break;
                                      (the beq displacement gives it
                                       away: over the call vs to the
                                       switch end)
      case ORDER in the source is recoverable from the jump table's
        targets (STX's execEv writes BOOK, RECORD, FOOD, PHONE,
        GET_DRESSED, DOG_FOOD)
      statement ORDER of two initialisations is evidence too
        (a_hello clears pick before prev_pick in STX)
      (Random() & 7) + 293 (ORG)  vs  (int)(Random() & 7) + 293
                                      (the cast makes the ADD word-
                                       sized: and.l then add.w, where
                                       the uncast form adds long)
      f((x & 15) | 1)      (ORG)  vs  f((int)((x & 15) | 1))
                                      (uncast, Alcyon computes in the
                                       argument slot: move.l d0,(sp) /
                                       andi.l #15,(sp))
      RECT16/short[4]      (ORG)  vs  short pts[10] -- the STX TV
                                      routines all carry a 10-short
                                      point buffer and write all four
                                      coordinates
      x = x + 1; if (x > N) (ORG)  vs  if (++x > N)  (the pre-increment
                                      form loads the value back for
                                      the compare: add.q then move to
                                      a register, where the compare on
                                      the plain variable is cmpi #N,mem)
      a = 0; b = 0; c = 0; (ORG)  vs  a = b = c = 0 (one value computed
                                      into d0 and stored right to
                                      left -- the FIRST store is the
                                      RIGHTMOST variable, so the store
                                      widths give the declaration
                                      types away)
      helper C function    (ORG)  vs  hand-assembly (psg_wr, psg_mix
                                      and mowrit are frameless
                                      absolute-long pokes in LCP_STX;
                                      source/psg_asm.s carries them)
      f(&d, &c, &b, &a)    (ORG)  vs  f(&a, &b, &c, &d)
      short table          (ORG)  vs  char table (sf_pri, g_mcpro),
                                      and mi_nxTk/mi_lpTk are LONG
                                      tick counters
      int field             (ORG)  vs  unsigned field (clr.w before
                                      every load -- cpyScr's MFDB
                                      extents; `(unsigned short) f`
                                      at the use site reproduces it)
      p->fd_addr = addr;   (ORG)  vs  the address latched, masked to
                                      its high half and stored as two
                                      words through a (short *) cast
                                      (sp_iniM)
      } else {             (ORG)  vs  } else if (arg != 0) {
                                      (a redundant re-test of the
                                       argument already implied by the
                                       else -- a_opcfd/a_opecc/a_opecd,
                                       and er_food re-tests g_dvdog)
      one assignment       (ORG)  vs  an if/else whose two arms assign
                                      the SAME value (stpScrB's size)
      p = (T *)((char *)p    (ORG)  vs  (char *) p += n;
        + n)                          (Alcyon C 4.14 accepts a CAST
                                       AS AN LVALUE; the compound form
                                       emits add.l d0,mem where the
                                       assignment form emits
                                       add.l mem,d0 / move.l d0,mem --
                                       sc_firw/sc_firs/sc_firb)
      (long) row * 160     (ORG)  vs  row * 160  (muls.w + ext.l
                                      instead of a call to lmul)
      for (i=0; s[i]; i++) (ORG)  vs  while (dst[i++] = *src++ & 0xff)
        dst[i] = s[i];                (v_gtext, and contrl[3] = --i)
      case X: return V;    (ORG)  vs  case X: return V; break;
                                      (the dead break emits a second
                                       branch; the LAST arm has none
                                       -- getKey)
      if (c) return v;     (ORG)  vs  if (c) return v; else <stmt>
                                      (the else-skip branch shows up
                                       even though the then-arm
                                       returns)
      for(;;){...if/else}  (ORG)  vs  a label + two explicit `goto`s
                                      (a loop whose arms each branch
                                       straight back, with no shared
                                       loop-back branch: fOpen,
                                       fr_read)
      void f()             (ORG)  vs  short f() returning the value
                                      the ORG version discards
      for (;;)             (ORG)  vs  while (1)  (the while form
                                      emits an entry bra to the
                                      bottom jump; for(;;) does not)
      x <<= 9              (ORG)  vs  x = x << 9  (<<= loads the
                                      shift count first)
      short table          (ORG)  vs  char table (moveb + extw at
                                      the use sites -- sf_pri)
      a static helper      (ORG)  vs  the body written out at each
                                      call site (dv_pick)
      if (a <= b)          (ORG)  vs  if (b >= a)  (which operand
                                      lands in d0 gives it away)
      Setscreen(l,p,-1L)   (ORG)  vs  Setscreen(l,p,-1)
      Giaccess(0L, 0x88L)  (ORG)  vs  Giaccess(0, 0x88) with the
                                      alcyon2 header's (char)/(short)
                                      argument casts
      lcp_y = lcp_y + 9    (ORG)  vs  lcp_y += 3; ... lcp_y += 6;
                                      (STX splits the step around the
                                       state assignment -- two subq/
                                       addq to memory, not one addi)
      p[i - 1]             (ORG)  vs  *(p + i - 1)  (writing the
                                      offset arithmetic inside the
                                      dereference makes the POINTER
                                      the index register and the
                                      counter the base -- mq_pacm)
      short locals for     (ORG)  vs  globals (aes_ini's graf_handle
        out-parameters                metrics; vdi_init's work_in /
                                      wk_out -- the frame collapses
                                      from -150 to -6)
      short *tab[3] = {r0, (ORG)  vs  short tab[3][8] and `tab[i][j]`
        r1, r2}                       (two ext.l and a trailing
                                      `add.l #base` -- chk_timA)
      x = x - 1;           (ORG)  vs  x--; if (x <= 0)   (subq to
        if (x < 1)                    memory then an explicit tst)
      x = x - 1;           (ORG)  vs  if (--x == 0)      (the subq's
        if (x == 0)                   own flags, no tst at all)
      t = f(); if (t == v) (ORG)  vs  if (f() == ++v)    (the call
                                      result stays in d0 across the
                                      addq to memory -- gameSim1)
      C helper             (ORG)  vs  hand-assembly: blkcp32 is an
                                      unrolled `dbf` loop of eight
                                      post-increment long moves
                                      (source/blkcp_a.s)
      two early returns    (ORG)  vs  one compound `if (a && b) {...}`
                                      followed by `break` (deal_kc's
                                      default arm -- the guards branch
                                      to the break, not the epilogue)
      arr[i] = v           (ORG)  vs  *(i + arr) = v  (naming the
                                      index first folds the base into
                                      `add.l #base,An` instead of a
                                      second address register)
      `return v;` in every (ORG)  vs  NO return at all in the arms:
        arm of a ladder               the ladder's jump to the
                                      function end leaves the last
                                      compared value in d0 (chk_timA)
      x = 4;               (ORG)  vs  x == 4;  -- a real 1985 typo in
                                      gameSim1's sickness clamp; the
                                      compiler emits the comparison
                                      and discards it, so the clamp
                                      never happens.  Preserved.
      Random()             (ORG)  vs  rnd(), a global wrapper around
                                      the bare XBIOS call that STX
                                      routes seven call sites through
                                      (rndRng still inlines the trap)
      moff only            (ORG)  vs  moff AND mon -- STX has the
                                      mouse-show counterpart right
                                      after it
      rev_tab as DATA      (ORG)  vs  built at run time by a 94-byte
                                      routine behind a 10-byte wrapper
                                      (STX 0x6804/0x680e -- not yet
                                      ported)
      no resolution check  (ORG)  vs  vdi_init splits in two: the
                                      opener checks wk_out[0] and hangs
                                      on a "[1][Must be in|low
                                      resolution.][REBOOT]" alert loop
                                      before bsr.s-ing into the
                                      attribute/clear half
      arr[i] != '\0'       (ORG)  vs  arr[i]  -- a BARE truthiness
                                      test is what makes Alcyon reach
                                      the array with an indexed EA
                                      (movea.w idx,a0 / movea.l
                                      #base,a1 / tst.b (0,a0,a1.l));
                                      the explicit `!= '\0'` emits the
                                      base+add form (ag_main)
      c = *p; if (c==' ')  (ORG)  vs  while ((c = *p++) == ' ')
                                      (Alcyon saves the flags across
                                       the pointer increment:
                                       cmp / move sr,d0 / addq to
                                       memory / move d0,ccr / branch)
      x = x + 1; if (x<N)  (ORG)  vs  if (x++ < N)  -- the same
                                      flag-save trick around a global
                                      (ag_main's guess counter), so
                                      BOTH arms see the increment
      pk_dppm();           (ORG)  vs  pk_dppm;  -- a real 1985 typo in
                                      pk_wrMn: the parentheses were
                                      left off, so Alcyon just emits
                                      move.l #_pk_dppm,d0 and drops
                                      it.  Preserved.
      switch with default: (ORG)  vs  no default arm -- the
                                      out-of-range branch goes to the
                                      switch END, not to a body
      a = f(); b = g();    (ORG)  vs  if ((a = f()) > (b = g()))
        if (a > b)                    (both values stay in d0/d1, so
                                      the compare is register-to-
                                      register with no reload)
      for(;;){...continue} (ORG)  vs  a label + `goto` -- `continue`
                                      branches to the loop's BOTTOM
                                      edge, a goto branches straight
                                      to the label, and the two are
                                      distinguishable whenever the
                                      loop is longer than a short
                                      branch (pk_wrMn)
      (long) x             (ORG)  vs  (long) x & 0xffffL  (the cast
                                      forces ext.l into d0 followed by
                                      and.l; without it Alcyon just
                                      sign-extends through an address
                                      register -- sf_irqp)
      short table row      (ORG)  vs  12-byte WORD_TO_ACTION: ten
                                      signed mask bytes, the action id
                                      at +10 and the priority at +11,
                                      with ew2pos/g_ew2b/bm_lo/g_ewb
                                      all char (chk_encm)
      concat22/rd_hz       (ORG)  vs  written out inline: sf_irqp
        helpers                       builds the 32-bit duration from
                                      its two halves and keeps its own
                                      Super block, with an UNUSED
                                      short in the frame between the
                                      counter pointer and the value
  **Compare the `link #-N` frame size FIRST.**  It says exactly how
  many locals the function really has, before touching anything:
  a_wandi needed an UNUSED local the port lacked, a_getd reuses one
  variable as its loop counter, and a_tidyh/a_playp/a_wakum have
  NONE because every call result is consumed in place.  Removing a
  declaration without checking every use breaks the build (a_tidyh
  used `result` twice).

  **Scripted gating can strand an `#else`/`#endif`.**  Wrapping a line
  that is already inside a `#ifdef FAITHFUL` block leaves the outer
  `#else`/`#endif` unmatched; cp68 does NOT error -- it drops
  everything from the unclosed `#ifdef` to EOF in the failing
  configuration, so the build still reports OK while whole functions
  disappear (hit in aleisure.c, 2026-09-02).  `stx_check.sh` now runs
  `tools/ppbalance.py` first; run it after any batch edit.

  **Gating a declaration must not disturb the OTHER configuration's
  declaration order.**  Local offsets are assigned in declaration
  order, so wrapping only the now-unused locals in `#ifdef FAITHFUL`
  silently reorders the FAITHFUL frame and breaks LCP_ORG byte
  identity (caught in a_takes/a_uset/a_opcbc: `result, count, pick`
  became `result, pick, count`).  Write BOTH lists out in full:

      #ifdef FAITHFUL
              short   result;
              short   count;
              short   pick;
      #else
              short   count;
      #endif

  Declaration ORDER is evidence too: the frame offsets pin it (a_driwa
  is rnd, counter, last_pick, pick in STX; the port had rnd, pick,
  last_pick, counter).  And STX's a_driwa never initialises last_pick
  -- the first comparison reads whatever the slot held.  Preserved as
  written; do not "fix" such things.
  a_sitae alone needed six of these; expect several per function.
  **parts/ files must respect BOTH orders.**  The four p_sf*
  wrappers sit in a different sequence in each revision (ORG:
  tvc, grt, spe, hnd; STX: tvc, grt, hnd, spe).  Putting them in one
  shared parts/ file in STX order silently changed LCP_ORG's layout
  and broke byte-identity -- the post-change cmp caught it.  They are
  now one file per function, included in each configuration's own
  order.  Rule: a parts/ file may hold several functions only if
  both revisions order them identically.

  **STX inlines what the port factored into static helpers.**
  mq_parh's switch case bodies ARE the mh_chac/mh_temp/mh_volu/
  mh_scat/mh_proc code (the jump-table targets 0x1246/0x1264/0x129c/
  0x12a4/0x132c all lie inside mq_parh) -- the port's own comments
  even carry those addresses, because its author read them there and
  factored them out.  Since the helpers are `static`, they emit no
  symbols and verify_bytes never reported them; the divergence only
  showed up as mq_parh's shape.  Expect more of this: a port helper
  whose comment cites an address INSIDE another function's range is
  inlined in STX.

  Two more type findings from the same function: STX's switch selector
  is masked (`switch (*p & 0xff)`), and mi_dvel/psg_dvol are `char`,
  not short -- byte compares and stores, word-aligned by Alcyon so
  they still sit 2 bytes apart.  Its scale-table ladder also emits the
  final `mi_dvel < 0x80` arm that LCP_ORG's comment calls dead (Alcyon
  narrows 0x80 to a signed byte, making the compare trivially true).

  A narrowing cast changes the operand width: `(Random() & 0x7f) | 8`
  gives or.l, `(unsigned short)(Random() & 0x7f) | 8` gives or.w
  (Alcyon's int is 16-bit).  Verify each guess by compiling both
  spellings before editing -- it is faster than re-running a sweep.
  **Do NOT mass-apply these rules (tested and reverted
  2026-09-01).**  Converting all 147 `x = x + 1` for-loop
  increments to a gated STEP() macro across 31 files moved the
  count 120 -> 122; gating the ONE loop there was actual evidence
  for (a_watat) reached the same 122 by itself.  The macro would
  have committed ~145 loops to a shape confirmed in only about five
  functions -- precisely the speculative single-token editing that
  produced the 2026-07-19 incident.  Gate per site, when a fn_diff
  shows it.

**Status (2026-09-03): 327 matched / 9 divergent, 66 664 of
104 156 STX text bytes (64.0%) proven byte-identical -- 70.4% of the
94 736 bytes that are the game's own code.**  LCP_ORG.PRG is NO
LONGER the reference (maintainer, 2026-09-02: it was a temporary
hack, not the original game).  New work is written directly in
LCP_STX shape instead of being gated behind `#ifdef FAITHFUL`, and
the LCP_ORG byte-identity check is no longer run.

Still divergent (all located): pk_main 0x8d10, pk_bjMn 0xbc72,
psg_upE 0x15ae, mq_dise 0x918, lcp_pat 0x470a, a_writl 0x13cd6, plus
three functions the port has only as stubs and that must be written
from scratch -- st_titl 0x6d7e (1040 B, a real interactive title
screen), cs_mvIn 0xe500 (968 B, the move-in cutscene) and cp_main
0x22c0 (7500 B, the uncracked copy protection).

Functions DELETED because LCP_STX has no counterpart: al_loan and
fLoad (lcp_crnd inlines the NAMES read, main inlines the .SCN path),
mq_spgm (a duplicate of mq_sepc), concat22 and rd_hz (inlined into
sf_irqp).

A fifth unity unit exists now: **stx_u4.c** for the sound object that
sits just ahead of the big 0xdece one -- sgPlay 0xd9ea < sf_irqp
0xdafc < sf_sl 0xdcc4 < sf_sele 0xdd88 < sf_so 0xddd8.  The evidence
is that sf_irqp reaches sf_so with a bsr while lt_sets (inside the
0xdece object) reaches sf_sele with a jsr.  aletter.c joined stx_u2
between td_line and lt_sets; lcp_flwp and getFlrY joined stx_u1 as
adjacent parts.

**Watch out for the duplicated minigame mains.**  games.c carries the
ROM banner stubs (ag_main, wp_main, pk_main, pk_wrMn, pk_bjMn) inside
`#ifdef FAITHFUL` at lines ~110-241 AND the real implementations
below the `#else`.  A unique-string edit can silently land in the
stub -- check the line number against that range before editing.

  Object membership is as much of the work as source shape.  A call
  that is `jsr` in the port but `bsr` in STX means the callee is in
  the STX object; a `bsrw` where STX has `bsrs` means the callee must
  be placed immediately after the caller.  Both are fixed by moving
  the function into `parts/` and including it at the right point in
  the unity file (see the `parts/` list in stx_u1.c / stx_u2.c /
  stx_u3.c).  Several port files straddle two STX objects and had to be split
  through parts/: render.c (fillTopR -> 0x400c, the rest 0xdece),
  gfx_prim.c (cpyScr/stpScrB -> 0x400c, sc_sdtb/sc_sdtf/drwPixel ->
  0xdece), walk.c (lcp_path/lcp_fstp -> 0x400c with getFlrY, lcp_wkD
  -> 0xdece), init.c (cl_drini/cl_redrH/cl_drwH/drwLine -> 0xdece),
  sprites.c (sp_updb/sp_drin/sp_lchu after gameTick), sprender.c
  (sp_iniM -> 0x400c), renderx.c (strPr/prCh -> 0x148fe), save.c
  (lcp_save closes the 0xdece object), sound.c (sfClick), tvanim.c
  (tv_scrc), and agames.c joins the 0xdece object entirely.

  **Extraction hazard:** a regex that stops at the first column-0 `}`
  can still sweep the NEXT function (or a following `#ifdef FAITHFUL
  #include "parts/..."` stub, whose relative path then no longer
  resolves).  After every extraction, check that the new parts/ file
  defines exactly one function and contains no `#include "parts/...`.

  **Finding a divergent function's STX address: use the CALL GRAPH.**
  The most reliable pairing signal is the set of already-matched
  callees.  For each divergent port function collect its call targets
  that are matched, translate them to STX addresses, and intersect
  with the call targets of each unclaimed STX slot; a 1.00 score with
  a plausible length is almost always right.  This pinned main 0x5546,
  lc_load 0x5ac8, vdi_init 0x66fe, chk_timA 0x6210, gameSim1 0x133da,
  rp_anim 0x13aec, sc_sctd 0x16d5a and a dozen more in one pass.
  (stx_locate/stx_neighbor are much weaker -- stx_neighbor in
  particular reports the function AFTER a match, which is only right
  when the port and STX agree on what comes next, and it produced
  several confident-looking false pairings.)

  **Unclaimed-slot inventory.**  Marking every matched function's STX
  bytes as claimed and splitting the remaining runs at `link a6`
  prologues gives the list of STX functions still unaccounted for,
  with exact addresses and lengths.  Disassembling a small slot is
  often enough to name it outright (a 20-byte slot that clears two
  globals, a 36-byte slot that calls graf_mouse(257) = `mon`, a
  10-byte slot that just calls the next function).

Roadmap (mirrors campaign #1):
 1. Function-level recovery: iterate fn_diff/verify_bytes with
    LCP_REF=DATA/LCP_STX.PRG over the divergent functions,
    recovering exact literals/shapes.  The kept build is the
    natural vehicle -- it should converge to this truth.
 2. Identify the STX revision's own runtime/lib shapes (alcyon2
    osbind/gemlib) and link order; build an stx equivalent of
    rom_data_map via rom_map.py.  DONE for the libraries:
    alcyon_link.sh links ~/Hatari_C/Compiler/Alcyon/alcyon2's
    GEMSTART.O/OSBIND.O/AESBIND/VDIBIND/GEMLIB/LIBF for the default
    build and the Atari DK set under FAITHFUL (six runtime functions
    matched immediately).  The DK gemstart is assembled to
    gemstart_dk.o and copied into place, so both configurations can
    share one build tree.
 3. End state: the default build reproduces LCP_STX.PRG
    byte-identically (its own gemstart/libs/layout), while
    -DFAITHFUL keeps reproducing LCP_ORG.PRG.

## Issue log -- ALL CLOSED (2026-09-01)

Maintainer ruling: with the FAITHFUL build proven byte-identical to
LCP_ORG.PRG, every open issue is closed -- any residual behavior in
the FAITHFUL binary is the original's own behavior, not a port bug.
The entries below are kept as historical findings.

- **LCP_ORG.PRG itself crashes on the first sound effect (CLOSED --
  original behavior).**  The ROM's sf_sl only opens+closes
  SOUNDS.LCP; nothing ever writes the mi_ntLp effect table, so
  sf_irqp dereferences a NULL entry and bus-errors reading address
  $0 (user-mode reads below $800 are supervisor-only on the ST).
  Verified 2026-09-01 by running DATA/LCP_ORG.PRG under Hatari
  --auto/TOS 1.04: op 3d50 fault at its own sf_irqp+0x64 within
  6000 VBLs.  Presumably the pre-crack cp_main loaded the blocks;
  the crack lost it.  FAITHFUL reproduces the ROM bytes (crash
  included); the KEPT build restores the SOUNDS.LCP block loader in
  sf_sl (#ifndef FAITHFUL) so effects play.

- **Real-time crash inside TOS VDI (CLOSED).**  The July report --
  wild PC in low RAM, TOS cleanup cascading into $fc9304 -- predates
  the tv polyline fix (12e572f), the freeze fix, and the entire
  byte-fidelity campaign, and its "memwatch $25722" diagnostic
  address is stale after the layout reshuffle.  With the FAITHFUL
  build byte-identical, any crash it exhibits is the original's.
  Caveat for future repro attempts: the saved Hatari config has
  bFastForward = TRUE, so scripted runs MUST pass an explicit
  --fast-forward on/off -- past "real-time" attempts that omitted
  the flag were silently fast-forwarded.

- **test_longrun_stable.sh env-side failures (CLOSED 2026-07-21).**
  Root cause was tv_boul / tv_patl calling v_pline with count=2 but
  only initialising 1 point; fixed in 12e572f.  The 36000-VBL
  long-run has passed repeatedly since (kept build, with and
  without SKIP_MIDI).

- **Stairs (CLOSED 2026-07-21).**  The TEST_STAIRS harness was a
  false negative -- it warped the LCP past the AI walk that
  establishes valid stair state.  Real play walks stairs correctly;
  lcp_path/lcp_flwp byte-faithful.  Harness deleted.

- **cp_main (CLOSED -- no longer non-fidelity).**  LCP_ORG.PRG is a
  cracked dump whose own cp_main is a 10-byte stub; the FAITHFUL
  build reproduces those exact bytes.  The original copy-protection
  routine (flock + XOR decrypt + FDC signature check) exists only in
  pre-crack originals and can't run under Hatari anyway.

- **.SNG playback (CLOSED).**  The ROM's music engine is vestigial
  (byte-identical, entered via mq_inis, never stepped).  The kept
  build's Timer-A ISR installs and ticks idle through a 36000-VBL
  --auto long-run (0 bus errors); a GEMDOS-traced run showed the
  resident simply never chose the record player autonomously in 10
  minutes.  Nothing left to fix; hearing a song is a play-session
  activity (ask the resident to play a record).
