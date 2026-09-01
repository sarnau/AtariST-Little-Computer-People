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
