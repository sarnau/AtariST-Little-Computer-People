# Working notes for Claude / AI assistants on this repo

This is a **faithful C port** of the 1985 Atari ST game *Little Computer
People* (Activision).  **`DATA/LCP_STX.PRG` is the one and only
reference** -- the uncracked shipped build, extracted from the
protected Pasti image in the repo root.  It is the ground truth for
every function, initialization step, and control-flow decision.  The C
source in `source/` compiles under Alcyon C 4.14 (K&R) and runs on
Hatari.

**The WHOLE BINARY is byte-identical to it** -- 123 352 bytes, MD5
eae52d14023b51d7ac459a90d37eed10, text 104 156 / data 12 260 / bss
187 450.  (This paragraph said "the TEXT segment ... data and bss
layout are the remaining work" until 2026-09-06; that has been out of
date since the goal was reached on 2026-09-03, described under "GOAL
ACHIEVED" below.)

**One command runs everything:**

    source/tools/run_all.sh              # ~3.5 min, includes the emulator
    source/tools/run_all.sh --quick      # ~10 s, build + host only

It does the byte-identity checks on a clean SHIPPED build, then the
host build and unit tests, then the runtime tests on a GATED one, and
restores the shipped build afterwards even if something fails.  That
ordering is the whole point: the two configurations are not
interchangeable, and leaving the wrong one in `build/alcyon` makes the
next run lie in a way that looks like a real failure.

The individual pieces, if you want one of them:

    source/tools/alcyon_build.sh && source/tools/alcyon_link.sh
    bash source/tools/stx_check.sh
    LCP_REF=DATA/LCP_STX.PRG python3 source/tools/prg_diff.py
    python3 source/tools/reloc_audit.py

## Toolchain

LCP_STX was built with **Alcyon C as shipped in the 1985-05-30
distribution** at `~/Hatari_C/Compiler/Alcyon/alcyon2` -- its startup
is that distribution's `GEMSTART.O` verbatim (250 bytes, only the
relocation tails differ), and its runtime is that OSBIND/AESBIND/
VDIBIND/GEMLIB.  There is NO libf: `__pftoa`/`__petoa` call `_ftoa`
and `_etoa` at address ZERO, i.e. the 1985 link left the two
externals unresolved because the %f/%e conversions are unreachable.
`alcyon_link.sh` reproduces that with link68's `UNDEFINED`.

The host toolchain is REGENERABLE with `source/tools/build_toolchain.sh`,
which rebuilds cp68/c068/c168/as68/ar68/link68/relmod/optimize from
Thorsten Otto's cleaned-up Alcyon sources into
`~/Hatari_C/hatari-c/{bin,src,TOOLS/INCLUDE,GAME}`.  Host patches (all
scripted, see the script header): SSIZE 8->32 (identifier/macro
significance), parser/init.c unsigned-array initializer fast paths,
macOS shims.  Quirk: this cp68 crashes on ~120+ char input paths, so
keep build paths short.

**The rebuilt toolchain is codegen-equivalent to the one that built
LCP_STX** (tested 2026-09-01): running alcyon2's own C168.PRG under
Hatari on the same input emits, instruction for instruction, what our
rebuilt c168 emits.  So source-shape differences really are source
differences, and the campaign never needs the period compiler.

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
shape-audit had passed; literal-audit had not.  Always verify the
sentinel against the binary, not against an anecdote -- getKey's
no-key sentinel is not the same in every revision.

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
- There is ONE SHIPPED build configuration, and it must stay
  byte-identical.  Two defines exist for test builds only:
  `-DSKIP_MIDI=1` (skips the Timer-A install for frame-hash
  determinism), `-DSKIP_TITLE=1` (seeds the guestbook fields -- PLAYER,
  noon, 0-0-0 -- instead of waiting on getKey for a name, a date and a
  time, so an unattended run reaches gameplay) and `-DSKIP_COPYPROT=1`
  (see below).  Pass them through `ALCYON_CPPFLAGS`, and rebuild from
  clean afterwards -- a stale object from a gated build would silently
  break byte identity.  alcyon_build.sh now records the flags in
  `build/alcyon/CPPFLAGS` and alcyon_link.sh SKIPS the BSS remap when
  that file is non-empty: the layout spec describes the shipped build,
  and remapping a gated one fails on the first site the spec has never
  seen.

- **Do NOT combine `-DSKIP_MIDI=1` with a long gameplay run.**  Without
  the Timer-A ISR the mq_* engine never completes a record, the
  resident retries the activity, and each retry's `Fsfirst("*.sng")`
  leaks a TOS folder buffer until GEMDOS halts with "OUT OF INTERNAL
  MEMORY -- SYSTEM HALTED" -- measured at ~75 s of gameplay (~4000
  VBLs).  frame_hash.sh keeps SKIP_MIDI and stays under that ceiling
  (2000 VBLs); test_longrun_stable.sh drops it.

- **`-DSKIP_COPYPROT=1` is what makes the game playable under an
  emulator.**  It replaces main's `cprot_r = cp_main();` with a
  non-zero constant, so cs_mvIn and gameLoop take their normal paths
  instead of parking the resident in `while (1) a_sleep(-1);`.  It
  skips the CALL, not the check, which also avoids the FDC wait that
  never terminates when the program was launched from a drive that is
  not the floppy.  With it the move-in cutscene runs, the resident
  walks the house, uses the stairs, changes clothes and sits down to
  read -- none of which the shipped build will do on any emulator
  here.

## The host build (`cd source && make`)

Compiles every .c with the host cc as a syntax/semantic check; it does
not produce a runnable ST binary.  `make linktest` additionally links
all 32 objects.  Both work again as of 2026-09-05.

What it takes to keep clang and Alcyon reading the same source:

- **`include/hostgem.h`** stands in for the four DK headers the port
  includes but the host has no copy of -- `<vdibind.h>`, `<ostruct.h>`,
  `<gembind.h>`, `<obdefs.h>`.  Every one of those includes is wrapped
  `#ifdef HOST` / `#else`, so the Atari build is untouched.  MFDB and
  _DTA use `short` where the ST headers say `int`: Alcyon's int is 16
  bits, and with the host's 32-bit int these would be the wrong size
  and the host build would stop being a check on the real layout.
- **`hostasm.c` and `savehost.c`** supply the symbols the five .s files
  and the GEMDOS/BIOS traps provide on the ST.  alcyon_build.sh skips
  both BY NAME, so they cannot reach the shipped binary.
- **`SOURCES` is derived, not hand-written**: every .c except the
  unity-unit constituents named in columns 2+ of `tools/stx_units.txt`.
  The old hand list still had midi_seq.c in it after the globals.c
  fold, so it went into the link twice.  (Beware: a literal `#` inside
  `$(shell ...)` starts a make comment and swallows the closing paren
  -- the awk in that line uses `\043`.)
- **Four warnings are turned back into warnings**: implicit function
  declarations, implicit int, falling off the end of a non-void
  function, and pointer-to-long assignment are all legal Alcyon and
  hard errors in modern clang.  The port relies on the third
  deliberately (pk_cace, chk_timA).
- **The lvalue cast has no clang spelling.**  `(char *) p += n;` is what
  emits `add.l d0,mem`, and clang cannot parse it at all, so its six
  sites (main.c, sc_firw.c, sc_firsb.c) carry an `#ifdef HOST`
  alternative beside the shipped line.
- **main() is `lcp_main()` on the host** so the unit tests can supply
  their own; it now lives inside stx_u1.o and cannot be left out.

Declaration fixes that fell out of it, all verified byte-neutral:
`pk_dchd` was forward-declared `void` and defined `short`, `pk_show`
had no declaration before its first call, `al_loal` was declared
`void`, `fr_read`'s definition had no return type at all, and
`erChr`/`stEnter` were declared nowhere.  stx_u1 gained alerts.h,
assets.h and render.h; stx_u3 gained gfx_prim.h.

**`make test` runs all ten unit tests and they pass** (2026-09-05).
Two things to know before touching them:

- **The host is little-endian and the loaders are not.**  sf_sl and
  al_loal read their length fields with raw two-byte fr_reads, so 34
  arrives as 8704 and the file walk is lost after the first block.
  That is faithful ST code.  t_sounds.c and t_assets.c therefore write
  a HOST-ENDIAN copy of the asset -- same payloads, same order, only
  the length fields swapped -- so the loader's LOGIC is what is under
  test.  Do the same for any new test that drives a real asset.
- **Not everything is checkable here.**  chk_encm walks g_ew2a until
  `table[0] == 0xff`, and Alcyon narrows that constant to a signed
  char so the sentinel matches; clang does not, so on the host the
  walk runs off the end of the table.  t_parser.c reports that case
  instead of asserting it.

`tests/reference/sprite_golden.pgm` was re-blessed on 2026-09-05: the
old master came from the retired LCP_ORG revision, and the sprite path
is now the byte-identical LCP_STX code.  A sprite test must call
`initBRev()` -- rev_tab is BSS here and built at boot, where it used to
be a shipped table, and without it every mirrored frame renders blank.

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

    source/tools/sync_ghidra_names.sh          # `verify` = read-only

**Ghidra must be CLOSED for it** -- it holds an exclusive lock, and
clearing a live one is how the database gets corrupted.  See "Syncing
names to Ghidra" further down for the address/truncation traps.

(`apply_ghidra_renames.sh` and `gen_ghidra_rename_tsv.py` were deleted
2026-09-06.  They POSTed to a Ghidra HTTP server on :8089 and needed
`RenameLcpGlobals.java`, `list_data_symbols.java` and a fresh
`/tmp/ghidra_syms.txt` -- none installed, and the endpoint does not
answer.  This paragraph used to present them as the way in.)

Struct name/field syncing uses the same HTTP mechanism via
`~/ghidra_scripts/RenameGhidraStructs.java` and a
`lcp_struct_rename_map.tsv`.

## Where the reference came from

The protected Pasti image `Little Computer People.stx` (repo root) was
extracted with `source/tools/stx_extract.py` -- an 80-track
single-sided FAT12 volume whose 32 DATA/ files are all byte-identical
to the repo's, plus the UNCRACKED `LCP.PRG` (123 352 bytes: text
104 156 / data 12 260 / bss 187 450), checked in as
`DATA/LCP_STX.PRG`.  It carries the playable minigames, an Xbtimer
(Timer-A) install, and a real copy-protection region.

**Ghidra correspondence:** `LCP.rep` contains two programs; the larger
one (`LCP.PRG.1.1`) IS LCP_STX loaded at BASE 0x10000.  Old port
comments citing addresses like `mq_tick 0x1219a`, `gameTick 0x256a6`,
`main 0x15546` or `st_titl 0x16de6` resolve as
`addr - 0x10000 = LCP_STX text offset`.

All comparison tools (verify_bytes.py, prg_diff.py, fn_diff.py,
stx_map.py) honour `LCP_REF=<path>`; the default is
`DATA/LCP_STX.PRG`.

## Recovering LCP_STX's C sources

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
  functions.

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

Structural rules of this build:
- **sf_sl is a REAL SOUNDS.LCP block loader** (fr_read sizes, Malloc
  per block, store into mi_ntLp at 0x43f7a = Ghidra 0x53f7a - 0x10000,
  er_nomem on failure).
- **GEMDOS binding lives at text 0x11a** (right after the alcyon2
  gemstart) and calls use MINIMAL argument shapes -- `Fclose(h)`
  pushes just the opcode+handle, no 3-arg padding.
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
  of this partition.  The port instead builds ~7 "unity" translation
  units that #include the constituent .c files in STX order.  The
  moves already committed
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
  and showLcp now have shared bodies in parts/, included by stx_u2.c
  at their STX addresses.  A
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
  body_ptr[frame] / body_shp[frame].  Same class as the
  g_obtmp->g_obtmt and
  od_* fixes -- expect more of these wherever the port carries a
  pointer variable that STX addresses as an array.

  **The VDI binding layer differs in three ways** (all six simple
  bindings recovered together): STX assigns intin[0] BEFORE filling
  contrl, RETURNS intout[0], and reaches the dispatcher with a jsr --
  i.e. it lives in another object.  vdistx_a.s supplies it as a
  separate object (see the byte-identity phase notes below).

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
   2. Add unity units (stx_u*.c) that #include their cluster's .c
      files; alcyon_build.sh compiles the units instead of the
      constituents.
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
  in STX order -- but the match count did NOT move (112 before and
  after).  Unity units are necessary
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
  source differences and must be recovered as such, and -- valuable
  side result -- the rebuilt
  toolchain is codegen-equivalent to the one that built LCP_STX, so
  the campaign never needs to run the period compiler under Hatari.

  **sf_sl is byte-recovered (2026-09-01).**  The SOUNDS.LCP block
  loader matches LCP_STX byte for byte at 0xdcc4.  The real
  1985 loader declares `fhandle, size, block, index` in that order,
  assigns the Malloc result straight into `mi_ntLp[index]` and reads
  it back into `block`, widens with `(long) size + 4` (not
  `(long)(size + 4)`), and walks the buffer with `block++` before
  the payload read.  So the port's sound loading is not a
  reconstruction: it is the original's code.

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
  where the port had it 40 bytes away.  Moving it fixed it.  Known games-object order from matched addresses:
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

  **Recurring source-shape rules.**  Each row is `wrong shape  vs
  what LCP_STX actually has` -- the left column is the spelling the
  port had before the rule was found, kept because the contrast is
  what makes the rule usable.  Do NOT mass-apply them: gate per site,
  when a fn_diff shows it.

      i = i + 1            (no)   vs  i++            (STX)
      x = x - 5            (no)   vs  x -= 5         (STX)
      *idx = *idx + 1      (no)   vs  (*idx)++       (STX)
      do{d--;if(!d)break;  (no)   vs  while(--d){body;
        body}while(cond)              if(!cond)break;}
      r = f(); if (r)      (no)   vs  if (f())       (STX)
      while (A && B){}     (no)   vs  while(A){if(!B)break;}
      unsigned short i     (no)   vs  short i        (STX)
      BOOL16 flag          (no)   vs  char flag      (STX)
                                      (tst.b at the use sites --
                                       mi_play, mi_dvel, psg_dvol)
                                      (no clr.w zero-extension
                                       around index arithmetic)
      mask in the loop     (no)   vs  folded into the assignment,
        condition                     computed once
      gameTick(3)          (no)   vs  t = 3; ... gameTick(t)
      if (x == 3) A else B (no)   vs  if (x != 3) B else A
      lcp_face = c ? L : R (no)   vs  the assignment duplicated
                                      inside both branches
      unsigned bound       (no)   vs  signed (bcs vs blt on the
                                      loop comparison)
      p = f(); q = p & 3   (no)   vs  p = f() & 3
      if/else-if ladder    (no)   vs  switch (Alcyon puts the
                                      compare chain at the bottom)
      w = f(); g(w)        (no)   vs  g(w = f())
      while (n != 0){...   (no)   vs  while (n--) { ... }
        n = n - 1;}                   (load into d0, subq to memory,
                                       test the OLD value in d0)
      if (c) f(); break;   (no)   vs  if (!c) break; f(); break;
                                      (the beq displacement gives it
                                       away: over the call vs to the
                                       switch end)
      case ORDER in the source is recoverable from the jump table's
        targets (STX's execEv writes BOOK, RECORD, FOOD, PHONE,
        GET_DRESSED, DOG_FOOD)
      statement ORDER of two initialisations is evidence too
        (a_hello clears pick before prev_pick in STX)
      (Random() & 7) + 293 (no)   vs  (int)(Random() & 7) + 293
                                      (the cast makes the ADD word-
                                       sized: and.l then add.w, where
                                       the uncast form adds long)
      f((x & 15) | 1)      (no)   vs  f((int)((x & 15) | 1))
                                      (uncast, Alcyon computes in the
                                       argument slot: move.l d0,(sp) /
                                       andi.l #15,(sp))
      RECT16/short[4]      (no)   vs  short pts[10] -- the STX TV
                                      routines all carry a 10-short
                                      point buffer and write all four
                                      coordinates
      x = x + 1; if (x > N) (no)   vs  if (++x > N)  (the pre-increment
                                      form loads the value back for
                                      the compare: add.q then move to
                                      a register, where the compare on
                                      the plain variable is cmpi #N,mem)
      a = 0; b = 0; c = 0; (no)   vs  a = b = c = 0 (one value computed
                                      into d0 and stored right to
                                      left -- the FIRST store is the
                                      RIGHTMOST variable, so the store
                                      widths give the declaration
                                      types away)
      helper C function    (no)   vs  hand-assembly (psg_wr, psg_mix
                                      and mowrit are frameless
                                      absolute-long pokes in LCP_STX;
                                      source/psg_asm.s carries them)
      f(&d, &c, &b, &a)    (no)   vs  f(&a, &b, &c, &d)
      short table          (no)   vs  char table (sf_pri, g_mcpro),
                                      and mi_nxTk/mi_lpTk are LONG
                                      tick counters
      int field             (no)   vs  unsigned field (clr.w before
                                      every load -- cpyScr's MFDB
                                      extents; `(unsigned short) f`
                                      at the use site reproduces it)
      p->fd_addr = addr;   (no)   vs  the address latched, masked to
                                      its high half and stored as two
                                      words through a (short *) cast
                                      (sp_iniM)
      } else {             (no)   vs  } else if (arg != 0) {
                                      (a redundant re-test of the
                                       argument already implied by the
                                       else -- a_opcfd/a_opecc/a_opecd,
                                       and er_food re-tests g_dvdog)
      one assignment       (no)   vs  an if/else whose two arms assign
                                      the SAME value (stpScrB's size)
      p = (T *)((char *)p    (no)   vs  (char *) p += n;
        + n)                          (Alcyon C 4.14 accepts a CAST
                                       AS AN LVALUE; the compound form
                                       emits add.l d0,mem where the
                                       assignment form emits
                                       add.l mem,d0 / move.l d0,mem --
                                       sc_firw/sc_firs/sc_firb)
      (long) row * 160     (no)   vs  row * 160  (muls.w + ext.l
                                      instead of a call to lmul)
      for (i=0; s[i]; i++) (no)   vs  while (dst[i++] = *src++ & 0xff)
        dst[i] = s[i];                (v_gtext, and contrl[3] = --i)
      case X: return V;    (no)   vs  case X: return V; break;
                                      (the dead break emits a second
                                       branch; the LAST arm has none
                                       -- getKey)
      if (c) return v;     (no)   vs  if (c) return v; else <stmt>
                                      (the else-skip branch shows up
                                       even though the then-arm
                                       returns)
      for(;;){...if/else}  (no)   vs  a label + two explicit `goto`s
                                      (a loop whose arms each branch
                                       straight back, with no shared
                                       loop-back branch: fOpen,
                                       fr_read)
      void f()             (no)   vs  short f() returning the value
                                      the ORG version discards
      for (;;)             (no)   vs  while (1)  (the while form
                                      emits an entry bra to the
                                      bottom jump; for(;;) does not)
      x <<= 9              (no)   vs  x = x << 9  (<<= loads the
                                      shift count first)
      short table          (no)   vs  char table (moveb + extw at
                                      the use sites -- sf_pri)
      a static helper      (no)   vs  the body written out at each
                                      call site (dv_pick)
      if (a <= b)          (no)   vs  if (b >= a)  (which operand
                                      lands in d0 gives it away)
      Setscreen(l,p,-1L)   (no)   vs  Setscreen(l,p,-1)
      Giaccess(0L, 0x88L)  (no)   vs  Giaccess(0, 0x88) with the
                                      alcyon2 header's (char)/(short)
                                      argument casts
      lcp_y = lcp_y + 9    (no)   vs  lcp_y += 3; ... lcp_y += 6;
                                      (STX splits the step around the
                                       state assignment -- two subq/
                                       addq to memory, not one addi)
      p[i - 1]             (no)   vs  *(p + i - 1)  (writing the
                                      offset arithmetic inside the
                                      dereference makes the POINTER
                                      the index register and the
                                      counter the base -- mq_pacm)
      short locals for     (no)   vs  globals (aes_ini's graf_handle
        out-parameters                metrics; vdi_init's work_in /
                                      wk_out -- the frame collapses
                                      from -150 to -6)
      short *tab[3] = {r0, (no)   vs  short tab[3][8] and `tab[i][j]`
        r1, r2}                       (two ext.l and a trailing
                                      `add.l #base` -- chk_timA)
      x = x - 1;           (no)   vs  x--; if (x <= 0)   (subq to
        if (x < 1)                    memory then an explicit tst)
      x = x - 1;           (no)   vs  if (--x == 0)      (the subq's
        if (x == 0)                   own flags, no tst at all)
      t = f(); if (t == v) (no)   vs  if (f() == ++v)    (the call
                                      result stays in d0 across the
                                      addq to memory -- gameSim1)
      C helper             (no)   vs  hand-assembly: blkcp32 is an
                                      unrolled `dbf` loop of eight
                                      post-increment long moves
                                      (source/blkcp_a.s)
      two early returns    (no)   vs  one compound `if (a && b) {...}`
                                      followed by `break` (deal_kc's
                                      default arm -- the guards branch
                                      to the break, not the epilogue)
      arr[i] = v           (no)   vs  *(i + arr) = v  (naming the
                                      index first folds the base into
                                      `add.l #base,An` instead of a
                                      second address register)
      `return v;` in every (no)   vs  NO return at all in the arms:
        arm of a ladder               the ladder's jump to the
                                      function end leaves the last
                                      compared value in d0 (chk_timA)
      x = 4;               (no)   vs  x == 4;  -- a real 1985 typo in
                                      gameSim1's sickness clamp; the
                                      compiler emits the comparison
                                      and discards it, so the clamp
                                      never happens.  Preserved.
      Random()             (no)   vs  rnd(), a global wrapper around
                                      the bare XBIOS call that STX
                                      routes seven call sites through
                                      (rndRng still inlines the trap)
      moff only            (no)   vs  moff AND mon -- STX has the
                                      mouse-show counterpart right
                                      after it
      rev_tab as DATA      (no)   vs  built at run time by a 94-byte
                                      routine behind a 10-byte wrapper
                                      (STX 0x6804/0x680e -- not yet
                                      ported)
      no resolution check  (no)   vs  vdi_init splits in two: the
                                      opener checks wk_out[0] and hangs
                                      on a "[1][Must be in|low
                                      resolution.][REBOOT]" alert loop
                                      before bsr.s-ing into the
                                      attribute/clear half
      arr[i] != '\0'       (no)   vs  arr[i]  -- a BARE truthiness
                                      test is what makes Alcyon reach
                                      the array with an indexed EA
                                      (movea.w idx,a0 / movea.l
                                      #base,a1 / tst.b (0,a0,a1.l));
                                      the explicit `!= '\0'` emits the
                                      base+add form (ag_main)
      c = *p; if (c==' ')  (no)   vs  while ((c = *p++) == ' ')
                                      (Alcyon saves the flags across
                                       the pointer increment:
                                       cmp / move sr,d0 / addq to
                                       memory / move d0,ccr / branch)
      x = x + 1; if (x<N)  (no)   vs  if (x++ < N)  -- the same
                                      flag-save trick around a global
                                      (ag_main's guess counter), so
                                      BOTH arms see the increment
      pk_dppm();           (no)   vs  pk_dppm;  -- a real 1985 typo in
                                      pk_wrMn: the parentheses were
                                      left off, so Alcyon just emits
                                      move.l #_pk_dppm,d0 and drops
                                      it.  Preserved.
      switch with default: (no)   vs  no default arm -- the
                                      out-of-range branch goes to the
                                      switch END, not to a body
      a = f(); b = g();    (no)   vs  if ((a = f()) > (b = g()))
        if (a > b)                    (both values stay in d0/d1, so
                                      the compare is register-to-
                                      register with no reload)
      for(;;){...continue} (no)   vs  a label + `goto` -- `continue`
                                      branches to the loop's BOTTOM
                                      edge, a goto branches straight
                                      to the label, and the two are
                                      distinguishable whenever the
                                      loop is longer than a short
                                      branch (pk_wrMn)
      (long) x             (no)   vs  (long) x & 0xffffL  (the cast
                                      forces ext.l into d0 followed by
                                      and.l; without it Alcyon just
                                      sign-extends through an address
                                      register -- sf_irqp)
      short table row      (no)   vs  12-byte WORD_TO_ACTION: ten
                                      signed mask bytes, the action id
                                      at +10 and the priority at +11,
                                      with ew2pos/g_ew2b/bm_lo/g_ewb
                                      all char (chk_encm)
      concat22/rd_hz       (no)   vs  written out inline: sf_irqp
        helpers                       builds the 32-bit duration from
                                      its two halves and keeps its own
                                      Super block, with an UNUSED
                                      short in the frame between the
                                      counter pointer and the value
      Giaccess(d, r)       (no)   vs  xbios(28, d, r) written out --
                                      mq_dise's PSG writes skip the
                                      macro's (char) cast on the data
                                      argument (sf_so still uses the
                                      macro, so do NOT change it)
      x &= 0xf             both       -- but `x = x & 0xff` is a
                                      DIFFERENT shape (load/and/store
                                      vs andi to memory); mq_dise uses
                                      each in the same function
      a = 1; b = 1;        (no)   vs  b = a = 1  (moveq into d0, then
                                      both stores from the register)
      char msg[] = "..."   (no)   vs  char *msg = "..."  (pk_bm and
                                      pk_rm are POINTERS, so every
                                      patch is movea.l var,a1 first)
      x-- < 1              (no)   vs  x-- <= 0  -- `< 1` lets Alcyon
                                      compare in memory and save the
                                      flags across the decrement,
                                      `<= 0` loads the old value into
                                      d0 and tst's it (psg_upEn)
      (char)((int) x / 10) (no)   vs  x / 10 + '0'  -- the (int) cast
        + '0'                         adds an ext.w the original has
                                      nowhere
      short ikey (a local) (no)   vs  a GLOBAL (pk_bjMn's key variable
                                      is addressed absolutely)
      CARD_NONE == -1      (no)   vs  255 -- but pk_rmch's empty-pile
                                      return is a plain -1
      if (A || B || C) {}  (no)   vs  if (!A && !B && !C) goto <next>;
                                      followed by the body unnested --
                                      the minigame mains invert every
                                      guard into a goto
      for (i = 0;          (no)   vs  for (i = 0; i < 5; i++) {
        i < 5 && a[i] != N;             if (a[i] == N) break; body; }
        i++) body;
      dealer stands > 16   (no)   vs  >= 17 (the literal in the cmpi
                                      is 17, not 16)
      p_sfgrt then p_sfspe (no)  vs  p_sfspe then p_sfgrt (a_hello's
                                      random arm -- the wrapper ids
                                      settle which is which)
      } else if (k == 1) { (no)   vs  goto next_round; } if (k == 1) {
                                      (the minigame mains close each
                                      arm with an explicit goto and
                                      start a fresh if; sizes are the
                                      same, only the branch TARGET
                                      differs -- the chain end vs the
                                      loop label)
      ...} goto next_round; (no)  vs  ...goto next_round; } (the
                                      trailing goto INSIDE the deepest
                                      block, which makes every
                                      end-of-if jump target the
                                      epilogue instead of the goto)
      x < 12 && x > 7      (no)   vs  x <= 11 && x >= 8  (the literal
                                      in the cmpi and the branch
                                      condition both change)
      while ((*p)-- != 0)  (no)   vs  while ((*p)--)
      short off = g*6      (no)   vs  arr[i + g * 4] written inline --
                                      `arr[idx + g * 4]` is exactly
                                      what emits moveaw g,a0 / addaw
                                      a0,a0 / addaw a0,a0 / addaw idx
                                      (probe-confirmed); a temp with
                                      the same arithmetic does not

  **Compare the `link #-N` frame size FIRST.**  It says exactly how
  many locals the function really has, before touching anything:
  a_wandi needed an UNUSED local the port lacked, a_getd reuses one
  variable as its loop counter, and a_tidyh/a_playp/a_wakum have
  NONE because every call result is consumed in place.  Removing a
  declaration without checking every use breaks the build (a_tidyh
  used `result` twice).

  **An unbalanced conditional does not fail the build.**  cp68 does
  NOT error on an unclosed `#ifdef` -- it silently drops everything
  from there to EOF, so the build reports OK while whole functions
  disappear (hit in aleisure.c, 2026-09-02).  `stx_check.sh` runs
  `tools/ppbalance.py` first; run it after any batch edit.

  **Local offsets are assigned in DECLARATION order**, so the frame
  offsets in a fn_diff pin the declaration list exactly.  Declaration
  ORDER is evidence too: the frame offsets pin it (a_driwa
  is rnd, counter, last_pick, pick in STX; the port had rnd, pick,
  last_pick, counter).  And STX's a_driwa never initialises last_pick
  -- the first comparison reads whatever the slot held.  Preserved as
  written; do not "fix" such things.
  a_sitae alone needed six of these; expect several per function.
  **A parts/ file may hold several functions only if they are
  adjacent in LCP_STX.**  The four p_sf* wrappers are one file each,
  because their order (tvc, spe, hnd, grt) had to be discovered from
  the sound ids in their bodies.

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
  final `mi_dvel < 0x80` arm that looks dead (Alcyon
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

**Status (2026-09-03): the TEXT segment is BYTE-IDENTICAL to
DATA/LCP_STX.PRG -- 104 156 bytes on both sides, zero differing bytes
modulo relocations.  All that remains is the data (+3008) and bss
(+53884) layout; see "Byte-identity phase" below.**

**cp_main is Activision's, not LCP's: 97.2% of it is byte-identical to
THE MUSIC STUDIO** (found 2026-09-06).  Extract the Music Studio ST
disk with the project's own `stx_extract.py` --
`Retro/Atari ST/music_studio_activision_(usa)/*.stx` -- and hash 32-byte
windows of its `AUDIO.PRG` text against LCP_STX's.  10 063 bytes are
shared, and the largest single run is **6 904 bytes, LCP_STX
0x02514-0x0400C**, which ends exactly at 0x400c where cp_asm stops and
stx_u1 begins.  Of cp_asm's 7 499 bytes, **7 289 are shared**; the 210
that are not sit in five islands (64 at the entry, then 41, 89, 13, 3),
and the 89-byte one is about the size of the self-decrypting block
described below.  So the protection is a shared Activision routine
with per-title constants, which independently corroborates that this
region is hand assembly rather than compiled C.

Two related results from the same comparison, and the sharing is
per-FUNCTION rather than a vague total:

      _cp_main    7290 / 7500   97.2%
      _mq_bust     344 /  466   73.8%
      _mq_dise     556 / 1258   44.2%
      _mq_pars      45 /  662    6.8%
      _psg_wr       78 bytes
      _sf_irqp       0 /  456    NOTHING

So LCP's **MIDI sequencer really does descend from Music Studio's
player** -- mq_bust, the scale-table builder, is three-quarters
identical and mq_dise, the MIDI-out dispatcher, nearly half -- which
is the lineage the shared .SNG format only implied.  But the
**sound-EFFECT engine does not**: sf_irqp shares ZERO of its 456
bytes, and the whole game-code span 0x400c-0x1733a shares nothing at
all.  That is why Music Studio cannot settle the g_sfDoB size question
-- the function that copies into that buffer is LCP's own code.
Everything else shared is DRI library, expected of two Alcyon builds.

**The comparison is a tool: `source/tools/xbin_diff.py`.**  Give it a
PRG or a whole .stx (it runs stx_extract.py itself) and it reports
shared bytes, coverage PER REFERENCE FUNCTION from lcp_sym.68k, and
the longest runs.  Judge by the coverage column, never by run length:
relocated longwords hold absolute addresses that two binaries never
agree on, so a shared function is chopped into a run per relocation
site.  Matches at or above 0x1733a are DRI library and expected;
matches BELOW it are the finding.

**The baseline, so the numbers mean something** (2026-09-06).  Run
against the period Alcyon toolchain's own binaries -- DRI-built ST
programs with no relationship to LCP whatsoever:

      C168.PRG      4347 shared, but only  68 below 0x1733a
      CP68.PRG      4177 shared, but only  68 below
      DOODLE.PRG     219 shared,            0 below
      COMMAND.PRG      0 shared,            0 below
      AUDIO.PRG    10055 shared,         8614 below   <-- Music Studio

So an unrelated Alcyon program shares 0-68 bytes of non-library code
with LCP_STX, and every one of C168's matches is libc (__doprt,
_malloc, __flsbuf, _free).  Music Studio's 8614 is ~125x that.  The
relationship is real and not an artifact of shared tooling -- which is
the question a reader should ask first, and now does not have to.

**Other Activision ST titles are NOT reachable here** (checked
2026-09-06).  Music Studio is the only one on this machine as a usable
image.  PaintWorks, Ghostbusters II, Fighting Soccer and Hacker II
exist only as GreaseWeazle FLUX dumps (`.scp` inside `.7z`, under
`Retro/Floppies and HDs/.../Atari ST Stream Images`), and neither
converter installed here will do it headlessly: `gw convert` dies with
`'bitarray.bitarray' object has no attribute 'itersearch'` (bitarray
3.x dropped it), and HxCFloppyEmulator / UnifiedFloppyTool are GUI
only.  Pinning an older bitarray would fix `gw`, but that is the
maintainer's environment to change.

cp_main (0x22c0-0x400b) is recovered as **hand-written assembly**
(`source/cp_asm.s`), not C -- the maintainer confirmed the whole
region is assembly, and it proves out: the object is entirely
self-contained, all 21 of its relocations pointing back inside
itself.  It saves d1-d7/a0-a5 into its own static block rather than
the stack, goes supervisor, sets TOS's flock byte at $43e, decrypts
96 bytes of itself in place (keyed by the current drive number) under
a raised interrupt mask, drives the 1772 FDC directly through
$ff8604/$ff8606 and the DMA address registers $ff8609/$ff860b/
$ff860d to restore-seek-read the protected track into a 6560-byte
buffer that lives INSIDE the text segment, then re-encrypts itself,
clears flock and restores the registers.  Its long return value is
assembled by six mutually recursive stubs that each add a constant --
obfuscation, not arithmetic.  main stores it in cprot_r and cs_mvIn
parks the resident in `while (1) a_sleep(-1);` if it is zero.

**Assemble cp_asm.s with `as68 -n`.**  as68 shortens branches on its
own and ignores an explicit `.w`; the original picks bsr.w in places
where bsr.s would fit (cpnxt's first call, +0x7e), so branch
optimization must be OFF and every branch in the file carries its own
size suffix.  Note also that `stx_check.sh`'s object list must mirror
`alcyon_link.sh`'s exactly -- if the symbol side-link disagrees with
LCP.PRG about object order, every symbol extent comes out wrong and
the sweep reports mass divergence that is not real.

st_titl (0x6d7e) and cs_mvIn (0xe500) were written from scratch and
both match.  st_titl brought three helpers the port never had:
plErCol (0x871a -- plEr with an explicit fill colour, the four VDI
attribute calls written out instead of initVdi/exitVdi), erChr
(0x72e6 -- blank one 8x8 character cell) and stEnter (0x718e -- the
fixed-width numeric field reader, which skips every third column so
the separators in MM/DD/YY and HH:MM cannot be typed over).

**The minigame mains share one skeleton** (pk_main and pk_bjMn both
match it): a `goto round; next_round: gameTick(0x18); round:` label
loop rather than a loop statement, the Mfree/moff cleanup written
INSIDE the first exit test with every other exit `goto`-ing into it,
each key read as its loop's condition, and pk_inph's idle sentinel
passed as 255 rather than 0.

lcp_path's three static helpers (wkCyc, setHTgt, stairCyc) do NOT
exist in LCP_STX -- their bodies are written out at all sixteen call
sites -- and every stair state machine WRAPS to its F0 frame instead
of clamping at F3S.  Expect more of both.

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
  can still sweep the NEXT function (or a following
  `#include "parts/..."` line, whose relative path then no longer
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

## GOAL ACHIEVED (2026-09-03): BYTE-IDENTICAL to LCP_STX.PRG

**The build reproduces `DATA/LCP_STX.PRG` exactly -- MD5
eae52d14023b51d7ac459a90d37eed10, 123 352 bytes: text 104 156, data
12 260, bss 187 450, relocations 6 908.**  Reproduce from a clean tree:

    rm -rf source/build/alcyon
    source/tools/alcyon_build.sh
    source/tools/alcyon_link.sh
    LCP_REF=DATA/LCP_STX.PRG python3 source/tools/prg_diff.py

The final step is `tools/bss_remap.py`.  lo68 and the 1985 linker pack
the same `.comm` blocks at different offsets (and the original does not
even align them -- scrbufA lands on an odd address), so TEXT, DATA and
the relocation stream come out identical while every relocated BSS
longword points somewhere else.  The original allocation is carried as
a checked-in spec, `tools/stx_bss_layout.tsv` (417 rows, port
symbol+offset -> address, bss size in the header).  alcyon_link.sh
takes a second SYMBOLS link as `lcp_sym.68k`, resolves the spec against
it, and rewrites the 3 842 affected sites; **the reference binary is
not read at link time.**  `bss_remap.py --gen` regenerates the spec
after a layout-affecting change, and its site-by-site pairing is the
proof that the port's reference structure matches the original's.

Keep the invariant: run prg_diff.py after any change.  Regenerate the
spec only when the layout really moved, and read the diff.

**prg_diff.py is not enough on its own.**  It compares the binary AFTER
bss_remap has rewritten every relocated BSS longword, so a wrong
variable reference is invisible to it -- as it is to verify_bytes and
stx_txtdiff, which wildcard relocated longwords outright.
`source/tools/reloc_audit.py` closes that hole: it pairs the two
relocation streams site by site (they are identical, because text and
data are) and reports six ways the port's variable structure can
disagree with the original's -- merges, aliases, split symbols,
symbols nothing relocates against, symbols whose base is only
INFERRED, and arrays declared larger than the original's storage.  Run
it after any change that touches a global.  What it should report:

    A merges          0
    B aliases         0
    C split symbols   0
    D unverifiable    0
    E inferred bases  1   scrbufA
    F over-declared   0

**scn_cmn was not real** (closed 2026-09-05).  Ghidra's own main
settles it: the 30-byte dictionary read is
`move.l #0x2c6ce,(SP) / move.l #0x1e,-(SP) / bsr fr_read`, and 0x2c6ce
is link-time 0x1c6ce -- scn_dic.  The `@ 0x4cf7c` in the port's old
comment was simply a wrong address: 0x3cf7c is two bytes INSIDE
scn_buf and would run over mf_scrp, g_inpmd and g_ltscb, every one of
them a symbol real relocations point at.  One object, two names, and
the unreferenced one is gone.

**Stale LABELS in Ghidra are not a different program.**  The open
project decompiles main with calls to `unScn`, `fLoad` and `lcp_load`,
which LCP_STX does not have as functions -- and that looks exactly
like the retired image until you check the bytes.  It is not: the
disassembly at 0x15546 matches our byte-identical build at text 0x5546
instruction for instruction.  Those were LCP_ORG-era NAMES still
attached to LCP_STX functions.  When a Ghidra name contradicts the
port, disassemble and compare bytes before concluding anything about
which revision you are looking at.  (Renamed 2026-09-05: unScn ->
scn_dec, fLoad -> al_loal, lcp_load -> lc_load, al_lost -> ldSpr,
sp_reglp -> sp_regs.)

**Syncing names to Ghidra: use `source/tools/sync_ghidra_names.sh`.**
The old `apply_ghidra_renames.sh` needed `RenameLcpGlobals.java`,
`list_data_symbols.java` and a fresh `/tmp/ghidra_syms.txt` -- none of
which are installed -- and it POSTed to `/run_script`, which this
plugin does not serve.  The replacement drives `analyzeHeadless` with
`tools/ghidra/LcpSyncNames.java`: no server, no GUI, works on a closed
project.  `sync_ghidra_names.sh verify` re-runs it read-only.

**But for ONE symbol, do not close Ghidra -- talk to the plugin over
HTTP** (established 2026-09-06).  The GhidraMCP plugin answers on
:8089 while Ghidra is OPEN, which the headless script cannot be.  It
takes `POST /<tool_name>` with a JSON body, and an earlier note here
that there is "no data-symbol rename endpoint" is WRONG -- the MCP
*tool* surface has none, but the plugin does:

    # what is actually at that cell, before touching it
    curl -s -X POST -H 'Content-Type: application/json' \
         -d '{"address":"0x2b6d6","length":2}' \
         http://127.0.0.1:8089/analyze_data_region
    # -> current_name, current_type, xref_count, xref_map

    curl -s -X POST -H 'Content-Type: application/json' \
         -d '{"address":"0x2b6d6","newName":"pat_ok"}' \
         http://127.0.0.1:8089/rename_data

Note `newName`, camelCase, where the address parameter is `address`;
`new_name` is silently null and the call fails with a Java NPE.  Other
live endpoints: `rename_or_label`, `create_label`,
`batch_create_labels`, `list_data_items`, `get_version`.  Endpoint
names match the bridge's tool names (`~/GhidraMCP/bridge_mcp_ghidra.py`
is the list); GET returns 404 on almost all of them, so probe with
POST.

`analyze_data_region`'s **xref_count is a free sanity check** and it
earned its keep: after correcting the action-table rows it showed
g_trel with 18 xrefs (everything tests `g_trel[0]`) against 1 each for
g_atact/g_atmod/g_atrel (indexed once apiece in airandom.c) and 1 each
for g_obala/g_obcla/g_obpha (one od_draw call apiece) -- Ghidra's own
analysis agreeing with the corrected pairing.  The plugin also warns
that a global "must start with g_"; that is ITS convention, not this
project's, and does not apply.

**The verify's expectations live OUTSIDE the repo and DRIFT.**
LcpVerifyNames.java reads `~/ghidra_scripts/lcp_verify.tsv`, which is
generated by hand during a sync session and is not version-controlled.
Renaming a port symbol therefore makes `verify` report a FALSE
mismatch until that file is edited too -- renaming dg_petok to pat_ok
produced exactly that (`want=dg_petok got=pat_ok`, ok 758 -> 757),
with Ghidra being the CORRECT side.  When verify flags one symbol and
that symbol was renamed recently, fix the TSV, not the database.

Ghidra MUST be closed -- it holds an exclusive lock, and clearing a
LIVE lock is how the database gets corrupted.  The script refuses
rather than fight over it.  If a SIGTERM ever leaves `LCP.lock` and
`LCP.lock~` behind with no process running, those are stale and safe
to delete; verify afterwards by reopening headless before committing
the database.

Three things to get right when building the rename list:
  * **Ghidra address = link address + 0x10000.**
  * **For BSS, lcp_sym.68k's address is lo68's, not the reference's.**
    Take it from `tools/stx_bss_layout.tsv`, which is where the remap
    actually puts the symbol.  DATA and TEXT are byte-identical, so
    their link addresses are the reference's already.
  * **lcp_sym.68k carries 8-char TRUNCATED linkage names.**  Pushing
    those gives Ghidra `lcp_pat` for `lcp_path` and `aes_ini` for
    `aes_init` -- an earlier sync did exactly that.  Expand them
    against the `extern` declarations in `include/*.h` first.

Rename data symbols BY ADDRESS, not by name, wherever the port's own
name has changed -- and always where two names were SWAPPED.  Going by
name there chases a symbol that has moved or collides with the name
still held by the other cell.  That is how the g_mnhi/g_mnlo pair had
to be done, and doing it by address is what revealed that GHIDRA had
them right all along and the port had them backwards.

State after the 2026-09-05 sync: 676 of 784 port symbols verified in
place by address, 0 failures.  Of the 97 that differ, 14 are DRI libc
internals (`___pname`), 4 are Ghidra placeholders with no name at all,
and 79 are globals the map has never covered -- the "~93 remaining"
its own coverage note describes.  Extending it is research, not a
mechanical push: each needs confirming that Ghidra's descriptive name
really is that symbol.  One to look at first is `mi_pgtab`, where
Ghidra says `midi_channel_volume`.

E is the one category that cannot be closed from the binary at all:
scrbufA is referenced only at +511 through the align-up constant, and
that constant lives inside a relocated longword, so `+0x1FF` with base
0x1a7 and `+0x200` with base 0x1a6 are indistinguishable.

**A constant subscript is not evidence of an array.**  Alcyon folds it
into the absolute address, so `arr[7]` and a plain short emit the same
instruction.  aes_intO[16] was a single short (now mi_tpb); the cell
sits 14 bytes past AESBIND's int_out, which made "the sequencer borrows
int_out[7]" tempting and wrong -- int_out is only 14 bytes, so writing
it that way lands on the next global.  This is what category E guards:
a symbol referenced only at a non-zero offset has an inferred base, and
the inference is only as good as the assumed shape.

**A char array's declared size never reaches the codegen**, so only the
gap to the next cell the reference uses can settle it: g_agscw is 10,
g_ltscb 40, g_sfDoB 56.  The last one means the original really does
overrun its Dosound buffer -- sf_irqp copies `size` bytes there
straight from SOUNDS.LCP, which holds longer effects.  Reproduced as
written; do not widen the buffer.  (But see "Is the 56 real?" under the
Hatari chapter: that reading is an inference, not a measurement.)

**The BSS accounting CLOSES, and it localises every open size question
to three symbols** (2026-09-06).  lo68 gives the port 185 892 bytes of
BSS where the reference has 187 450 -- bss_remap only patches the
header number -- so the reference holds **1558 bytes that no port
symbol claims**.  Summing the gaps between each remapped symbol and
its declared size accounts for **1556** of them (the missing 2 are the
last symbol's tail).  That is a useful invariant: every other byte of
the reference's BSS is claimed, so the port's sizes are right
everywhere except where a gap shows.  27 gaps exist and 24 are <= 20
bytes of alignment slop.  The three that matter:

      533   scrbufA    category E -- its base is inferred from a +511
                       reference, so its size is undecidable (see
                       reloc_audit)
      488   mi_lstk    see below
      340   g_sfDoB    the Dosound-buffer question above

Re-run this sum after any change that touches a global: if the total
stops matching 1558, a declared size has drifted.

**mi_lstk's 488-byte hole: open, but harmless** (2026-09-06).  The
port declares `long mi_lstk[50]` (200 bytes) and the reference's next
used cell, mi_nnOn, is 688 bytes away.  Every other mi_* symbol in the
region is dense, so this is a genuine standout.

Two things make it a WEAKER puzzle than g_sfDoB, and the difference is
worth internalising:

  * **Nothing sits inside the gap.**  g_sfDoB's case is forced because
    g_sfdos/g_sfdoc are referenced at +56/+58, INSIDE the disputed
    extent, which is what rules out a plain array and demands a
    struct.  Here the 488 bytes are untouched by any relocation, so
    "mi_lstk is simply bigger" and "mi_lstk is 200 and a dead ~488-byte
    global follows it" are both unforced.  Dead declarations are known
    in this source -- mi_sig is declared and referenced by nothing,
    cmd_num is an uncalled static.
  * **The size has NO behavioural consequence.**  mq_pshl guards
    `mi_evcn < 49` and writes [mi_evcn] and [mi_evcn+1]; mq_popl reads
    [mi_evcn-2] and [mi_evcn-1].  Max index 49, so the highest byte
    touched is 49*4+3 = **199 of 200**.  No overrun is possible at any
    declared size >= 200.  Whichever reading is right, nothing
    observable changes -- unlike g_sfDoB, where the answer decides
    whether a shipped overrun exists at all.

Curiosity worth recording: the loop stack's empty sentinel is
`mi_evcn == 9`, and mq_zero initialises mi_evcn to 9 -- so indices
0..8 (36 bytes) are dead at the FRONT too.  Only [9..49] is ever
touched, a 164-byte window inside a 688-byte allocation.

An avenue that is CLOSED: declaration order cannot place a
hypothetical dead global here.  The 1985 linker's `.comm` order is not
source order -- the run around mi_lstk comes out in globals.c line
order 438, 247, 233, 273, 161, 30, 577, 97, 643, 537, 299, 199, 238 --
so there is no way to argue from where a declaration would have sat.

**Regenerating the spec needs the PRE-REMAP binary.**  alcyon_link.sh
leaves it as `build/alcyon/LCP_nobss.PRG`, and both `bss_remap.py
--gen` and reloc_audit.py default to it.  Running --gen against an
already-remapped LCP.PRG would pair the reference against itself and
freeze the drift into the spec.

**verify_bytes is not sufficient for this phase.**  It wildcards
relocations AND PC-relative displacements, so a function can report
MATCH while its internal branch targets differ (psg_upEn and ag_main
both did), and it walks the port's SYMBOL table, so `static` helpers
-- which Alcyon emits without a symbol -- were never compared at all
(`source/tools/stx_unverified.py` prints those runs).  Two tools
carried this phase instead:
  * `source/tools/stx_txtdiff.py` -- whole-text compare, relocations
    only wildcarded.  `[START] [COUNT]` to focus.
  * a prologue-span pairing pass: split both images at `link a6`
    prologues that follow an rts, then match each LCP_STX span to the
    port span of the same length with zero differing bytes.  That is
    what produced stx_u2's function order and what proves the
    inventory complete -- **265 LCP_STX spans against 265 port spans,
    one to one.**

Three layout levers, all now applied:

 1. **Object order** -- alcyon_link.sh names LCP_STX's explicitly for
    the default build: midi_seq 0x12a, mq_tick 0x219a, psg_asm 0x2272,
    cp_asm 0x22c0, stx_u1 0x400c, games 0x73e8, stx_u4 0xd9ea,
    stx_u2 0xde36, stx_u3 0x148fe, blkcp_a 0x17310, vdistx 0x1733a,
    then the library, with osbind.o right behind gemstart (the trap
    bindings sit at 0xfa).  stx_check.sh reuses that list verbatim --
    if the symbol side-link disagrees with LCP.PRG about object order,
    every symbol extent comes out wrong and the sweep reports mass
    divergence that is not real.
 2. **Function order inside each object.**  For a unity unit this is
    just reordering the #include lines -- but the unit then needs
    every header at the top (they emit no code, so layout is
    unaffected), and obdefs.h has no include guard of its own, so port
    sources include `obdefs1.h` instead.
    **LCP_STX did NOT group stx_u2 by source file**: aleisure's nine
    functions alone run from 0xe338 to 0x12ca0.  So the port has no
    action .c files left at all (aleisure, asimple, adoors, delivery,
    afood, abathrm, aidle, ahouse, aletter are deleted); every body
    lives in parts/, and **stx_u2.c's include list IS the object's
    function order**.
 3. **The VDI binding module.**  LCP_STX has ONE trap dispatcher and
    ONE parameter block where the port carried three copies of the
    same 22-byte routine (vdiown_a.s's vdi_go on vdipb, vdilib_a.s's
    vdi_go2 on vdipb2, VDIBIND's gsx1 on its private pblock) -- the
    port's entire +44-byte text surplus.  Its module is one object:

        vswr_mode 0x1733a < v_bar < v_gtext < v_opnvwk 0x17426
        < v_pline < vqt_attributes 0x174e4 < vro_cpyfm 0x1753a
        < vsf_color < vsf_interior < vsf_style < vsl_color
        < vst_color < vst_height 0x176b8 < wr_src < wr_dst
        < gsx1 0x1772e

    vqt_attributes and vst_height sitting BETWEEN Activision's own
    bindings proves they are not linked from VDIBIND there: the 1985
    source copied the DRI bodies in, as it did with the rest of the
    layer.  The default build therefore compiles `source/vdistx.c`
    (module in STX order; the nine shared bindings come from parts/,
    the four library-shaped bodies are written out) plus
    `source/vdistx_a.s` (wr_src, wr_dst and the single `_gsx1` on
    `_vdipb` -- defining `_gsx1` keeps VDIBIND's gsx1 member and its
    private pblock out of the link).  vdiown.h maps vdi_go/vdi_go2
    onto gsx1 for this configuration, tools/stx_units.txt skips
    vdiown.c and vdilib.c, and alcyon_link.sh drops
    vdilib.o/vdilib_a.o from the default LIST.

**Statics recovered in this phase** (all byte-exact, all found with
stx_txtdiff.py): pk_dbet 0x87a0, pk_evh 0x8804, pk_show 0x9a3a,
pk_cace 0xa1bc, pk_blf 0xa24a, pk_cdrw 0xa27a, pk_ante 0xaf66,
pk_chsc 0xd1b4, pk_bjr 0xd294, pk_cnbj 0xd608, pk_dchd 0xd67c,
pk_dbhi 0xd78e, pk_sbet 0xd864, pk_bjwr 0xb784, cmd_num 0x17278.
Recurring findings in this class: the counter is declared FIRST and
the inner loops reuse it; one local often doubles as two things
(pk_evh's j is the bubble-sort flag, its tmp is both the swap
temporary and the wheel flag); comparisons put the computer's side on
the left; scan loops are written body-first with a break, not with a
compound for condition; flag arrays are tested bare; and a helper
often has NO explicit return on its success path, leaving the last
compared value in d0 (pk_cace, the chk_timA pattern).  pk_tcm joins
pk_bm and pk_rm as a char POINTER.

A static defined AFTER its caller needs a file-scope forward
declaration or Alcyon treats the call as an external and the linker
resolves it to 0 -- and pk_main reaches pk_show with a bsr.s, so
pk_show must sit immediately behind it.

Things that cost bytes the original does not have: a `static` Alcyon
emits even when nothing calls it (midi_seq.c's five mh_* header
handlers, games.c's gamePlWQ), and helpers whose LCP_STX version is
smaller (gameCln takes no argument there and does not free).  Both
classes are now gated.  The converse also happens: **LCP_STX emits
statics nothing calls** -- cmd_num (0x17278) has no jsr or bsr
anywhere in the image -- so an unreferenced helper in the 1985 source
still costs its bytes.

Two more findings from this phase: LCP_STX keeps mi_dwrm, mi_rlock,
g_mtpre, g_msmsa and psg_ntAc in the TEXT segment immediately behind
mq_tick (0x226a-0x2271), and the last two are real BYTES, not BOOL16
words -- mq_tick.s defines all five itself now.

**Duplicated bodies are real.**  LCP_STX carries the "stand and look"
gesture twice -- 0x12c08 with `link #-10` (three locals it never
references) and 0x12c54 with `link #-4` -- and the callers name them:
a_lists/a_playp reach the first (li_loor), tt_on/tt_off the second
(li_lool).  Beware the mirror-image trap: two functions with identical
bodies make length+content pairing report a duplicate that is not one.
0xe310 is sc_sdtf, NOT a second exitVdi, even though their bodies are
the same `Setscreen(g_srlgb, -1L, -1)`; verify_bytes had simply never
compared it, because it skips functions under 48 bytes.

Source shapes recovered late in this phase:
  - The four SFX wrappers are ordered tvc, spe, hnd, grt (ids in the
    0xf91e SPEECH/3 and 0xf952 GREETING/2 bodies), and a_hello's
    random arm plays SPEECH on the non-zero roll, GREETING on zero.
  - gameTick's `if (g_sepex[g_lcieo] < 0) g_sepex[g_lcieo] = 0;` clamp
    is INSIDE the non-carrying arm -- the 0x1570e end-of-then jump
    (to 0x157bc, the epilogue, not to 0x1579a) is what says so.
  - pk_main's round loop: three `else if` chains are separate `if`s
    whose preceding arm closes with `goto next_round;`, and the
    trailing `goto next_round;` lives INSIDE the deepest block, which
    is why every end-of-if jump in the function targets the epilogue.
  - `x <= 11 && x >= 8` (not `< 12 && > 7`) for the face-card range.

Roadmap:
 1. ~~Function-level recovery~~ DONE -- 265/265 spans, zero code
    differences.
 2. ~~Object and function order~~ DONE -- text is byte-identical.
 3. **DATA and BSS layout.**  Analysed and largely fixed 2026-09-03
    by RELOCATION PAIRING: the text is byte-identical, so its 6429
    relocation SITES are identical in both binaries, and each site's
    stored longword gives a port-address -> LCP_STX-address pair.
    That pairs ~1070 distinct addresses and yields LCP_STX's whole
    data/bss layout without guessing.  **This is the tool for the
    layout phase** -- rebuild the pairing after any change and ask it
    which symbols are in the wrong segment, the wrong size, or the
    wrong order.

    Deltas went from data +3008 / bss +53884 to **-44 / -580**.  What
    the pairing showed and what was done:

    * 155 port DATA symbols were BSS in LCP_STX -- every
      `short x = 0;`.  Alcyon puts an explicitly zeroed global in
      .data; LCP_STX writes `short x;` and gets a `.comm`.  Twenty of
      them were NOT zero (g_pcmon/g_ppmon 400, mi_nlp0/mi_nxTk/
      mi_lpTk/g_mtdiv 100, psg_cvol 15, mi_evcn 9, lcp_watr 7,
      g_spdc/g_aprio 5, lcp_food 4, lcp_bwlS/g_aggun/scr_scal 1,
      lastAct/g_msmap/g_lcieo -1, vdipb's five pointers) -- LCP_STX
      initialises none of them and each has a run-time writer.  vdipb
      is safe because v_opnvwk assigns all five entries before its
      first trap.
    * 8 port BSS symbols were DATA in LCP_STX and got their real
      contents, read straight out of its data segment: bm32or and
      bm32and (LCP_STX ships the 32 shift/mask longs rather than
      building them at run time -- so initBM is not "dead code", it is
      redundant), g_mstr, g_mcpro, mi_chmap, g_sepef, moff_f, g_ew2a.
    * **g_ew2a can be a real table after all.**  Alcyon rejects the
      NESTED form `{ {..}, a, p }` with "mismatched curly braces", but
      takes the FLATTENED list -- which is how LCP_STX ships it.  And
      the port's host-side rows had action and priority the wrong way
      round in 33 of 34 rows: chk_encm returns the byte at +10 and
      adds the byte at +11 to g_aprio, and LCP_STX's row 0 is (24,15).
    * **g_mstr's static content is 0..99 then 110..127 then zeros** --
      the row 100..109 is missing from the 1985 table.  Preserved;
      mq_bust rewrites all 132 entries before anything reads them.
    * BSS was two over-allocated arrays.  scrbufA and scrbufB are ONE
      aligned screen each (32512), not two -- the "alt screen at
      +0x8000" reading came from the `&scrbufA[0x8000]` bug.  dsb_stor
      is 12832 bytes: fillTopR's largest caller is mg_stp's
      fillTopR(0x4d) = 77 rows of 160, plus the 512 the align-up can
      shift (LCP_STX's gap there is 12836).
    * SPRITE_HW_SLOTS_ALLOC is 8 again.  The port had widened those
      arrays to 10 so a stray slot-9 write stayed in bounds regardless
      of link order; LCP_STX tolerates it because the write lands in
      the ADJACENT array (g_sepey[9] == g_seacw[1]).  That safety now
      rests on reproducing the original's adjacency.
    * Deleted workin/work_out and g_setmt/g_setaw/g_setah -- duplicates
      of work_in/wk_out and the g_obt* trio, referenced by nothing.

    **A 2-D array's ROW STRIDE is written down in the text.**  Alcyon
    indexes `T a[N][M]` as `base + i * sizeof(T[M])` and emits that
    scale as a literal `muls.w #K,Dn` (or a shift for a power of two),
    so `source/tools/stx_strides.py` reads every array's stride back
    out of the disassembly.  Combined with the relocation gap it gives
    the OTHER dimension for free: N = gap / K.  It independently
    confirmed g_obtmt[56] (stride 20, gap 1120), wp_ans[5][12] (stride
    12, gap 60) and SPRITE_HW_SLOTS = 8 (g_semfi/g_semfm, stride 20,
    gap 160), each of which had been derived a different way -- and it
    checks a declaration outright: if the port says `a[N][M]` and the
    text multiplies by anything but M, one of them is wrong.  Watch for
    false positives: a `muls.w #10` next to a reference to a char
    buffer is decimal arithmetic (`d[1]*10 + d[2]`), not indexing.

    **The code's own loop bound is the authority on an array size.**
    main's OBJECTS walk is a literal `for (i = 0; i < 56; i++)`, so
    g_obtmt/g_obtaw/g_obtah are [56] -- and LCP_STX's gaps for all
    three agree (1120/112/112).  Conversely a loop bound can be a
    generous limit rather than the size: sf_sl walks `index < 500` but
    LCP_STX's mi_ntLp measures ~100 bytes, because the SOUNDS.LCP
    size-0 sentinel ends it after ~25 blocks.  And LCP_BODY_DEST_WORDS
    is 256 even though sp_lcpf only ever writes 168 of them -- four
    independent gaps (g_lsimg/g_lsmas/g_hsbuf/g_hsmas, 512 bytes each)
    say the original declared a round 256.

    **Screen buffers are 32512, not a round 32768.**  32000 plus the
    512 the align-up can shift.  All three align-up sites mask to 512,
    which is visible in the binary -- sprites.c 0x15110 pushes
    scrbufA+0x1ff then `andi.l #-512,(sp)`; stpScrB 0x65ae and
    fillTopR 0x6880 both `addl #512` then `andl #-512`.  The ST
    hardware only needs 256-byte alignment (which would make 32255
    enough), but this code does not use it.

    **DATA IS DONE (2026-09-03).**  12 260 == 12 260, the relocation
    site lists are IDENTICAL, and only NINE bytes differ: the low three
    bytes of each of psg_epp's three pointers into BSS, whose 14-byte
    stride already matches and whose base moves once BSS is laid out.

    Getting there needed one more idea beyond relocation pairing:
    **a compilation unit's .data comes out in source order, and its
    string bodies come out in the order c168 met them.**  Both streams
    therefore record where each declaration sits relative to the
    functions around it, and the two constraints together pin a global
    exactly:
      * A global's own bytes land among the globals of the file that
        declares it, in declaration order.
      * A string literal it names lands in the unit's literal pool,
        after the literals of every function compiled before that
        point.
    So a pointer whose string is late in the pool was declared late in
    the source -- e.g. pex_name ("pex.lcp", not "PE0.LCP") sits between
    ldSpr and main, and g_ltg's four sign-offs between a_lists and
    a_writl.  Moving such a declaration moves BOTH its bytes and its
    string, so a neighbour sometimes has to move with it (g_ltcwt
    followed g_ltg).  The port carries this as per-object data files
    included at the right points: dat_u1.c/dat_u1b.c/dat_u1c.c/
    dat_u1d.c, dat_u2.c/dat_u2b.c, dat_u3a.c/dat_u3b.c, dat_u4.c and
    dat_games.c/2/3/4, all listed in tools/stx_units.txt.

    **A switch jump table splits a data file.**  Alcyon emits the table
    into the .data of the object holding the function, so anything
    declared after that function lands after the table: execEv's and
    doAct's occupy 0xa20..0xaf3 and getKey's 0xba4..0xbe7, which is
    why stx_u1's globals come in three pieces.  A table's targets all
    point inside one function, which is how to tell it from a string
    pointer array.

    Content recoveries that fell out of the same comparison, each
    settled by the reference's own bytes:
      * sf_pri is 26 entries, not 32.  The port's dump had swallowed
        six bytes of mi_sig, the ten-byte Music Studio file signature
        (0xCD "Mstudio" 0xCD 0x02) that every SOUNDS.LCP and .SNG
        starts with -- a declared global nothing references.
      * mi_pgtab: sixteen bytes of initialized program map that the
        port had swallowed as g_msmk's second half while pointing
        mi_pgmap at a same-named EMPTY BSS array.  _mi_pgmapb and
        _mi_pgmap both truncate to _mi_pgma, so the pointer had been
        initialized to its own address.
      * ew2pos is 161 bytes: it ends in -1 and Alcyon pads the odd
        length to 162.  That 0xff had been mistaken for a {255, 0}
        sentinel at the head of g_ew2b.
      * g_ddipt and g_ddyot are nine entries (the picker's index is
        rndRng(base, 8)), g_ddxot eleven.
      * mo_names holds three-letter abbreviations, so the calendar
        reads "Sep 4, 1985"; g_ltg's sign-offs are real strings, not
        the four NULLs an LCP_ORG-era comment claimed; the anagram
        prompts are padded to 19 characters so each overwrites the
        last; g_aggpr[10] and g_agwgm[5] are declared past their
        initializer lists and Alcyon zero-fills the tails.
      * env_val, g_mccha, g_dsb and g_obtmp do not exist.  They were
        referenced by no code -- only by prose in comments -- and with
        them parked at the end the port carried 404 data relocations
        against 402.
      * mq_parh has no default arm.  Its default slot pointed at a
        `bra` that the text (already byte-identical) still contains;
        writing `break;` after the MIDI_HDR_END arm's `return;` keeps
        that unreachable branch and sends the default straight to the
        switch end.

    **BSS: DONE.**  Its layout is the LINKER's `.comm` allocation and
    matches no surviving tool, so it is not reproduced -- it is
    REMAPPED after the link from a checked-in spec (see the top of
    this section).  What the port had to fix first was not sizes but
    REFERENCES: pairing every relocation target and demanding the same
    SEGMENT, and the same address for text and data targets, turned up
    nine bugs no byte comparison can see.

      * dg_wkPth and lcp_flwp had their first comparison's operands
        the wrong way round.  Alcyon evaluates the RIGHT operand
        first, so `getFlrY(dog_y) != getFlrY(g_dty)` reaches dog_y in
        the second relocation -- the port compared the same two values
        but loaded them in the other order.
      * gameTick's four sound guards test g_sfplf, the "an effect is
        playing" flag sfx_irq sets, not g_sfacf.
      * mg_wkev reads the global lcp_watr, not lcp.water_level.
      * MFDB_A is a `short[10]`, not an MFDB, and stpScrB clears its
        first two words -- the halves of fd_addr -- where the port set
        fd_w/fd_h and passed the base four bytes below what cpyScr
        used.  (Alcyon rejects `((short *) &MFDB_A)[0] = 0` outright,
        "no code table for =", which is its own argument that the
        original declared an array.)
      * g_msmap and mi_seqE are one variable: the end-of-sequence
        pointer, with -1 for "no limit".
      * mi_ndur is a SECOND duration cell -- mq_pars writes it and
        only mq_qnne reads it, while mq_rdur's identical expression
        goes to mi_nlp0, which drives the tick counters.
      * g_mnhi / g_mnlo were named backwards (0x60 is the TOP of the
        playable note range), and mq_dise's guard tests the low limit
        first.
      * lcp_lgt and lcp_rgt write g_inpmd, not no_keyin.
      * cl_redrH's second cl_drwH call reads the cached g_cmmin and
        g_chhou back rather than t_min/t_hour.

    The last of these was last_hz / mi_lasT: ONE cell in the original,
    word accesses from the compositor and byte accesses from the
    sequencer, a byte write landing on the word's high half.  A cast
    macro does NOT express it -- `#define mi_lasT (*(char *) &last_hz)`
    makes Alcyon take the width from `&last_hz` and emit move.w where
    the original has move.b.  A UNION does: `union LASTHZ { unsigned
    short w; char b; }` with `#define last_hz lasthz.w` and
    `#define mi_lasT lasthz.b` keeps every use site unchanged and emits
    the original's widths exactly.  One symbol, one address, and the
    spec dropped from 417 rows to 416.  bss_remap.py still allows
    many-to-one and prints any alias, but there is none left.

    **Relocation pairing catches what byte comparison cannot.**  Both
    verify_bytes and stx_txtdiff wildcard relocated longwords, so a
    function can match byte for byte while the ADDRESS it loads is
    wrong.  Two such bugs fell out of this pass:
      - sc_ren8's alternate compositing buffer.  `&scrbufA[0x8000]`
        does not mean what it looks like: Alcyon's int is 16-bit, so
        0x8000 is -32768 and c168 emitted `move.l #-32768+_scrbufA` --
        the alt screen base was a pointer into the TEXT segment.
        LCP_STX stores `scrbufA + 0x1FF`, the same aligned buffer
        sp_iniM uses.
      - mq_tick.s had g_msmsa and psg_ntAc swapped (LCP_STX: psg_ntAc
        0x2270, g_msmsa 0x2271), so seven relocations pointed one byte
        off.
    With both fixed, every text relocation resolves to the same
    segment as LCP_STX's and every text->text one to the identical
    address.

## Running it under Hatari (2026-09-04)

Setup: TOS104US.ROM (never EmuTOS) copied somewhere with NO SPACES in
the path -- Hatari's option parser rejects a quoted path -- plus
`--machine st --cpulevel 0 --cpuclock 8 --memsize 1`.  Without those
the saved config's MMU/68030 settings double bus error during boot,
which looks like a game crash and is not.

What works: the build boots, draws the title screen, takes the
guestbook name/date/time through stEnter, loads HOUSE.SCN and draws the
house, and runs for ten emulated minutes with the dog wandering all
three floors and the clock advancing, with no frame corruption.

**What does NOT work: the copy protection.**  Under Hatari `cp_main`
always takes its failure path -- `cprot_r` reads 0, so `cs_mvIn` enters
`while (1) a_sleep(-1);`, which re-runs `lcp_hwt()` every iteration:
**the resident stands and waves for ever and the game never starts.**
`cpretv` (text 0x2604) is also 0, so it bails inside `cpseek`/`cprd`
-- the restore-to-track-0 and seek-to-track-79 step -- before it ever
issues the Read Track (0xE4) that reads the protected track.  Tried
without effect: --fastfdc off, --compatible on, --cpu-exact on,
--protect-floppy on, --drive-a-heads 1 and 2.

This is NOT a port defect, and the control is decisive: the run that
fails is `A:\LCP.PRG` off the Pasti image itself -- the original 1985
binary, which cmp shows is byte-identical to what we build.  Do not
chase it in the C source.  When a session shows the resident waving on
the spot, read cprot_r before assuming a regression.

**Build with `-DSKIP_COPYPROT=1` to actually play it** (see "Launching
/ running the port").  Verified 2026-09-04 from a GEMDOS drive: the
move-in cutscene plays, the dog arrives, the resident walks down from
the attic, sits in the armchair to read, and later changes clothes --
about ten emulated minutes with no corruption.  That configuration is
NOT byte-identical, by construction; rebuild from clean before
checking prg_diff again.

Launching from a GEMDOS drive fails earlier and differently: cp_main
calls Dgetdrv() and derives the PSG drive-select from it, so on C: it
selects no drive at all and spins in the FDC wait.  The game has to run
from the drive holding the disk.

**A gated build's BSS layout is not the original's, and that is not
cosmetic** (diagnosed 2026-09-06).  A `-DSKIP_COPYPROT` run used to die
at **VBL 16983**, reproducible to the frame: a bus error reading
`$ffffffa8` inside TOS's VDI at `$fd23da`, after which TOS loops on
`Pterm(-1)` and the program exits -- about 4.7 emulated minutes, which
is the "game exits after four minutes" report.

It is the `g_sfDoB` overrun, and it is worth knowing how it was found
because the same route works for any wild-pointer crash here:

  * `--trace vdi` names the failing call outright (`VDI 0x6D`,
    vro_cpyfm).  `--trace gemdos,cpu_exception` is 300 MB of normal
    trap dispatch -- exception 33 is TRAP #1, 34 is TRAP #2, 28/30 are
    autovector interrupts.  None of those is a fault; only exception 2
    is.
  * Hatari drops into its debugger on a bus error, so piping commands
    to its **stdin** dumps memory at the fault.  The first debugger
    entry is the harmless boot-time `$ffff8a00` blitter probe, so the
    script is `c` and then the real dump.
  * The load base is in the log: `Mshrink(0x12496, ...)` gives the
    basepage, text is basepage + 0x100 = **0x12596**, and
    `runtime - 0x12596` indexes `lcp_sym.68k` directly.

That gave `contrl[0]=109`, a source MFDB of `g_obtmt + 0x118` (stride
20, so object 14 -- perfectly valid) and a `pxy` of
`0,0,15,0x8003 / 271,92,286,0x805F`.  Working back through od_draw,
`sy2 = g_obtah[14] - 1`, so `g_obtah[14]` was `0x8004`.  `g_obtaw` was
intact; `g_obtah` held **SOUNDS.LCP block 17's payload[56:], byte for
byte** -- `sf_irqp`'s copy running off the end of `g_sfDoB[56]`.

SOUNDS.LCP has 23 effects; three exceed 56 bytes -- block 8
(SFX_HEAD_NOD) and block 17 (SFX_TOILET_REFILL) at 148, block 19 at
60.  So the worst overrun is **92 bytes**.

**Is the 56 real?  UNDECIDED, and the binary cannot decide it**
(raised by the maintainer, 2026-09-06).  A declared array size never
reaches the codegen, so `g_sfDoB[56]` was only ever inferred from the
distance to the next referenced cell.  `0x3fe48 -> 0x3ffd8` is exactly
**400**, which invites the reading that the original declared one
400-byte object and that `g_sfdos`/`g_sfdoc` are FIELDS INSIDE it at
+56/+58 -- in which case there is no overrun at all and the port has
simply mis-split one object into three.

What is settled: a plain `char g_sfDoB[400]` plus two separate
`short`s is IMPOSSIBLE.  `.comm` blocks pack densely here (see
0x3fe2a/0x3fe2e/0x3fe46), so a 400-byte buffer would put `g_sfdos` at
+400, not +56.  If the buffer is 400 those cells must be struct
fields.

What is not settled, and cannot be from the image:

  * 56+2+2 needs an unreferenced ~340-byte global at 0x3fe84.  That is
    NOT exotic in this source -- `mi_sig` (the ten-byte Music Studio
    signature) is declared and referenced by nothing, and `cmd_num`
    (0x17278) is a static with no caller in the whole image.  There is
    a second unexplained hole of the same kind at `mi_lstk` (+488).
  * A 400-byte struct needs a 56-byte buffer, two write-only status
    words, and a 340-byte unused tail.
  * 400 is round; so is the 340 the other model leaves.
  * Both models are BEHAVIOURALLY IDENTICAL -- under either, any
    effect over 56 bytes writes those two cells, and under either they
    are write-only.  No run can tell them apart, and the shipped
    binary is byte-identical either way.
  * A tempting false lead: `0xff` then `0` is the Dosound terminator,
    which would make the cells sound DATA.  It does not hold -- the
    reference emits `move.w`/`clr.w`, so the bytes are `00 FF 00 00`,
    not `FF 00`.

**THE MUSIC STUDIO CANNOT SETTLE IT EITHER** (checked 2026-09-06).
The obvious external source is the Activision Music Studio ST disk,
whose player LCP's sequencer descends from, and it IS on this machine
under `Retro/Atari ST/music_studio_activision_(usa)`.  Extracting it
and diffing its `AUDIO.PRG` against LCP_STX finds 10 063 shared bytes
-- but they are the copy protection (97.2% of cp_asm), the MIDI
sequencer's descent from the Music Studio player (mq_bust 73.8%,
mq_dise 44.2%), and DRI library.  **sf_irqp shares ZERO of its 456
bytes**, and the whole 0x400c-0x1733a game span shares nothing.  The
function that copies into g_sfDoB is LCP's own code, so Music Studio's
layout says nothing about that buffer.  Its `STANDARD.SND` is an
instrument-NAME table (signature version 0x01), not Dosound data, and
is no help either.  That avenue is closed; anything further has to
come from actual 1985 source or an analysis note predating the Ghidra
sync.

Ghidra's independent analysis calls them `soundeffect_dosound_status`
and `soundeffect_dosound_control`, separate from
`soundeffect_DoSound_Buffer[]` -- the same model the port uses, but an
analyst reading absolute addresses cannot distinguish a struct field
from a global either, so that is not evidence.

Treat the paragraph below as describing the port's CURRENT model, not
a proven fact about the 1985 source.  Settling it needs external
evidence, not another sweep.

Why the shipped build does not care: bss_remap puts `g_sfDoB` at
0x3fe48 followed by `g_sfdos` (+56) and `g_sfdoc` (+58) -- both
**write-only**, set by `sf_so()` and read nowhere in C or asm -- and
then 342 bytes no symbol claims (next is `g_srlgb` at +400).  The
overrun dies in that hole.  That is why 1985 shipped it.  A gated
build skips bss_remap, and lo68 puts `g_obtah` -- the 56-entry object
HEIGHT table -- at exactly +56.

`globals.c` therefore pads `g_sfDoB` to 400 **in test builds only**
(`SKIP_COPYPROT` / `SKIP_TITLE` / `SKIP_MIDI`), reproducing the
original's gap.  The shipped build keeps 56: widening it there would
change the BSS size in the header and break byte identity.  Verified
both ways -- shipped still MD5 eae52d14..., gated now runs 40 000 VBLs
with no bus error and no exit.

Two harness lessons from the same session.  `test_longrun_stable.sh`
ran to 15 000 VBLs, i.e. it stopped **1983 VBLs short** of the bug and
scored STABLE; the default is now 30 000.  And this Hatari wants
`--avirecord on` -- the bare flag now eats the next argument, which is
why the script reported "didn't produce an AVI" while swallowing
`--auto`.

**cp68 has no `defined()`.**  `#if defined(A) || defined(B)` does not
fail the build loudly, it just makes the file MISS; collect the gates
with separate `#ifdef`s instead.

## Every typed command tested (2026-09-06)

The parser's whole reachable surface, driven through the Hatari MCP and
checked against `g_aqueu`/`g_aliss`, which is where prsCmd appends the
action chk_encm returned.

**How the parser works, and how to enumerate it.**  Each recognised
word ORs `bm_lo[g_ew2b[w]]` into `g_ewb[ew2pos[w]]`; each of g_ew2a's
rows is a 10-byte mask plus an action at +10 and a priority at +11, and
a row fires when its mask is a SUBSET of the accumulated bits.  First
match wins.  So the command set is fully derivable from the tables --
pick, for each bit a row needs, a word that supplies it, then check no
earlier row is also satisfied.  `PLAY GAME` -> row 22 -> action 16
validates the model against observed behaviour.

**33 rows, 31 reachable, all 31 verified.**  One command per row:

    1 MAKE LOG 20        2 YOU SEEM COLD 20    3 USE LOG 20
    4 PUT MUSIC 5        5 TIDY UP OUGHT 36    7 USE PIANO 26
    8 USE SONG 26        9 TICKLE IVORIES 26  10 TYPE NOTE 7
   11 IS NOTE 7         12 BRUSH TEETH 17     13 MESSY TEETH 17
   14 DRINK WATER 13    15 SEEM GLASS 13      16 FEED DOG 31
   17 FILL CAN 31       18 OPEN CAN 31        19 MOON 8
   20 TIRED MUSIC 6     21 HATE MUSIC 6       22 USE WAR 16
   23 DUST ADDITION 14  24 ATARI 2            25 WHAT IN UPSTAIRS
   CLOSET 27            26 WHAT IN BEDROOM CLOSET 34
   27 WHAT IN KITCHEN CABINET 18              28 WHAT IN FILING
   CABINET 16           29 WHAT IN FREEZER 18 30 WHAT IN FRIDGE 18
   31 WHAT IN DRESSER 34                      32 WHAT IN NIGHTSTAND 34

**TWO rows can never fire, and it is the 1985 data that says so** --
the tables are byte-identical, so these are the original's quirks, not
port bugs.  Both predictions were confirmed by typing them: neither
produced a queue entry.

  * **Row 0, ACTION_HELLO.**  Its mask needs byte 9 bits 0x01|0x02.
    Only five words reach byte 9 -- EXCUSE, PARDON, HELLO, ATTENTION,
    HEY -- and ALL FIVE carry bit 0x02.  Nothing supplies 0x01, so the
    greeting action is unreachable from the keyboard.
  * **Row 6, the `MESSY IS HOME` phrasing of ACTION_CLEAN_UP.**  It
    needs byte 4 bit 0x08, which only `IS` at vwd_tab index 84
    supplies -- and index 84 is a DUPLICATE.  chk_vwd returns the
    first spelling match, index 26, whose bit is byte 1 0x02.  (The
    action itself is still reachable through row 5, `TIDY UP OUGHT`.)

**vwd_tab has four dead entries.**  chk_vwd scans linearly and returns
the first match, so a repeated spelling makes every later copy
unreachable: **START** (idx 35, shadowed by 20), **LIKE** (71, by 3)
and **IS** (84, by 26).  And index 0, **PLEASE**, is dead for a
different reason -- chk_encm tests `chk_vwd(...) == 0` as
"unrecognised", so the word at index 0 can never contribute its bit
and instead takes the +4 priority PENALTY that unknown words get.
Saying please makes the request less likely to be obeyed.

Method notes: a newline inside `type_text` acts as Return, so several
commands go in one call; the queue holds 10 and prsCmd drops anything
further, so drain it under turbo (watch g_aliss) between batches.  A
long batch can silently lose a keystroke -- two commands that failed
in a batch of five both worked when retyped alone, so re-test a
failure individually before believing it.

## All ten deal_kc key commands work (2026-09-06)

Driven through the Hatari MCP and verified by STATE, not by animation
-- each one has a global that must move:

      Ctrl-A  0x01  alarm     alarm_p set, then cleared when consumed
      Ctrl-B  0x02  book      putEv() entered
      Ctrl-C  0x03  phone     putEv() entered
      Ctrl-D  0x04  dog food  putEv() entered
      Ctrl-F  0x06  food      putEv() entered
      Ctrl-M  0x0D  Return    prsCmd -- proven all session (PLAY GAME,
                              CALL DOG, every minigame answer)
      Ctrl-P  0x10  pat LCP   g_ptdoa set then cleared (the RESIDENT,
                              not the dog -- see below)
      Ctrl-R  0x12  record    putEv() entered
      Ctrl-W  0x17  water     lcp_watr 4 -> 7 for three presses
      (8)           erase     g_cdibp 5 -> 4, g_cdinb[4] nulled --
                              works from the cursor-LEFT arrow and
                              from Backspace, which are the same code

**`KEY_CURSOR_LEFT` is named correctly** -- an earlier note in this
file called it a misnomer and that was WRONG, retracted 2026-09-06.
getKey has `case 0x4b: return 8;`, and 0x4b IS the cursor-left
scancode, reached because the arrow's ASCII byte is 0.  Backspace
independently IS ASCII 8.  So the two keys are aliases and both erase
a character; measured, the arrow takes g_cdibp 5 -> 4 and nulls
g_cdinb[4].  (st_titl and stEnter spell the same code as a bare `8`.)

The retracted claim came from a test run with turbo ON, where the
arrow appeared to do nothing.  It was the tx_sctm confound below:
key code 8 is NOT in tick.c's exempt list, so a key arriving after
tx_sctm has expired hits `g_cdibp = 0` FIRST and the erase then finds
an empty buffer.  With turbo off the same keypress works every time.
A key that looks dead under turbo is the emulator's pacing, not the
port's.

**Ctrl-P pats the RESIDENT, not the dog** (corrected by the
maintainer, 2026-09-06 -- an earlier note here called it "pat dog",
which is wrong).  The code says so three ways:

  * the handler sets `lcp.happiness = MOOD_HAPPY` -- the RESIDENT's
    mood, with `happiness_duration_active` reloaded;
  * the animation cycles `SPRITE_PET_HAND_1..6` -- a HAND, drawn
    `SPRITE_BEHIND_LCP` and ping-ponged 1->6->2 by tick.c -- at a
    FIXED (192,165), i.e. the player's hand reaching in, not the
    resident reaching for anything;
  * (192,165) is the phone: tick.c draws OBJ_PHONE_2 at (190,168),
    and ev_ansPh draws od_med1 at the same spot.

**Two renames followed, and only two** (2026-09-06):

    POS_BTM_DOG_FOOD (43)  ->  POS_BTM_COUCH
    dg_petok               ->  pat_ok

`POS_BTM_COUCH` is `g_rpxs[43] = 110`, i.e. **x = 220** on the BOTTOM
floor.  That is the couch beside the phone, and three things say so:
a_socwd sits there with `STATE_SIT_COUCH_UPRIGHT` and parks
SPRITE_READING_1 at (221,172); ev_ansPh answers the phone there; and
the real dog bowl is `POS_BTM_DOG_BOWL` (33) at x = 16, over in the
kitchen, used by a_feedd.  The old name had ONE user -- a_calld --
and was almost certainly guessed from a_calld's own name, which makes
it circular evidence.

**The rest of the "dog" cluster was left alone, because it is not
wrong.**  a_socwd genuinely drives `STATE_SIT_COUCH_PETTING_DOG`: the
resident really does pet the dog on that couch.  So `a_calld`
(crouches -- plausibly calling the dog over), `a_petd`,
`ACTION_CALL_DOG` and `ACTION_PET_DOG` all have real dog evidence and
keep their names.  `POS_BTM_DOG_FOOD_STORE` (44) is referenced by
NOTHING, so renaming it would swap one guess for another; left as is.

Renaming cost nothing here and that was checked, not assumed:
POS_BTM_* are `#define`s, and neither pat_ok nor g_ptdoa has a row in
`tools/stx_bss_layout.tsv` (only g_ptanf does, and "petting anim
frame" is already accurate).  `bss_remap.py --gen` reproduced the
spec UNCHANGED at 416 rows, prg_diff stayed BYTE-IDENTICAL, and
reloc_audit stayed clean.  Anything that DOES have a spec row needs
the regenerate-and-review cycle before it can be renamed.

For testing: pat_ok is set ONLY by a_calld, which no typed command
reaches -- neither ACTION_CALL_DOG (40) nor ACTION_PET_DOG (42)
appears in g_ew2a, so both are autonomous-only.  The reachable route
is a phone call (ev_ansPh calls a_calld), but the YES window is narrow
and easy to sample past.  Force the guard instead:
`w $<pat_ok> 0 1` (that write syntax is byte-at-a-time), then press
the key and watch g_ptdoa.

Three traps, all of which cost time here:

  * **The load base is NOT the same under the MCP.**  `--auto` puts
    text at 0x12596; the MCP's run_program puts it at **0x12492**.
    Every symbol address shifts by 0x104, and a wrong base reads
    plausible-looking garbage rather than failing.  Derive it at run
    time instead of assuming: dump a known table and subtract.  The
    power-of-two longs 1,2,4,8,16 are bm32or, which pins the base in
    one read.  (An earlier session's introSeq/dg_init readings used
    the wrong base; re-verified at the right one, both are 0 and the
    conclusion was unaffected -- but only the screenshots had ever
    really supported it.)
  * **Read memory while the machine is RUNNING.**  `debug('m ...')`
    works without pausing, and pause/resume round trips cost enough
    emulated time that short-lived state is gone before you look.
  * **Turbo makes transient state unobservable.**  tx_sctm is 160
    ticks, and with turbo on hundreds of frames pass between two MCP
    calls, so g_cdibp resets to 0 and a typed buffer looks like it
    never accumulated.  For anything transient use a VALUE-CHANGE
    breakpoint -- `($addr).w ! ($addr).w` -- which catches both the
    set and the clear and cannot be sampled past.  That is how
    alarm_p and g_ptdoa were confirmed after direct reads showed 0.

## All five minigames play (2026-09-06)

Driven end to end through the Hatari MCP on the gated build.  **All
five work**, and between them they exercise most of the statics the
byte-identity phase recovered:

  1. **Anagrams** (`ag_main`) -- puzzle rendered, a guess accepted and
     rejected (-> "Guess #2"), F1 clue re-jumbled the letters and
     consumed itself (the header drops "F1 Clue").
  2. **War** (`pk_wrMn`) -- 25/25 deal, round resolved to 24/26, next
     card dealt.
  3. **Poker** (`pk_main`) -- ante, five-card deal, bet, draw, second
     betting round, the RESIDENT bet 14 of its own accord, call,
     raise, showdown, pot of 32 transferred (384 -> 416).  Covers
     pk_dbet/pk_sbet, pk_evh, pk_cdrw/pk_ldCrd.
  4. **Blackjack** (`pk_bjMn`) -- bet, deal, SPLIT offered on 6h/6d
     (pk_chsc), double-down offered (pk_dbhi), hit to 19, stand,
     settled 401/399.
  5. **Word Puzzles** (`wp_main`) -- puzzle rendered, F1/F2 navigation
     between puzzles, solve mode, two-word answer taken a word at a
     time (wp_solv), return to selection.

Stability alongside it: **VBL 579 524, about 2.7 emulated hours**, 34x
past the 16 983 crash point, no bus error and no corruption, with the
house still compositing cleanly at the end.

**How to reach the menu.**  Type `PLAY GAME` and press Return --
ordinary characters accumulate in `g_cdinb` through `deal_kc`'s
default arm and `KEY_CTRL_M` (Return) submits via `prsCmd()`.  The
resident then fetches the game box from the filing cabinet, carries it
down to the kitchen table and sits; the menu is `a_plaag`'s, keys
'1'..'5'.  It is a long walk -- budget a few thousand frames -- and a
need (thirst, bathroom, a delivery) can interrupt it through
`lcp_lgt`, in which case the resident abandons the table and the
attempt has to be repeated.

Two harness traps, both of which cost real time:

  * **Keystrokes buffer and replay.**  `a_plaag` polls `getKey()` in a
    loop, so digits pressed early sit in the IKBD buffer and relaunch
    the game again and again -- measured at 7 entries to `ag_main` and
    26 to `pk_wrMn` from a handful of presses.  Those counts look
    exactly like a re-entrancy bug and are not one.  Screenshot to
    confirm the menu is actually up, then press the digit ONCE.
  * **`breakpoint_hits` under-reports.**  Breakpoints on `pk_main`,
    `pk_bjMn` and `wp_main` stayed armed (`list_breakpoints` confirms)
    yet reported no hits while all three demonstrably ran.  Do not
    read a silent `breakpoint_hits` as "the code never executed" --
    corroborate with a screenshot, which is the stronger evidence
    anyway: the game's UI cannot render without its main having run.

## Issue log -- ALL CLOSED

Kept as historical findings; none is an open port bug.

- **"The dog is missing", introSeq/dg_init never clear (CLOSED
  2026-09-06).**  Not a separate bug and nothing to do with
  SKIP_TITLE, which is where it was wrongly pinned twice.  It was the
  `g_sfDoB` overrun crash: `cs_mvIn` is a LONG cutscene -- doorbell,
  kitchen cabinet, sink, dresser, bathroom, suitcase -- and only its
  last few statements release the dog (`dg_init = 0`) and clear
  `introSeq`.  The program died at VBL 16983, i.e. INSIDE the
  cutscene, so neither ever ran.  With the overrun contained the
  cutscene completes: measured at VBL 25560, `introSeq` 0, `dg_init`
  0, dog visible in the kitchen and walking the ground floor two
  thousand frames later.

  The general lesson is worth keeping: a symptom that looks like
  missing game logic can just be a crash upstream of the code that
  would have produced it.  Check that the program is still alive
  before hunting for the flag that never got set.

- **test_longrun_stable.sh env-side failures (CLOSED 2026-07-21).**
  Root cause was tv_boul / tv_patl calling v_pline with count=2 but
  only initialising 1 point; fixed in 12e572f.  The 36000-VBL
  long-run has passed repeatedly since, with and without SKIP_MIDI.

- **Stairs (CLOSED 2026-07-21).**  The TEST_STAIRS harness was a
  false negative -- it warped the LCP past the AI walk that
  establishes valid stair state.  Real play walks stairs correctly;
  lcp_path/lcp_flwp byte-faithful.  Harness deleted.

- **Real-time crash inside TOS VDI (CLOSED).**  The July report --
  wild PC in low RAM, TOS cleanup cascading into $fc9304 -- predates
  the tv polyline fix (12e572f), the freeze fix, and the whole
  byte-fidelity campaign, and its "memwatch $25722" diagnostic
  address is stale after the layout reshuffle.  Caveat for future
  repro attempts: the saved Hatari config has bFastForward = TRUE, so
  scripted runs MUST pass an explicit --fast-forward on/off -- past
  "real-time" attempts that omitted the flag were silently
  fast-forwarded.

- **.SNG playback (CLOSED).**  The Timer-A ISR installs and ticks
  idle through a 36000-VBL --auto long-run (0 bus errors); a
  GEMDOS-traced run showed the resident simply never chose the record
  player autonomously in 10 minutes.  Nothing to fix -- hearing a song
  is a play-session activity (ask the resident to play a record).
